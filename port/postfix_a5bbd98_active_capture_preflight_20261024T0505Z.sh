#!/usr/bin/env bash
# Prepared only. Run exclusively after the physical :0/NVE7 and build owner releases them.
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd -P)
cd "$root"
expected=34c6549cd6649f39de79a2594a069b95fec508ed
# a5bbd98..34c6549 only changes PERFORMANCE_CHECKPOINT_STATUS.md; frozen software inputs remain current-tree hashes.
[[ $(git rev-parse HEAD) == "$expected" ]] || { echo 'HEAD changed; review/reprepare this preflight' >&2; exit 2; }
[[ ${DEATHLEAK_GPU_RELEASED:-} == yes ]] || { echo 'Set DEATHLEAK_GPU_RELEASED=yes only after performance owner explicitly releases physical GPU/build lane' >&2; exit 2; }
command -v docker >/dev/null
command -v python3 >/dev/null
packet="$root/port/build/postfix-a5bbd98-active-$(date -u +%Y%m%dT%H%M%SZ)-$$"
mkdir -p "$packet"
# Freeze the actual current-tree inputs, including pre-existing dirt, without modifying production.
git status --short > "$packet/status-before.txt"
git rev-parse HEAD > "$packet/head.txt"
sha256sum port/linux/main.cpp port/linux/SDLRenderer.cpp port/game/LevelScene.cpp > "$packet/source-sha256.txt"
python3 - "$root" "$packet" <<'PY'
from pathlib import Path
import sys
root, packet = map(Path, sys.argv[1:])
main = (root/'port/linux/main.cpp').read_text()
a = '    for (int frame = 0; !frameLimit || frame < frameLimit; ++frame) {'
b = '        renderer->EndFrame();\n'
assert main.count(a) == 1 and main.count(b) == 1
main = main.replace(a, '''    std::ofstream presentTrace;
    if (const char* path = std::getenv("DEATHLEAK_PRESENT_TRACE")) {
        presentTrace.open(path, std::ios::trunc);
        if (!presentTrace) { std::cerr << "Present trace open failed\\n"; return 1; }
        presentTrace << "frame,steady_ns,realtime_ns,level,loading_phase,gameplay_ran,camera_x,camera_y,player_x,player_y\\n";
    }
''' + a)
main = main.replace(b, b + '''        if (presentTrace) {
            const auto steadyNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
            const auto realtimeNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            presentTrace << frame << ',' << steadyNs << ',' << realtimeNs << ',' << currentLevel
                         << ',' << static_cast<int>(loadingPhase) << ',' << packetGameplayRan
                         << ',' << cameraX << ',' << cameraY << ',' << worldX << ',' << worldY << '\\n';
            presentTrace.flush();
        }
''')
(packet/'main.trace.cpp').write_text(main)
scene = (root/'port/game/LevelScene.cpp').read_text()
a = '        renderer.DrawSprite(draw);\n        if (object.player && filter.only.empty()) for (const auto& spray : footstepSprays_)'
assert scene.count(a) == 1
scene = scene.replace('#include <iostream>\n', '#include <iostream>\n#include <fstream>\n', 1)
scene = scene.replace(a, '''        if ((object.xmlId == "13" || object.xmlId == "14") &&
            levelPath_ == "Game/XmlFiles/levels/waterfall_top.xml" && draw.pixelWobble.enabled) {
            static std::ofstream uvTrace;
            static bool uvTraceOpened = false;
            if (!uvTraceOpened) {
                uvTraceOpened = true;
                if (const char* path = std::getenv("DEATHLEAK_ID13_14_UV_TRACE")) {
                    uvTrace.open(path, std::ios::trunc);
                    uvTrace << "id,submitted_v,pixel_wobble_time,camera_x,camera_y\\n";
                }
            }
            if (uvTrace) {
                uvTrace << object.xmlId << ',' << draw.uvShiftY << ',' << draw.pixelWobble.time
                        << ',' << cameraX << ',' << cameraY << '\\n';
                uvTrace.flush();
            }
        }
''' + a)
(packet/'LevelScene.trace.cpp').write_text(scene)
PY
sha256sum "$packet/main.trace.cpp" "$packet/LevelScene.trace.cpp" > "$packet/trace-source-sha256.txt"
# Isolated writable /out; source, Game assets, and overlay instrumentation remain read-only.
docker run --rm -u "$(id -u):$(id -g)" \
  --mount "type=bind,src=$root,dst=/src,readonly" \
  --mount "type=bind,src=$packet,dst=/out" \
  --mount "type=bind,src=$packet/main.trace.cpp,dst=/src/port/linux/main.cpp,readonly" \
  --mount "type=bind,src=$packet/LevelScene.trace.cpp,dst=/src/port/game/LevelScene.cpp,readonly" \
  -w /src deathleak-linux-build:bookworm \
  make -C port TARGET=/out/deathleak-linux all > "$packet/build.log" 2>&1
sha256sum "$packet/deathleak-linux" > "$packet/binary-sha256.txt"
cat > "$packet/record.sh" <<'REC'
#!/bin/sh
set -eu
cd /src
out=/out
DEATHLEAK_PRESENT_TRACE="$out/presents.csv" DEATHLEAK_ID13_14_UV_TRACE="$out/id13_14_uv.csv" \
 SDL_VIDEODRIVER=x11 SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy \
 stdbuf -oL -eL /out/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
 --script B:1800 --frames 100000 --fixed-dt 0.016666667 > "$out/game.log" 2>&1 &
game_pid=$!
ff_pid=
trap 'if [ -n "$ff_pid" ]; then kill -INT "$ff_pid" 2>/dev/null || true; fi; kill "$game_pid" 2>/dev/null || true' EXIT INT TERM
n=0
window_id=
while [ "$n" -lt 600 ]; do
  window_id=$(xwininfo -root -tree 2>/dev/null | awk '/"Death Leak - Linux renderer slice"/ {print $1; exit}')
  if [ -n "$window_id" ] && grep -q 'Audio level Game/XmlFiles/levels/forest_exploration_1.xml' "$out/game.log"; then break; fi
  sleep 0.05
  n=$((n+1))
done
[ -n "$window_id" ] || { echo 'No active Forest window'; exit 1; }
grep -q 'Audio level Game/XmlFiles/levels/forest_exploration_1.xml' "$out/game.log" || exit 1
xwininfo -id "$window_id" > "$out/window.txt"
ffmpeg -nostdin -hide_banner -loglevel verbose -debug_ts -f x11grab -window_id "$window_id" -framerate 30 -i :0 \
 -t 45 -vf scale=1280:720 -an -c:v mpeg4 -q:v 4 -pix_fmt yuv420p -y "$out/source.mp4" > "$out/ffmpeg.log" 2>&1 &
ff_pid=$!
n=0
while [ "$n" -lt 240 ]; do
  if awk -F, '$4 == "Game/XmlFiles/levels/waterfall_bottom.xml" && $5 == 0 && $6 == 1 {found=1; exit} END {exit !found}' "$out/presents.csv"; then break; fi
  if ! kill -0 "$ff_pid" 2>/dev/null; then echo 'Recorder exited before active bottom'; exit 1; fi
  sleep 0.25
  n=$((n+1))
done
[ "$n" -lt 240 ] || { echo 'No active bottom present within 60s'; exit 1; }
sleep 5
kill -INT "$ff_pid" 2>/dev/null || true
wait "$ff_pid" || true
ff_pid=
kill "$game_pid" 2>/dev/null || true
wait "$game_pid" || true
ffprobe -v error -count_frames -select_streams v:0 -show_entries stream=codec_name,width,height,avg_frame_rate,nb_read_frames -show_entries format=duration,size -of default=noprint_wrappers=1 "$out/source.mp4" > "$out/ffprobe.txt"
ffmpeg -nostdin -hide_banner -loglevel error -xerror -i "$out/source.mp4" -f null - > "$out/decode.log" 2>&1
REC
chmod +x "$packet/record.sh"
# Same proven host / Xauthority / NVE7 route; never run while the owner occupies the lane.
docker run --rm --network host --hostname jerome-Linux-MacBookPro \
  --device /dev/dri \
  --mount "type=bind,src=$root,dst=/src,readonly" \
  --mount "type=bind,src=$packet,dst=/out" \
  --mount type=bind,src=/tmp/.X11-unix,dst=/tmp/.X11-unix \
  --mount type=bind,src=/var/run/lightdm/root/:0,dst=/tmp/deathleak.Xauthority,readonly \
  -e DISPLAY=:0 -e XAUTHORITY=/tmp/deathleak.Xauthority \
  -w /src deathleak-capture:post-foliage /out/record.sh > "$packet/runner.log" 2>&1
# Hard gates: live accelerated device, all three active states, and unbounded submitted V after crossing.
grep -Eq 'video_driver=x11 name=opengl .*accelerated=1 software=0 .*mode=strict-accelerated' "$packet/game.log"
grep -q 'OpenGL device vendor=nouveau renderer=NVE7' "$packet/game.log"
for level in forest_exploration_1 waterfall_top waterfall_bottom; do
  awk -F, -v level="Game/XmlFiles/levels/$level.xml" '$4 == level && $5 == 0 && $6 == 1 {found=1; exit} END {exit !found}' "$packet/presents.csv"
done
awk -F, 'NR>1 && $1==13 && $2>1 {a=1} NR>1 && $1==14 && $2>1 {b=1} END {exit !(a&&b)}' "$packet/id13_14_uv.csv"
python3 - "$packet" <<'PY'
from pathlib import Path
from bisect import bisect_right
import csv, json, re, sys
p = Path(sys.argv[1])
rows = list(csv.DictReader((p/'presents.csv').open()))
rows = sorted(rows, key=lambda r:int(r['realtime_ns']))
times = [int(r['realtime_ns']) for r in rows]
pts_us = [int(m.group(1)) for m in re.finditer(r'^demuxer -> .*?\bpkt_pts:(\d+)', (p/'ffmpeg.log').read_text(), re.M)]
if len(pts_us) < 600: raise SystemExit('Too few FFmpeg x11grab source timestamps')
packets = [x*1000 for x in pts_us]
active = {}
for row in rows:
    if row['loading_phase']=='0' and row['gameplay_ran']=='1':
        active.setdefault(row['level'],row)
for level in ('forest_exploration_1','waterfall_top','waterfall_bottom'):
    name='Game/XmlFiles/levels/'+level+'.xml'
    if name not in active: raise SystemExit('Missing active '+name)
samples = []
for second in (1,9,11,14,18,20,22,24):
    target=packets[0]+second*1000000000
    index=min(range(len(packets)),key=lambda i:abs(packets[i]-target))
    present_index=bisect_right(times,packets[index])-1
    if present_index < 0: raise SystemExit('No prior present for source packet')
    samples.append({'nominal_second':second,'source_packet_index':index,
                    'source_realtime_ns':packets[index],'last_present':rows[present_index]})
bottom = active['Game/XmlFiles/levels/waterfall_bottom.xml']
if sum(x >= int(bottom['realtime_ns']) for x in packets) < 30:
    raise SystemExit('Insufficient synchronized active bottom source packets')
(p/'alignment.json').write_text(json.dumps({'source_first_realtime_ns':packets[0],
  'source_last_realtime_ns':packets[-1], 'source_packet_count':len(packets),
  'present_count':len(rows),'first_active_present':active,'samples':samples},indent=2))
PY
docker run --rm --mount "type=bind,src=$packet,dst=/out" -w /out deathleak-capture:post-foliage sh -c '
  for second in 1 9 11 14 18 20 22 24; do
    ffmpeg -nostdin -hide_banner -loglevel error -ss "$second" -i /out/source.mp4 -frames:v 1 -y "/out/still-${second}s.png" || exit
  done
'
sha256sum "$packet/source.mp4" > "$packet/video-sha256.txt"
git status --short > "$packet/status-after.txt"
cat "$packet/ffprobe.txt"
echo "READY EVIDENCE: $packet"
echo 'Review alignment.json and stills for top paired rafts/bottom; inspect per-ID V/time. Linux effect only, not Windows parity.'

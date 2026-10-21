#!/bin/sh
# Scripted save/load smoke test on the physical Xorg GPU; audio uses SDL dummy.
set -eu
root=$(pwd -P)
bin="$root/port/build/deathleak-linux"
test -x "$bin" || { echo 'build port/build/deathleak-linux first' >&2; exit 2; }
mkdir -p "$root/port/build/physical-xorg-save-load"
base="$root/port/build/physical-xorg-save-load"
save="$base/progression.txt"
container="deathleak-xorg-save-$$"
trap 'docker stop "$container" >/dev/null 2>&1 || true' EXIT HUP INT TERM
run_xorg() {
  timeout --signal=TERM --kill-after=10s 90s docker run --rm --name "$container" --network host --hostname jerome-Linux-MacBookPro \
    --device /dev/dri \
    --mount "type=bind,src=$root,dst=/src" \
    --mount type=bind,src=/tmp/.X11-unix,dst=/tmp/.X11-unix \
    --mount type=bind,src=/var/run/lightdm/root/:0,dst=/tmp/deathleak.Xauthority,readonly \
    -e DISPLAY=:0 -e XAUTHORITY=/tmp/deathleak.Xauthority \
    -e SDL_VIDEODRIVER=x11 -e SDL_RENDER_DRIVER=opengl -e SDL_AUDIODRIVER=dummy \
    -w /src deathleak-linux-build:bookworm port/build/deathleak-linux "$@"
}
rm -f "$save"
run_xorg --play Game/XmlFiles/levels/forest_exploration_1.xml --script R:420,E:40 \
  --frames 460 --fixed-dt 0.0166667 --save-file port/build/physical-xorg-save-load/progression.txt --profile >"$base/save.log" 2>&1 || { tail -n 20 "$base/save.log" >&2; exit 1; }
test -s "$save" || { echo 'physical Xorg save missing or empty' >&2; exit 1; }
run_xorg --load-save port/build/physical-xorg-save-load/progression.txt --script R:120 \
  --frames 120 --fixed-dt 0.0166667 --profile >"$base/load.log" 2>&1 || { tail -n 20 "$base/load.log" >&2; exit 1; }
for log in "$base/save.log" "$base/load.log"; do
  grep -q 'video_driver=x11 name=opengl .*accelerated=1 software=0 .*mode=strict-accelerated' "$log" || { echo "strict Xorg acceleration absent: $log" >&2; exit 1; }
  grep -q 'OpenGL device vendor=nouveau renderer=NVE7' "$log" || { echo "physical NVE7 GPU absent: $log" >&2; exit 1; }
  if grep -q 'Player died at frame' "$log"; then echo "save/load smoke-test coverage ended in ordinary gameplay death: $log" >&2; exit 1; fi
done
grep -q '^Entered Game/XmlFiles/levels/waterfall_top.xml$' "$base/save.log" || { echo 'authored transition absent before save' >&2; exit 1; }
grep -q '^Simulation ended at Game/XmlFiles/levels/waterfall_top.xml .*orbs=7 .*health=25 ' "$base/save.log" || { echo 'saved gameplay state unexpected' >&2; exit 1; }
grep -q '^Simulation ended at Game/XmlFiles/levels/waterfall_top.xml .*orbs=9 .*health=25 ' "$base/load.log" || { echo 'loaded gameplay did not advance with preserved state' >&2; exit 1; }
printf 'Physical Xorg save/load passed: forest→waterfall save (7 orbs), GPU reload and resumed play (9 orbs); scripted input, dummy audio.\n'

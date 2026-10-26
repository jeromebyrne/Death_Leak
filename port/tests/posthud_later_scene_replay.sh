#!/bin/sh
# Functional offscreen parity after uninterrupted ordinary-control Forest route.
set -eu
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
out=${1:-port/build/posthud-later-scenes}
mkdir -p "$out"
route=R:1100,Q:40,R:640,E:1500,R:1300
for case_spec in cave6:1781:underwater_cave_6.xml hot_springs:3038:hot_springs.xml cave4:3191:underwater_cave_4.xml cave3:3416:underwater_cave_3.xml; do
  name=${case_spec%%:*}; rest=${case_spec#*:}; frames=${rest%%:*}; scene=${rest#*:}
  prefix="$out/$name"
  if [ "${DEATHLEAK_VERIFY_ONLY:-0}" != 1 ]; then
    for arm in ordinary repeat; do
      if [ "$arm" = repeat ]; then suffix=.repeat; repeat_arg=--repeat-presentation-check; else suffix=; repeat_arg=; fi
      port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
        --script "$route" --frames "$frames" --fixed-dt 0.0166667 \
        --test-film-clock-step-ms 100 --resolution 1280x720 \
        --capture "$prefix$suffix.bmp" --combat-state $repeat_arg > "$prefix.$arm.log" 2>&1
    done
  fi
  python3 - "$prefix" "$scene" <<'PY'
import hashlib, pathlib, re, sys
p=pathlib.Path(sys.argv[1]); scene=sys.argv[2]
paths=[p.with_suffix('.bmp'), pathlib.Path(str(p)+'.repeat.bmp'), pathlib.Path(str(p)+'.repeat.bmp.repeat.bmp')]
blobs=[x.read_bytes() for x in paths]
assert all(x[:2]==b'BM' and int.from_bytes(x[18:22],'little')==1280 and int.from_bytes(x[22:26],'little')==720 and int.from_bytes(x[28:30],'little')==32 for x in blobs), paths
assert blobs[0]==blobs[1]==blobs[2], f'post-HUD pixel mismatch: {paths}'
logs=[pathlib.Path(str(p)+'.ordinary.log').read_text(),pathlib.Path(str(p)+'.repeat.log').read_text()]
for tag in ('Trigger at frame','Door at frame','Simulation ended at','Player/NPC contacts=','Melee active=','Deflection unlocked='):
    def lines(s): return [re.sub(r' elapsed=[0-9.]+s', ' elapsed=<ignored>', x) for x in s.splitlines() if x.startswith(tag)]
    assert lines(logs[0])==lines(logs[1]), f'{p.name}: {tag} endpoint/event mismatch'
assert 'PRESENTATION_REPEAT ' in logs[1] and 'PRESENTATION_REPEAT ' not in logs[0]
assert f'Simulation ended at Game/XmlFiles/levels/{scene}' in logs[0]
for frame,src,dst in ((398,'forest_exploration_1.xml','waterfall_top.xml'),(762,'waterfall_top.xml','waterfall_bottom.xml'),(1378,'waterfall_bottom.xml','underwater_cave.xml'),(1780,'underwater_cave.xml','underwater_cave_6.xml')):
    assert re.search(rf'Trigger at frame {frame} from .*{src} .*{dst}', logs[0]), (p.name,frame)
if p.name!='cave6':
    assert re.search(r'Trigger at frame 3037 from .*underwater_cave_6.xml .*hot_springs.xml', logs[0])
if p.name in ('cave4','cave3'):
    assert re.search(r'Trigger at frame 3190 from .*hot_springs.xml .*underwater_cave_4.xml', logs[0])
if p.name=='cave3':
    assert re.search(r'Trigger at frame 3415 from .*underwater_cave_4.xml .*underwater_cave_3.xml', logs[0])
assert 'Player died' not in logs[0]
print(p.name, '1280x720 0/921600 changed pixels', hashlib.sha256(blobs[0]).hexdigest(), 'route/endpoints identical')
PY
done

#!/bin/sh
# Test-only offscreen full post-HUD exact-byte parity at a real arena damage tick.
set -eu
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
out=${1:-port/build/posthud-arena-hit}
mkdir -p "$out"
route=L:1500,Q:40,L:100,F:300
if [ "${DEATHLEAK_VERIFY_ONLY:-0}" != 1 ]; then
  port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script "$route" --frames 1463 --fixed-dt 0.0166667 --simulate-only \
    --combat-state > "$out/prehit.log" 2>&1
fi
python3 - "$out/prehit.log" <<'PY_CHECK'
import pathlib, re, sys
s=pathlib.Path(sys.argv[1]).read_text()
end=next(x for x in s.splitlines() if x.startswith('Simulation ended at'))
assert 'first_ninja_arena.xml' in end and re.search(r'\bhits=0\b', end), end
assert 'Trigger at frame 300 from' in s and 'Trigger at frame 1093 from' in s
PY_CHECK
for case_spec in first_hit:1464:0:1:36 fired_followthrough:1940:18:18:38; do
  name=${case_spec%%:*}; rest=${case_spec#*:}; frames=${rest%%:*}; rest=${rest#*:}; fired=${rest%%:*}; rest=${rest#*:}; shots=${rest%%:*}; orbs=${rest#*:}
  if [ -n "${DEATHLEAK_CASE_ONLY:-}" ] && [ "$name" != "$DEATHLEAK_CASE_ONLY" ]; then continue; fi
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
  python3 - "$prefix" "$fired" "$shots" "$orbs" <<'PY'
import hashlib, pathlib, re, sys
p=pathlib.Path(sys.argv[1]); fired=int(sys.argv[2]); shots=int(sys.argv[3]); orbs=int(sys.argv[4])
paths=[p.with_suffix('.bmp'), pathlib.Path(str(p)+'.repeat.bmp'), pathlib.Path(str(p)+'.repeat.bmp.repeat.bmp')]
blobs=[x.read_bytes() for x in paths]
assert all(x[:2]==b'BM' and int.from_bytes(x[18:22],'little')==1280 and int.from_bytes(x[22:26],'little')==720 and int.from_bytes(x[28:30],'little')==32 for x in blobs), paths
assert blobs[0]==blobs[1]==blobs[2], f'post-HUD pixel mismatch: {paths}'
logs=[pathlib.Path(str(p)+'.ordinary.log').read_text(),pathlib.Path(str(p)+'.repeat.log').read_text()]
tags=('Trigger at frame','Door at frame','Simulation ended at','Player/NPC contacts=','Melee active=','Deflection unlocked=','Enemy ','Shot ')
def state(s):
    return [re.sub(r' elapsed=[0-9.]+s', ' elapsed=<wall-time>', line) for line in s.splitlines() if line.startswith(tags)]
assert state(logs[0])==state(logs[1]), f'{p.name}: gameplay/combat/transition endpoint mismatch'
assert 'PRESENTATION_REPEAT ' in logs[1] and 'PRESENTATION_REPEAT ' not in logs[0]
for frame,src,dst in ((300,'forest_exploration_1.xml','grass_exploration_3.xml'),(1093,'grass_exploration_3.xml','first_ninja_arena.xml')):
    assert re.search(rf'Trigger at frame {frame} from .*{src} .*{dst}', logs[0]), (p.name,frame)
assert len([line for line in logs[0].splitlines() if line.startswith('Trigger at frame')])==2
end=next(line for line in logs[0].splitlines() if line.startswith('Simulation ended at'))
assert 'first_ninja_arena.xml' in end
for key,value in {'orbs':orbs,'keys':0,'health':22,'balance':orbs,'spawned':3,'enemies':2,'hits':1,'kills':0,'fired':fired,'shots':shots}.items():
    assert re.search(rf'\b{key}={value}(?:\s|$)', end), (p.name,key,end)
assert 'Player died' not in logs[0] and 'Player died' not in logs[1]
assert len([line for line in logs[0].splitlines() if line.startswith('Enemy ')])==2
print(p.name, '1280x720 full post-HUD 0/921600 changed pixels', hashlib.sha256(blobs[0]).hexdigest(), 'gameplay/combat/transition endpoints identical')
PY
done

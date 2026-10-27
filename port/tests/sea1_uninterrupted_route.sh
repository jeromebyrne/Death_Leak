#!/bin/sh
# Forest-start authored route through cave-6, arena, grass-2, cliff, and Sea-1.
# CPU/scheduler correctness only; no physical display/GPU/input acceptance.
set -eu
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
binary=${1:-port/build/deathleak-linux}
out=${2:-port/build/sea1-uninterrupted}
mkdir -p "$out"
route=B:1100,Q:40,R:641,J:200,E:2200,L:48,I:10,L:1500,Q:40,L:100,F:300,R:300,F:300,L:245,Q:80,N:100,Q:80,N:788,Q:35,N:243,L:250,O:1,Q:599,L:2100
for hz in 60 144; do
  "$binary" --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script "$route" --frames 11300 --simulate-only --combat-state \
    --test-refresh-hz "$hz" > "$out/$hz.log" 2>&1
done
python3 - "$out" <<'PY'
import pathlib,re,sys
root=pathlib.Path(sys.argv[1]); a=(root/'60.log').read_text(); b=(root/'144.log').read_text()
tags=('Trigger at frame','Door at frame','Simulation ended at','Player/NPC contacts=','Melee active=','Deflection unlocked=','Enemy ','Shot ')
def state(s):
    return [re.sub(r' elapsed=[0-9.]+s',' elapsed=<wall>',line) for line in s.splitlines() if line.startswith(tags)]
assert state(a)==state(b), '60/144-Hz fixed gameplay, combat, or transition state differs'
expected=((398,'forest_exploration_1.xml','waterfall_top.xml'),(762,'waterfall_top.xml','waterfall_bottom.xml'),(1378,'waterfall_bottom.xml','underwater_cave.xml'),(1780,'underwater_cave.xml','underwater_cave_6.xml'),(3224,'underwater_cave_6.xml','hot_springs.xml'),(3377,'hot_springs.xml','underwater_cave_4.xml'),(3602,'underwater_cave_4.xml','underwater_cave_3.xml'),(3637,'underwater_cave_3.xml','underwater_cave_4.xml'),(3868,'underwater_cave_4.xml','underwater_cave_5.xml'),(4590,'forest_exploration_1.xml','grass_exploration_3.xml'),(5383,'grass_exploration_3.xml','first_ninja_arena.xml'),(8307,'grass_exploration_1.xml','grass_exploration_2.xml'),(9467,'grass_exploration_2.xml','cliff.xml'),(11080,'cliff.xml','sea_1.xml'))
for frame,src,dst in expected:
    assert re.search(rf'Trigger at frame {frame} from .*{re.escape(src)} .*{re.escape(dst)}',b), (frame,src,dst)
assert re.search(r'Door at frame 4231 from .*underwater_cave_5\.xml to .*forest_exploration_1\.xml',b)
assert re.search(r'Simulation ended at .*sea_1\.xml .*orbs=134 .*health=4 .*balance=134 .*hits=9 kills=0 fired=168',b), 'Sea-1 alive endpoint changed'
assert 'Player died' not in b
print('PASS: authored cave-6, cave-5 door, arena, grass-2, cliff, Sea-1; exact 60/144-Hz gameplay/combat endpoint')
for line in state(b):
    if line.startswith(('Trigger at frame 8307','Trigger at frame 9467','Trigger at frame 11080','Simulation ended at')): print(line)
PY

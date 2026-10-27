#!/bin/sh
# Forest-start ordinary-control granary acquisition and authored lower-grass return.
set -eu
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
binary=${1:-port/build/deathleak-linux}
out=${2:-port/build/granary-return-route}
mkdir -p "$out"
route=B:1100,Q:40,R:641,J:200,E:2200,L:48,I:10,L:1500,Q:40,L:100,F:300,R:300,F:300,L:245,Q:80,N:100,Q:80,N:765,Q:160,N:100,I:10,R:100,E:80,N:2,E:80
for hz in 60 144; do
  "$binary" --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script "$route" --frames 8581 --simulate-only --combat-state \
    --test-refresh-hz "$hz" > "$out/$hz.log" 2>&1
done
python3 - "$out" <<'PY'
import pathlib,re,sys
root=pathlib.Path(sys.argv[1]); logs=[(root/f'{hz}.log').read_text() for hz in (60,144)]
tags=('Trigger at frame','Door at frame','Collected key','Simulation ended at','Player/NPC contacts=','Melee active=','Deflection unlocked=','Enemy ','Shot ')
def state(s):
    return [re.sub(r' elapsed=[0-9.]+s',' elapsed=<wall>',line) for line in s.splitlines() if line.startswith(tags)]
assert state(logs[0])==state(logs[1]),'60/144-Hz fixed-state route differs'
for log in logs:
    assert 'Collected key granary at frame 8311' in log
    assert re.search(r'Trigger at frame 8515 from .*grass_exploration_dead_end_below_2\.xml .*grass_exploration_1\.xml',log)
    assert re.search(r'Simulation ended at .*grass_exploration_1\.xml .*keys=1 health=4 ',log)
    assert 'Player died' not in log
print('PASS granary frame 8311; authored return frame 8515; alive with key; synthetic 60/144-Hz fixed-state parity')
PY

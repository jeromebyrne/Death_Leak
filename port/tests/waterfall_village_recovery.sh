#!/bin/sh
# Current fixed-clock ordinary-input crossing, not real-display acceptance.
set -eu
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
out=${1:-port/build/waterfall-village-recovery}
mkdir -p "$out"
for hz in 60 144; do
    port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
        --script R:711,E:40,B:4515 --frames 5266 --simulate-only \
        --test-refresh-hz "$hz" --combat-state --save-file "$out/$hz.save" > "$out/$hz.log" 2>&1
done
python3 - "$out" <<'PY'
import pathlib,re,sys
root=pathlib.Path(sys.argv[1]); logs=[(root/f'{hz}.log').read_text() for hz in (60,144)]
tags=('Trigger at frame','Simulation ended at','Player/NPC contacts=','Melee active=','Deflection unlocked=','Enemy ','Shot ')
def state(s):
    return [re.sub(r'elapsed=[0-9.]+s','elapsed=<wall>',x) for x in s.splitlines() if x.startswith(tags)]
assert state(logs[0])==state(logs[1]), 'synthetic refresh changed fixed gameplay state'
transitions=((398,'forest_exploration_1','waterfall_top'),(1074,'waterfall_top','forest_exploration_2'),(1730,'forest_exploration_2','forest_exploration_barn'),(2730,'forest_exploration_barn','village_forts'),(4053,'village_forts','village_central'))
for hz,log in zip((60,144),logs):
    for frame,src,dst in transitions:
        assert re.search(rf'Trigger at frame {frame} from .*{src}\.xml .*to .*{dst}\.xml',log),(frame,src,dst)
    assert 'Player died' not in log and 'Entered Game/XmlFiles/levels/waterfall_bottom.xml' not in log
    assert re.search(r'Simulation ended at .*village_central.xml player=\(7287.18,-1525.13\) orbs=101 keys=0 health=4 .*balance=101',log),log
    lines=(root/f'{hz}.save').read_text().splitlines()
    ids=[x for x in lines if x.startswith('Game/XmlFiles/levels/') and '#' in x]
    assert len(ids)==len(set(ids))==101 and 'B:101' in lines
print('PASS: ordinary-input fixed-clock waterfall crossing to village; five transitions; 101 unique earned orbs; health 4; synthetic 60/144 endpoint parity')
PY

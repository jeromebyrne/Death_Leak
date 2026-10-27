#!/bin/sh
# One process per snapshot from the authored spawn. Test-only encounter clock,
# not a claim of actual wall-clock pacing, physical display, or controller input.
set -eu
binary=${1:-port/build/deathleak-linux}
out=${2:-port/build/earned-wall-clock-route}
mkdir -p "$out"
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
route=$(cat port/tests/routes/earned_wall_clock_ending.script)
for stage in purchase preboat completion; do
    case "$stage" in
        purchase) frames=31930 ;;
        preboat) frames=56556 ;;
        completion) frames=60000 ;;
    esac
    # Isolate metadata from prior executions; no progression is ever loaded.
    printf 'fresh test-only progression\n' > "$out/$stage.save"
    DEATHLEAK_TRACE_ENCOUNTERS=1 "$binary" --play Game/XmlFiles/levels/forest_exploration_1.xml \
        --script "$route" --frames "$frames" --fixed-dt 0.0166667 \
        --simulate-only --simulate-wall-clock --combat-state \
        --save-file "$out/$stage.save" > "$out/$stage.log" 2>&1
done
python3 - "$out" <<'PY'
from pathlib import Path
import re, sys, xml.etree.ElementTree as ET
root=Path(sys.argv[1]); cache={}
script=Path('port/tests/routes/earned_wall_clock_ending.script').read_text().strip()
for token in script.split(','):
    command, count=token.split(':')
    assert command in '.RLEQBNYZJIMP' and len(command)==1 and int(count)>0, token
logs={name:(root/f'{name}.log').read_text() for name in ('purchase','preboat','completion')}
for name,log in logs.items():
    assert 'TEST-ONLY encounter clock follows fixed gameplay dt' in log
    assert 'Player died' not in log, name
    assert 'Encounter initialize id=Game/XmlFiles/levels/village_central.xml#151 elapsed_ms=116717 suppressed=1' in log
    assert 'Purchased kDeflection at frame 31929 balance=22' in log
for name,count,balance,keys in (
    ('purchase',397,22,{'granary'}),
    ('preboat',421,46,{'granary','dojo','village_hut','has_repair_tools'}),
):
    lines=(root/f'{name}.save').read_text().splitlines()
    ids=[x for x in lines if x.startswith('Game/XmlFiles/levels/') and '#' in x]
    assert len(ids)==len(set(ids))==count
    assert [x for x in lines if x.startswith('B:')]==[f'B:{balance}']
    assert count-balance==375 and 'U:kDeflection' in lines
    assert {x[2:] for x in lines if x.startswith('K:')}==keys
    for pickup in ids:
        level,object_id=pickup.rsplit('#',1)
        if level not in cache: cache[level]={x.get('id'):x.tag for x in ET.parse(level).getroot()}
        assert cache[level].get(object_id)=='CurrencyOrb', pickup
pre=logs['preboat']; end=logs['completion']
assert re.search(r'Simulation ended at .*cliff.xml .*orbs=421 keys=4 health=1 .*balance=46 spawned=26 enemies=0',pre)
for event in ('Collected key granary at frame 25681','Collected key dojo at frame 40768',
              'Collected key village_hut at frame 42982','Collected key has_repair_tools at frame 48476',
              'Boat at frame 56556','Final title DEATH LEAK at frame 57457','EndStory at frame 58657'):
    assert event in end, event
assert 'Deflection unlocked=1 count=1' in end
assert re.search(r'orbs=421 keys=4 health=1 .*balance=46 spawned=26 enemies=0 .*kills=6',end)
assert (root/'completion.save').read_text().splitlines()==['DL6','MENU','C:1']
print('PASS: clock-aware ordinary-input earned ending;421 unique authored IDs;375 debit;4 keys;natural deflection;26 legitimate spawns;EndStory58657;not physical/live-clock acceptance')
PY

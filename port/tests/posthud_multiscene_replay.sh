#!/bin/sh
# Functional offscreen exact-pixel acceptance gate; not physical GPU proof.
set -eu
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
out=${1:-port/build/posthud-multiscene}
mkdir -p "$out"
for case_spec in forest_combat:180:B:180 waterfall_fade:416:R:416 waterfall_active:501:R:501 waterfall_bottom:781:R:781; do
  name=${case_spec%%:*}; rest=${case_spec#*:}; frames=${rest%%:*}; rest=${rest#*:}
  key=${rest%%:*}; count=${rest#*:}
  prefix="$out/$name"
  if [ "${DEATHLEAK_VERIFY_ONLY:-0}" != 1 ]; then
  port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script "$key:$count" --frames "$frames" --fixed-dt 0.0166667 \
    --test-film-clock-step-ms 100 --resolution 1280x720 --capture "$prefix.bmp" \
    --combat-state > "$prefix.ordinary.log" 2>&1
  port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script "$key:$count" --frames "$frames" --fixed-dt 0.0166667 \
    --test-film-clock-step-ms 100 --resolution 1280x720 --capture "$prefix.repeat.bmp" \
    --repeat-presentation-check --combat-state > "$prefix.repeat.log" 2>&1
  fi
  python3 - "$prefix" <<'PY'
import hashlib, pathlib, re, sys
p=pathlib.Path(sys.argv[1]); paths=[p.with_suffix('.bmp'),pathlib.Path(str(p)+'.repeat.bmp'),pathlib.Path(str(p)+'.repeat.bmp.repeat.bmp')]
blobs=[x.read_bytes() for x in paths]
assert all(x[:2]==b'BM' and int.from_bytes(x[18:22],'little')==1280 and int.from_bytes(x[22:26],'little')==720 and int.from_bytes(x[28:30],'little')==32 for x in blobs)
assert blobs[0]==blobs[1]==blobs[2], f'nonidentical post-HUD BMPs: {paths}'
logs=[pathlib.Path(str(p)+'.ordinary.log').read_text(),pathlib.Path(str(p)+'.repeat.log').read_text()]
for tag in ('Trigger at frame','Simulation ended at','Player/NPC contacts=','Melee active=','Deflection unlocked='):
    def lines(s): return [re.sub(r' elapsed=[0-9.]+s', ' elapsed=<ignored>', x) for x in s.splitlines() if x.startswith(tag)]
    assert lines(logs[0])==lines(logs[1]), f'{tag} endpoint/event mismatch'
assert 'PRESENTATION_REPEAT ' in logs[1] and 'PRESENTATION_REPEAT ' not in logs[0]
scene={'forest_combat':'forest_exploration_1.xml','waterfall_fade':'waterfall_top.xml','waterfall_active':'waterfall_top.xml','waterfall_bottom':'waterfall_bottom.xml'}[p.name]
assert f'Simulation ended at Game/XmlFiles/levels/{scene}' in logs[0]
if p.name=='forest_combat': assert re.search(r' fired=[1-9][0-9]* ', logs[0])
if p.name!='forest_combat': assert 'Trigger at frame 398 from' in logs[0]
if p.name=='waterfall_bottom': assert 'Trigger at frame 762 from' in logs[0]
print(p.name, '1280x720 0/921600 changed pixels', hashlib.sha256(blobs[0]).hexdigest(), 'route/endpoints identical')
PY
done

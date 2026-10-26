#!/bin/sh
# Test-only offscreen functional gate; no physical refresh/GPU acceptance.
set -eu
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
out=${1:-port/build/fresh-refresh-slots}
mkdir -p "$out"
for hz in 60 120 144; do
  extra=
  if [ "$hz" != 60 ]; then extra=--full-frame-topology-check; fi
  port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script R:120 --frames 80 --test-refresh-hz "$hz" --refresh-slots \
    --resolution 1280x720 --combat-state $extra >"$out/$hz.log" 2>&1
done
python3 - "$out" <<'PY'
import pathlib,re,sys
p=pathlib.Path(sys.argv[1]); logs={hz:(p/f'{hz}.log').read_text() for hz in (60,120,144)}
def endpoint(s):
    lines=[x for x in s.splitlines() if x.startswith(('Simulation ended at','Player/NPC contacts=','Melee active=','Deflection unlocked='))]
    return [re.sub(r' elapsed=[0-9.]+s',' elapsed=<wall>',x) for x in lines]
base=endpoint(logs[60]); assert len(base)==4
for hz in (120,144):
    data=logs[hz]
    assert endpoint(data)==base, f'simulation/combat endpoint differs at {hz} Hz'
    assert 'MIXED_COMMAND_TOPOLOGY frame=63 sampled=60' in data, f'full post-HUD motion not admitted at {hz} Hz'
    m=re.search(r'Refresh-only presents=(\d+) fresh_presents=(\d+) surface_failures=(\d+)',data)
    assert m and int(m[1])>0 and int(m[2])>0 and int(m[3])==0, f'fresh display slots absent or surface unstable at {hz} Hz'
    assert f'TEST_REFRESH_HZ={hz}' in data
    print(f'synthetic {hz}-Hz full-packet display: {m[2]} fresh / {m[1]} extra presents; fixed gameplay endpoint matches 60-Hz')
PY

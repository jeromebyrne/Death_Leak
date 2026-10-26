#!/bin/sh
# Test-only offscreen functional gate; no physical refresh/GPU acceptance.
set -eu
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
out=${1:-port/build/fresh-refresh-slots}
mkdir -p "$out"
for hz in 60 120; do
  extra=
  if [ "$hz" = 120 ]; then extra=--full-frame-topology-check; fi
  port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script R:120 --frames 80 --test-refresh-hz "$hz" --refresh-slots \
    --resolution 1280x720 --combat-state $extra >"$out/$hz.log" 2>&1
done
python3 - "$out" <<'PY'
import pathlib,re,sys
p=pathlib.Path(sys.argv[1]); a=(p/'60.log').read_text(); b=(p/'120.log').read_text()
def endpoint(s):
    lines=[x for x in s.splitlines() if x.startswith(('Simulation ended at','Player/NPC contacts=','Melee active=','Deflection unlocked='))]
    return [re.sub(r' elapsed=[0-9.]+s',' elapsed=<wall>',x) for x in lines]
assert len(endpoint(a))==4 and endpoint(a)==endpoint(b), 'simulation/combat endpoint differs by display cadence'
assert 'MIXED_COMMAND_TOPOLOGY frame=63 sampled=60' in b, 'live full post-HUD motion was not admitted'
m=re.search(r'Refresh-only presents=(\d+) fresh_presents=(\d+) surface_failures=(\d+)',b)
assert m and int(m[1])>0 and int(m[2])>0 and int(m[3])==0, 'fresh display slots absent or surface unstable'
assert 'TEST_REFRESH_HZ=120' in b
print(f'synthetic 120-Hz full-packet display: {m[2]} fresh / {m[1]} extra presents; fixed gameplay endpoint matches 60-Hz')
PY

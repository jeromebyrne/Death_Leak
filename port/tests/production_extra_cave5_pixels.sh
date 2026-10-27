#!/bin/sh
# Opt-in synthetic production refresh slots in the authored cave 5 scene.
# Actual extra-slot readback is compared with main.cpp's independent sealed-packet replay.
set -eu
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
out=${1:-/tmp/deathleak-production-extra-cave5}
mkdir -p "$out"
prefix="$out/cave_5"
route=R:25
if [ "${DEATHLEAK_VERIFY_ONLY:-0}" != 1 ]; then
  port/build/deathleak-linux --play Game/XmlFiles/levels/underwater_cave_5.xml \
    --script "$route" --frames 22 --simulate-only --combat-state \
    --test-refresh-hz 60 > "$prefix.60.log" 2>&1
  port/build/deathleak-linux --play Game/XmlFiles/levels/underwater_cave_5.xml \
    --script "$route" --frames 22 --test-refresh-hz 144 --refresh-slots \
    --resolution 1280x720 --combat-state \
    --capture-refresh-extra-prefix "$prefix" --capture-refresh-extra-frame 15 > "$prefix.144.log" 2>&1
fi
python3 - "$prefix" <<'PY'
import hashlib,pathlib,re,sys
p=pathlib.Path(sys.argv[1]); a=pathlib.Path(str(p)+'.60.log').read_text(); b=pathlib.Path(str(p)+'.144.log').read_text()
tags=('Trigger at frame','Door at frame','Simulation ended at','Player/NPC contacts=','Melee active=','Deflection unlocked=','Enemy ','Shot ')
def state(s): return [re.sub(r' elapsed=[0-9.]+s',' elapsed=<wall>',x) for x in s.splitlines() if x.startswith(tags)]
assert state(a)==state(b), 'synthetic rate changed gameplay/transition/combat endpoint'
assert 'Simulation ended at Game/XmlFiles/levels/underwater_cave_5.xml' in b and 'Player died' not in b
assert 'Loaded ' in b and 'original level sprites' in b
rows=re.findall(r'REFRESH_EXTRA_CAPTURE fixed_frame=(\d+) slot=(\d+) alpha=([0-9.]+) actual_sampled=(\d+) expected_sampled=(\d+)',b)
assert rows, 'no production cave 5 extra slot captured'
assert 'surface_failures=0' in b
for f,slot,alpha,sampled,expected in rows:
    assert int(f)==15 and int(sampled)==int(expected) and int(sampled)>0, (f,slot,sampled,expected)
    actual,oracle,held=[pathlib.Path(f'{p}.{slot}.{arm}.bmp').read_bytes() for arm in ('actual','expected','held')]
    assert all(x[:2]==b'BM' and int.from_bytes(x[18:22],'little')==1280 and int.from_bytes(x[22:26],'little')==720 and int.from_bytes(x[28:30],'little')==32 for x in (actual,oracle,held))
    assert actual==oracle and actual!=held, f'cave 5 extra slot {slot}: full post-HUD oracle parity/distinctness failed'
    print('cave_5','frame',f,'slot',slot,'alpha',alpha,'sampled',sampled,'post-HUD exact',hashlib.sha256(actual).hexdigest())
print('cave 5 synthetic 60-Hz simulation and 144-Hz production-present endpoints identical')
PY

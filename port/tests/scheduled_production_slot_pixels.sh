#!/bin/sh
# Controlled clock enters the same SealFreshBoundary/TryPresent branch as live production.
# Offscreen software only; not physical refresh-rate/FPS evidence.
set -eu
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
out=${1:-port/build/scheduled-production-slot}
mkdir -p "$out"
prefix="$out/forest"
if [ "${DEATHLEAK_VERIFY_ONLY:-0}" != 1 ]; then
  port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script R:120 --frames 80 --simulate-only --combat-state --test-refresh-hz 60 > "$prefix.60.log" 2>&1
  port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script R:120 --frames 80 --test-refresh-hz 144 --refresh-slots --test-scheduled-refresh-slots \
    --verify-refresh-side-effects --resolution 1280x720 --combat-state \
    --capture-refresh-extra-prefix "$prefix" --capture-refresh-extra-first-admitted-after 20 > "$prefix.144.log" 2>&1
  port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script R:120 --frames 24 --test-refresh-hz 30 --refresh-slots --test-scheduled-refresh-slots \
    --verify-refresh-side-effects --resolution 1280x720 > "$prefix.30.log" 2>&1
fi
python3 - "$prefix" <<'PY'
import hashlib,pathlib,re,sys
p=pathlib.Path(sys.argv[1]); a=pathlib.Path(str(p)+'.60.log').read_text(); b=pathlib.Path(str(p)+'.144.log').read_text(); c=pathlib.Path(str(p)+'.30.log').read_text()
tags=('Trigger at frame','Door at frame','Simulation ended at','Player/NPC contacts=','Melee active=','Deflection unlocked=','Enemy ','Shot ')
def state(s): return [re.sub(r' elapsed=[0-9.]+s',' elapsed=<wall>',x) for x in s.splitlines() if x.startswith(tags)]
assert state(a)==state(b), 'scheduled present changed fixed gameplay/combat endpoint'
assert 'Player died' not in b and 'surface_failures=0' in b and 'surface_failures=0' in c
for hz,s in ((144,b),(30,c)):
    m=re.search(r'TEST_SCHEDULED_REFRESH boundary_presents=(\d+) boundary_not_due=(\d+)',s)
    assert m and int(m[1])>0, (hz,'production scheduled branch not entered')
    n=re.search(r'REFRESH_SIDE_EFFECTS invariant_slots=(\d+)',s)
    service=re.search(r'Refresh-only presents=(\d+) fresh_presents=(\d+) surface_failures=(\d+)',s)
    assert n and service and int(n[1])+int(m[1])==int(service[1]) and int(service[3])==0
    if hz==144: assert int(n[1])>0 and int(service[2])>0, 'no fresh due update-free slot'
    print(hz,'boundary presented/not_due',m[1],m[2],'update-free',n[1],'fresh',service[2])
rows=re.findall(r'REFRESH_EXTRA_CAPTURE fixed_frame=(\d+) slot=(\d+) alpha=([0-9.]+) actual_sampled=(\d+) expected_sampled=(\d+)',b)
assert rows and any(int(x[3])>0 and 0<float(x[2])<1 for x in rows), 'no fractional admitted update-free production present captured'
for f,slot,alpha,sampled,expected in rows:
    assert int(f)>=20 and int(sampled)==int(expected)
    actual,oracle,held=[pathlib.Path(f'{p}.{slot}.{arm}.bmp').read_bytes() for arm in ('actual','expected','held')]
    assert all(x[:2]==b'BM' and int.from_bytes(x[18:22],'little')==1280 and int.from_bytes(x[22:26],'little')==720 and int.from_bytes(x[28:30],'little')==32 for x in (actual,oracle,held))
    assert len(actual)==len(oracle)==len(held) and actual==oracle
    if int(sampled)>0: assert 0<float(alpha)<1 and actual!=held
    else: assert actual==held
    changed=sum(actual[i:i+4]!=held[i:i+4] for i in range(54,len(actual),4))
    print('scheduled production extra frame',f,'slot',slot,'alpha',alpha,'sampled',sampled,'changed',changed,'sha256',hashlib.sha256(actual).hexdigest())
print('PASS: actual scheduled branch, full post-HUD pixels, purity, exact fixed endpoint')
PY

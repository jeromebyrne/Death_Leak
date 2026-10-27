#!/bin/sh
# Forest normal-control route into cave 6; actual synthetic production extra slot.
# Offscreen software pixels are correctness evidence, never physical-Hz/FPS evidence.
set -eu
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
out=${1:-port/build/uninterrupted-cave6-fresh-slot}
mkdir -p "$out"
prefix="$out/cave6"
route=R:1100,Q:40,R:900
if [ "${DEATHLEAK_VERIFY_ONLY:-0}" != 1 ]; then
  port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script "$route" --frames 1880 --simulate-only --combat-state \
    --test-refresh-hz 60 > "$prefix.60.log" 2>&1
  port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script "$route" --frames 1880 --test-refresh-hz 144 --refresh-slots \
    --verify-refresh-side-effects --resolution 1280x720 --combat-state \
    --capture-refresh-extra-prefix "$prefix" \
    --capture-refresh-extra-first-admitted-after 1800 > "$prefix.144.log" 2>&1
fi
python3 - "$prefix" <<'PY'
import hashlib,pathlib,re,sys
p=pathlib.Path(sys.argv[1]); a=pathlib.Path(str(p)+'.60.log').read_text(); b=pathlib.Path(str(p)+'.144.log').read_text()
tags=('Trigger at frame','Door at frame','Simulation ended at','Player/NPC contacts=','Melee active=','Deflection unlocked=','Enemy ','Shot ')
def state(s): return [re.sub(r' elapsed=[0-9.]+s',' elapsed=<wall>',x) for x in s.splitlines() if x.startswith(tags)]
assert state(a)==state(b), 'synthetic refresh altered fixed gameplay/combat/transition endpoint'
transitions=((398,'forest_exploration_1.xml','waterfall_top.xml'),(762,'waterfall_top.xml','waterfall_bottom.xml'),(1378,'waterfall_bottom.xml','underwater_cave.xml'),(1780,'underwater_cave.xml','underwater_cave_6.xml'))
for n,src,dst in transitions:
    assert re.search(rf'Trigger at frame {n} from .*{src} .*{dst}',b), (n,src,dst)
assert 'Simulation ended at Game/XmlFiles/levels/underwater_cave_6.xml' in b and 'Player died' not in b
assert 'surface_failures=0' in b
m=re.search(r'REFRESH_SIDE_EFFECTS invariant_slots=(\d+)',b)
assert m and int(m[1])>0, 'no update-free production slots had fixed-state purity checked'
service=re.search(r'Refresh-only presents=(\d+) fresh_presents=(\d+) surface_failures=(\d+)',b)
assert service and int(service[1])==int(m[1]) and int(service[2])>0 and int(service[3])==0, 'not every presented extra slot was checked'
# Scene-change audio dispatch is fixed-update-owned; extra display slots may not duplicate it.
audio=re.findall(r'^Audio level (Game/XmlFiles/levels/[^ ]+) authored_ambience=',b,re.M)
assert [pathlib.Path(x).name for x in audio]==['forest_exploration_1.xml','waterfall_top.xml','waterfall_bottom.xml','underwater_cave.xml','underwater_cave_6.xml'],audio
rows=re.findall(r'REFRESH_EXTRA_CAPTURE fixed_frame=(\d+) slot=(\d+) alpha=([0-9.]+) actual_sampled=(\d+) expected_sampled=(\d+)',b)
assert rows, 'no admitted production extra slot in cave 6'
frames={int(x[0]) for x in rows}; assert len(frames)==1 and all(1800<=f<1880 for f in frames),frames
for f,slot,alpha,sampled,expected in rows:
    assert int(sampled)==int(expected)>0 and 0<float(alpha)<1,(f,slot,alpha,sampled,expected)
    actual,oracle,held=[pathlib.Path(f'{p}.{slot}.{arm}.bmp').read_bytes() for arm in ('actual','expected','held')]
    for img in (actual,oracle,held):
        assert img[:2]==b'BM' and int.from_bytes(img[18:22],'little')==1280 and int.from_bytes(img[22:26],'little')==720 and int.from_bytes(img[28:30],'little')==32
    assert len(actual)==len(oracle)==len(held) and actual==oracle and actual!=held, 'full post-HUD pixel oracle/distinctness failed'
    changed=sum(actual[i:i+4]!=held[i:i+4] for i in range(54,len(actual),4))
    assert changed>0
    print('cave6 route frame',f,'slot',slot,'alpha',alpha,'sampled',sampled,'changed_vs_held',changed,'post-HUD sha256',hashlib.sha256(actual).hexdigest())
print('PASS: four authored transitions; exact 60/144-Hz fixed endpoint; fixed-state purity across',m[1],'extra slots; one audio dispatch per scene')
PY

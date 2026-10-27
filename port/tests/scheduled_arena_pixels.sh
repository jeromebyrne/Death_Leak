#!/bin/sh
# Uninterrupted Forest-to-first-ninja-arena combat; actual scheduled production branch.
# Offscreen software pixels are correctness evidence, never physical-Hz/FPS evidence.
set -eu
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
out=${1:-port/build/scheduled-arena}
mkdir -p "$out"
prefix="$out/arena"
route=B:1100,Q:40,R:641,J:200,E:2200,L:48,I:10,L:1500,Q:40,L:100,F:300,R:300,F:300
if [ "${DEATHLEAK_VERIFY_ONLY:-0}" != 1 ]; then
  port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script "$route" --frames 5900 --simulate-only --combat-state \
    --test-refresh-hz 60 > "$prefix.60.log" 2>&1
  port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script "$route" --frames 5900 --test-refresh-hz 144 --refresh-slots \
    --test-scheduled-refresh-slots --verify-refresh-side-effects --resolution 640x360 --combat-state \
    --capture-refresh-extra-prefix "$prefix" \
    --capture-refresh-extra-first-admitted-after 5390 > "$prefix.144.log" 2>&1
fi
python3 - "$prefix" <<'PY'
import hashlib,pathlib,re,sys
p=pathlib.Path(sys.argv[1]); a=pathlib.Path(str(p)+'.60.log').read_text(); b=pathlib.Path(str(p)+'.144.log').read_text()
tags=('Trigger at frame','Door at frame','Simulation ended at','Player/NPC contacts=','Melee active=','Deflection unlocked=','Enemy ','Shot ')
def state(s): return [re.sub(r' elapsed=[0-9.]+s',' elapsed=<wall>',x) for x in s.splitlines() if x.startswith(tags)]
assert state(a)==state(b), 'synthetic refresh altered fixed gameplay/combat/transition endpoint'
transitions=((398,'forest_exploration_1.xml','waterfall_top.xml'),(762,'waterfall_top.xml','waterfall_bottom.xml'),(1378,'waterfall_bottom.xml','underwater_cave.xml'),(1780,'underwater_cave.xml','underwater_cave_6.xml'),(3224,'underwater_cave_6.xml','hot_springs.xml'),(3377,'hot_springs.xml','underwater_cave_4.xml'),(3602,'underwater_cave_4.xml','underwater_cave_3.xml'),(3637,'underwater_cave_3.xml','underwater_cave_4.xml'),(3868,'underwater_cave_4.xml','underwater_cave_5.xml'),(4590,'forest_exploration_1.xml','grass_exploration_3.xml'),(5383,'grass_exploration_3.xml','first_ninja_arena.xml'))
for n,src,dst in transitions:
    assert re.search(rf'Trigger at frame {n} from .*{src} .*{dst}',b), (n,src,dst)
assert re.search(r'Door at frame 4231 from .*underwater_cave_5\.xml to .*forest_exploration_1\.xml',b)
assert re.search(r'Simulation ended at Game/XmlFiles/levels/first_ninja_arena\.xml .*health=22 .*enemies=2 hits=2 kills=0 fired=67',b) and 'Player died' not in b
assert 'surface_failures=0' in b
m=re.search(r'REFRESH_SIDE_EFFECTS invariant_slots=(\d+)',b)
assert m and int(m[1])>0, 'no update-free production slots had fixed-state purity checked'
service=re.search(r'Refresh-only presents=(\d+) fresh_presents=(\d+) surface_failures=(\d+)',b)
boundary=re.search(r'TEST_SCHEDULED_REFRESH boundary_presents=(\d+) boundary_not_due=(\d+)',b)
assert boundary and int(boundary[1])>0, 'scheduled production boundary path absent'
assert service and int(service[1])==int(m[1])+int(boundary[1]) and int(service[2])>0 and int(service[3])==0, 'scheduled boundary/update-free accounting failed'
# Scene-change audio dispatch is fixed-update-owned; extra display slots may not duplicate it.
audio=re.findall(r'^Audio level (Game/XmlFiles/levels/[^ ]+) authored_ambience=',b,re.M)
assert [pathlib.Path(x).name for x in audio]==['forest_exploration_1.xml','waterfall_top.xml','waterfall_bottom.xml','underwater_cave.xml','underwater_cave_6.xml','hot_springs.xml','underwater_cave_4.xml','underwater_cave_3.xml','underwater_cave_4.xml','underwater_cave_5.xml','forest_exploration_1.xml','grass_exploration_3.xml','first_ninja_arena.xml'],audio
rows=re.findall(r'REFRESH_EXTRA_CAPTURE fixed_frame=(\d+) slot=(\d+) alpha=([0-9.]+) actual_sampled=(\d+) expected_sampled=(\d+)',b)
assert rows, 'no admitted production extra slot in first-ninja arena'
frames={int(x[0]) for x in rows}; assert len(frames)==1 and all(5390<=f<5900 for f in frames),frames
for f,slot,alpha,sampled,expected in rows:
    assert int(sampled)==int(expected),(f,slot,alpha,sampled,expected)
    actual,oracle,held=[pathlib.Path(f'{p}.{slot}.{arm}.bmp').read_bytes() for arm in ('actual','expected','held')]
    for img in (actual,oracle,held):
        assert img[:2]==b'BM' and int.from_bytes(img[18:22],'little')==640 and int.from_bytes(img[22:26],'little')==360 and int.from_bytes(img[28:30],'little')==32
    assert len(actual)==len(oracle)==len(held) and actual==oracle, 'full post-HUD pixel oracle failed'
    if int(sampled)>0: assert actual!=held, 'admitted slot not distinct'
    else: assert actual==held, 'held slot changed pixels'
    changed=sum(actual[i:i+4]!=held[i:i+4] for i in range(54,len(actual),4))
    assert changed>0 if int(sampled)>0 else changed==0
    print('Arena route frame',f,'slot',slot,'alpha',alpha,'sampled',sampled,'changed_vs_held',changed,'post-HUD sha256',hashlib.sha256(actual).hexdigest())
assert any(int(row[3])>0 and 0<float(row[2])<1 for row in rows), 'no fractional due update-free arena present'
print('PASS: eleven authored triggers, cave-5 door, arena combat; exact 60/144-Hz fixed endpoint; scheduled boundary',boundary[1],'update-free purity',m[1],'one audio dispatch per scene')
PY

#!/bin/sh
# Opt-in production refresh service, synthetic offscreen slots only.
# Independent sealed-packet replay is captured by main.cpp at the identical slot.
set -eu
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
out=${1:-/tmp/deathleak-production-extra-later-scenes}
mkdir -p "$out"
for spec in waterfall_active:510:501:waterfall_top.xml:R:510 cave6:1782:1781:underwater_cave_6.xml:R:1100,Q:40,R:640,E:1500,R:1300 arena_damage:1465:1464:first_ninja_arena.xml:L:1500,Q:40,L:100,F:300; do
  name=${spec%%:*}; rest=${spec#*:}; frames=${rest%%:*}; rest=${rest#*:}
  sample=${rest%%:*}; rest=${rest#*:}; scene=${rest%%:*}; route=${rest#*:}
  prefix="$out/$name"
  if [ "${DEATHLEAK_VERIFY_ONLY:-0}" != 1 ]; then
    port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
      --script "$route" --frames "$frames" --simulate-only --combat-state \
      --test-refresh-hz 60 > "$prefix.60.log" 2>&1
    port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
      --script "$route" --frames "$frames" --test-refresh-hz 144 --refresh-slots \
      --resolution 1280x720 --combat-state \
      --capture-refresh-extra-prefix "$prefix" --capture-refresh-extra-frame "$sample" > "$prefix.144.log" 2>&1
  fi
  python3 - "$prefix" "$sample" "$scene" <<'PY'
import hashlib,pathlib,re,sys
p=pathlib.Path(sys.argv[1]); frame=int(sys.argv[2]); scene=sys.argv[3]
a=pathlib.Path(str(p)+'.60.log').read_text(); b=pathlib.Path(str(p)+'.144.log').read_text()
tags=('Trigger at frame','Door at frame','Simulation ended at','Player/NPC contacts=','Melee active=','Deflection unlocked=','Enemy ','Shot ')
def state(s):
    return [re.sub(r' elapsed=[0-9.]+s',' elapsed=<wall>',line) for line in s.splitlines() if line.startswith(tags)]
assert state(a)==state(b), f'{p.name}: synthetic rate changed gameplay/transition/combat endpoint'
assert f'Simulation ended at Game/XmlFiles/levels/{scene}' in b
assert 'Player died' not in b
for n,src,dst in ((398,'forest_exploration_1.xml','waterfall_top.xml'),(762,'waterfall_top.xml','waterfall_bottom.xml'),(1378,'waterfall_bottom.xml','underwater_cave.xml')):
    if p.name=='cave6': assert re.search(rf'Trigger at frame {n} from .*{src} .*{dst}',b)
if p.name=='waterfall_active':
    assert 'Trigger at frame 398 from' in b
if p.name=='cave6':
    assert 'Trigger at frame 1780 from' in b
if p.name=='arena_damage':
    assert 'Trigger at frame 300 from' in b and 'Trigger at frame 1093 from' in b
    assert re.search(r'\bhits=1\b',b) and re.search(r'\bhealth=22\b',b)
rows=re.findall(r'REFRESH_EXTRA_CAPTURE fixed_frame=(\d+) slot=(\d+) alpha=([0-9.]+) actual_sampled=(\d+) expected_sampled=(\d+)',b)
assert rows, f'{p.name}: no actual production extra slot captured'
if p.name=='waterfall_active':
    assert any(int(sampled)>0 for _,_,_,sampled,_ in rows), 'Waterfall active slot never admitted motion'
for f,slot,alpha,sampled,expected_sampled in rows:
    assert int(f)==frame and sampled==expected_sampled
    paths=[pathlib.Path(str(p)+f'.{slot}.{arm}.bmp') for arm in ('actual','expected','held')]
    actual,expected,held=[x.read_bytes() for x in paths]
    assert all(x[:2]==b'BM' and int.from_bytes(x[18:22],'little')==1280 and int.from_bytes(x[22:26],'little')==720 and int.from_bytes(x[28:30],'little')==32 for x in (actual,expected,held))
    assert actual==expected, f'{p.name} slot {slot}: actual does not match independent sealed-packet oracle'
    if int(sampled)==0: assert actual==held, f'{p.name} slot {slot}: failed admission did not hold current'
    else: assert actual!=held, f'{p.name} slot {slot}: admitted motion did not change pixels'
    print(p.name,'frame',f,'slot',slot,'alpha',alpha,'sampled',sampled,'post-HUD exact',hashlib.sha256(actual).hexdigest())
print(p.name,'synthetic 60-Hz simulation and 144-Hz rendered gameplay/combat/transition endpoints identical')
PY
done

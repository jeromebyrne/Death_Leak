#!/bin/sh
# Actual opt-in production extra-slot pixel oracle across active/fallback routes.
# Software/synthetic correctness only; no physical cadence or FPS conclusion.
set -eu
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
out=${1:-/tmp/deathleak-production-extra-multiscene}
mkdir -p "$out"
for spec in forest:64:80:R:120:fresh combat:80:180:B:180:hold entry:416:430:R:430:hold waterfall:501:510:R:510:fresh arena_hit:1464:1470:L:1500,Q:40,L:100,F:300:hold; do
  name=${spec%%:*}; rest=${spec#*:}; sample=${rest%%:*}; rest=${rest#*:}; frames=${rest%%:*}; rest=${rest#*:}; route=${rest%:*}; expect=${rest##*:}
  prefix="$out/$name"
  port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script "$route" --frames "$frames" --test-refresh-hz 144 --refresh-slots \
    --resolution 1280x720 --combat-state --capture-refresh-extra-prefix "$prefix" \
    --capture-refresh-extra-frame "$sample" > "$prefix.log" 2>&1
  python3 - "$prefix" "$sample" "$expect" <<'PY'
import hashlib,pathlib,re,sys
p=pathlib.Path(sys.argv[1]); frame=int(sys.argv[2]); expect=sys.argv[3]
s=p.with_suffix('.log').read_text(); rows=re.findall(r'REFRESH_EXTRA_CAPTURE fixed_frame=(\d+) slot=(\d+) alpha=([0-9.]+) actual_sampled=(\d+) expected_sampled=(\d+)',s)
assert rows, (p,'no actual production extra slot')
for f,slot,alpha,sampled,expected in rows:
    assert int(f)==frame and sampled==expected, (p,rows)
    actual=pathlib.Path(f'{p}.{slot}.actual.bmp').read_bytes(); oracle=pathlib.Path(f'{p}.{slot}.expected.bmp').read_bytes(); held=pathlib.Path(f'{p}.{slot}.held.bmp').read_bytes()
    assert actual[:2]==b'BM' and int.from_bytes(actual[18:22],'little')==1280 and int.from_bytes(actual[22:26],'little')==720 and int.from_bytes(actual[28:30],'little')==32
    assert len(actual)==len(oracle)==len(held) and actual==oracle, (p,slot,'full post-HUD pixel mismatch')
    if expect=='hold': assert int(sampled)==0 and actual==held, (p,slot,'fallback changed pixels')
    else: assert int(sampled)>0 and actual!=held, (p,slot,'fresh slot not distinct')
    print(p.name, 'slot',slot,'alpha',alpha,'sampled',sampled,'pixel_sha256',hashlib.sha256(actual).hexdigest())
assert 'surface_failures=0' in s and 'Player died' not in s
end=next(x for x in s.splitlines() if x.startswith('Simulation ended at'))
if p.name=='entry' or p.name=='waterfall': assert 'waterfall_top.xml' in end and 'Trigger at frame 398 from' in s
if p.name=='arena_hit': assert 'first_ninja_arena.xml' in end and re.search(r'\bhits=1\b',end)
if p.name=='combat': assert re.search(r'\bfired=[1-9]',end)
PY
done

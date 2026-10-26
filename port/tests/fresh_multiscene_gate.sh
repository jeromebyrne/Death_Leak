#!/bin/sh
# Offscreen functional gate for fresh full post-HUD packets and production refresh slots.
# Synthetic slots/llvmpipe are not physical selected-mode or FPS evidence.
set -eu
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
out=${1:-port/build/fresh-multiscene-gate}
mkdir -p "$out"
# The sampled tick is the final rendered tick. Routes enter scenes through authored
# ordinary controls, including a real arena damage event.
for spec in forest_motion:64:63:R:120 waterfall_entry:416:415:R:416 waterfall_active:501:500:R:520 arena_hit:1464:1463:L:1500,Q:40,L:100,F:300; do
  name=${spec%%:*}; rest=${spec#*:}; frames=${rest%%:*}; rest=${rest#*:}
  sample=${rest%%:*}; route=${rest#*:}; prefix="$out/$name"
  if [ "${DEATHLEAK_VERIFY_ONLY:-0}" != 1 ]; then
    port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
      --script "$route" --frames "$frames" --fixed-dt 0.0166667 \
      --test-film-clock-step-ms 100 --resolution 1280x720 \
      --capture "$prefix.bmp" --combat-state --mixed-replay-prefix "$prefix.fresh" \
      --mixed-replay-frame "$sample" > "$prefix.log" 2>&1
  fi
  python3 - "$prefix" "$sample" <<'PY'
import hashlib,pathlib,re,sys
p=pathlib.Path(sys.argv[1]); frame=int(sys.argv[2]); name=p.name
paths=[pathlib.Path(str(p)+suffix) for suffix in ('.bmp','.fresh.current.bmp','.fresh.alpha1.bmp','.fresh.alpha05.bmp','.fresh.alpha05again.bmp')]
blobs=[path.read_bytes() for path in paths]
for blob in blobs:
    assert blob[:2]==b'BM' and int.from_bytes(blob[18:22],'little')==1280 and int.from_bytes(blob[22:26],'little')==720 and int.from_bytes(blob[28:30],'little')==32
assert blobs[0]==blobs[1]==blobs[2], f'{name}: alpha1 differs from exact final post-HUD ordinary output'
assert blobs[3]==blobs[4], f'{name}: midpoint changes between same-alpha samples'
s=p.with_suffix('.log').read_text()
assert f'MIXED_REPLAY frame={frame} label=alpha05' in s
assert f'FRESH_PRESENTATION frame={frame} sampled=' in s
end=next(line for line in s.splitlines() if line.startswith('Simulation ended at'))
expected={'forest_motion':'forest_exploration_1.xml','waterfall_entry':'waterfall_top.xml','waterfall_active':'waterfall_top.xml','arena_hit':'first_ninja_arena.xml'}[name]
assert expected in end and 'health=0 ' not in end and 'Player died' not in s, (name,end)
if name.startswith('waterfall'):
    assert 'Trigger at frame 398 from' in s
if name=='arena_hit':
    assert 'Trigger at frame 300 from' in s and 'Trigger at frame 1093 from' in s
    assert re.search(r'\bhits=1\b',end),end
    assert re.search(r'\bhealth=22\b',end),end
changed=sum(blobs[1][i:i+4]!=blobs[3][i:i+4] for i in range(54,len(blobs[1]),4))
if name in ('forest_motion','waterfall_active'):
    assert changed>100, f'{name}: admitted motion did not alter midpoint pixels'
else:
    assert changed==0, f'{name}: scene cut or combat topology must hold current packet'
print(name, 'alpha1 exact 0/921600 changed pixels; midpoint stable; midpoint delta',changed,'sha256',hashlib.sha256(blobs[0]).hexdigest())
PY
done
# Exercise actual opt-in service, not just the diagnostic sampler. Avoid wall-time
# assertions: software OpenGL may miss synthetic slot deadlines.
for spec in combat:180:B:180 transition:416:R:416; do
  name=${spec%%:*}; rest=${spec#*:}; frames=${rest%%:*}; route=${rest#*:}
  for hz in 60 144; do
    if [ "${DEATHLEAK_VERIFY_ONLY:-0}" != 1 ]; then
      port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
        --script "$route" --frames "$frames" --test-refresh-hz "$hz" --refresh-slots \
        --resolution 1280x720 --combat-state > "$out/$name.$hz.log" 2>&1
    fi
  done
  python3 - "$out" "$name" <<'PY'
import pathlib,re,sys
root=pathlib.Path(sys.argv[1]); name=sys.argv[2]
a=(root/f'{name}.60.log').read_text(); b=(root/f'{name}.144.log').read_text()
tags=('Trigger at frame','Door at frame','Simulation ended at','Player/NPC contacts=','Melee active=','Deflection unlocked=','Enemy ','Shot ')
def state(s):
    return [re.sub(r' elapsed=[0-9.]+s',' elapsed=<wall>',line) for line in s.splitlines() if line.startswith(tags)]
assert state(a)==state(b), f'{name}: fixed-60 gameplay/combat/transition endpoint differs across slots'
assert 'Player died' not in b
if name=='combat': assert re.search(r'\bfired=[1-9][0-9]*\b',next(x for x in b.splitlines() if x.startswith('Simulation ended at')))
if name=='transition': assert 'Trigger at frame 398 from' in b
m=re.search(r'Refresh-only presents=(\d+) fresh_presents=(\d+) surface_failures=(\d+)',b)
assert m and int(m[1])>0 and int(m[2])>0 and int(m[3])==0, f'{name}: no stable fresh production refresh slots'
assert 'TEST_REFRESH_HZ=144' in b and 'TEST_REFRESH_HZ=60' in a
print(name, 'synthetic 144-Hz production refresh',m[2],'fresh /',m[1],'extra; exact fixed endpoint vs 60-Hz')
PY
done

#!/bin/sh
# Functional offscreen gate; never evidence of a physical selected high-Hz mode.
set -eu
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
out=${1:-port/build/fresh-posthud-packet}
mkdir -p "$out"
for arm in ordinary sampled; do
  extra=
  if [ "$arm" = sampled ]; then extra="--mixed-replay-prefix $out/fresh --mixed-replay-frame 63"; fi
  port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script R:120 --frames 80 --fixed-dt 0.0166667 --test-film-clock-step-ms 1 \
    --resolution 1280x720 --capture "$out/$arm.bmp" --combat-state $extra >"$out/$arm.log" 2>&1
done
python3 - "$out" <<'PY'
import pathlib, re, sys
p=pathlib.Path(sys.argv[1])
images={n:(p/(n+'.bmp')).read_bytes() for n in ('ordinary','sampled','fresh.current','fresh.alpha1','fresh.alpha05','fresh.alpha05again')}
for name,b in images.items():
    assert b[:2]==b'BM' and int.from_bytes(b[18:22],'little')==1280 and int.from_bytes(b[22:26],'little')==720 and int.from_bytes(b[28:30],'little')==32,name
assert images['ordinary']==images['sampled'], 'diagnostic replay changed final ordinary post-HUD frame'
assert images['fresh.current']==images['fresh.alpha1'], 'alpha=1 not exact ordinary frame'
assert images['fresh.alpha05']==images['fresh.alpha05again'], 'same-alpha replay changed pixels'
a,b=images['fresh.current'],images['fresh.alpha05']
offset=int.from_bytes(a[10:14],'little')
changed=sum(a[i:i+4]!=b[i:i+4] for i in range(offset,len(a),4))
assert changed>100, f'no meaningful midpoint motion: {changed} pixels'
assert len(a)==len(b)==offset+1280*720*4
logs=[(p/(n+'.log')).read_text() for n in ('ordinary','sampled')]
keys=('Trigger at frame','Door at frame','Simulation ended at','Player/NPC contacts=','Melee active=','Deflection unlocked=','Enemy ','Shot ')
def state(s):
    return [re.sub(r' elapsed=[0-9.]+s',' elapsed=<wall-time>',line) for line in s.splitlines() if line.startswith(keys)]
assert state(logs[0])==state(logs[1]), 'gameplay/combat endpoint changed'
assert 'FRESH_PRESENTATION frame=63 sampled=61' in logs[1]
assert 'MIXED_REPLAY frame=63 label=alpha05' in logs[1]
print(f'fresh full post-HUD packet: alpha1 exact, midpoint {changed}/921600 changed pixels, repeat exact, gameplay invariant')
PY

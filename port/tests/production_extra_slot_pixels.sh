#!/bin/sh
# Synthetic offscreen functional gate, never physical high-Hz or FPS evidence.
set -eu
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
out=${1:-/tmp/deathleak-production-extra-slot-pixels}
mkdir -p "$out"
for hz in 60 144; do
  extra=
  if [ "$hz" = 144 ]; then extra="--capture-refresh-extra-prefix $out/forest --capture-refresh-extra-frame 64"; fi
  port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script R:120 --frames 80 --test-refresh-hz "$hz" --refresh-slots \
    --resolution 1280x720 --combat-state $extra > "$out/forest.$hz.log" 2>&1
done
python3 - "$out" <<'PY'
import hashlib,pathlib,re,sys
root=pathlib.Path(sys.argv[1]); a=(root/'forest.60.log').read_text(); b=(root/'forest.144.log').read_text()
tags=('Trigger at frame','Door at frame','Simulation ended at','Player/NPC contacts=','Melee active=','Deflection unlocked=','Enemy ','Shot ')
def fixed_state(s):
    return [re.sub(r' elapsed=[0-9.]+s',' elapsed=<wall>',line) for line in s.splitlines() if line.startswith(tags)]
assert fixed_state(a)==fixed_state(b), 'fixed-step gameplay/combat/transition effects differ'
assert 'Player died' not in b
rows=re.findall(r'REFRESH_EXTRA_CAPTURE fixed_frame=(\d+) slot=(\d+) alpha=([0-9.]+) actual_sampled=(\d+) expected_sampled=(\d+)',b)
assert rows, 'no production extra slot captured'
assert any(int(r[3])>0 for r in rows), 'selected slot did not admit motion'
for frame,slot,alpha,actual_sampled,expected_sampled in rows:
    assert int(frame)==64 and actual_sampled==expected_sampled
    actual=(root/f'forest.{slot}.actual.bmp').read_bytes(); expected=(root/f'forest.{slot}.expected.bmp').read_bytes(); held=(root/f'forest.{slot}.held.bmp').read_bytes()
    assert actual[:2]==expected[:2]==b'BM'
    assert int.from_bytes(actual[18:22],'little')==1280 and int.from_bytes(actual[22:26],'little')==720
    assert len(actual)==len(expected)==len(held) and actual==expected, f'extra slot {slot} changed full post-HUD pixels'
    if int(actual_sampled)>0: assert actual!=held, f'extra slot {slot} did not produce distinct motion pixels'
    print('slot',slot,'alpha',alpha,'sampled',actual_sampled,'full post-HUD pixels exact',hashlib.sha256(actual).hexdigest())
print('fixed-step gameplay/combat/transition effects exact at synthetic 60/144 Hz')
PY

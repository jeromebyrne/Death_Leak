#!/bin/sh
# Run from repository root with a built port/build/deathleak-linux.
# Deterministic CPU progression gate; not GPU/display acceptance.
set -eu
export SDL_VIDEODRIVER=offscreen
export SDL_RENDER_DRIVER=opengl
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
route=$(port/build/deathleak-linux \
  --play Game/XmlFiles/levels/forest_exploration_1.xml \
  --script R:1100,Q:40,R:900 \
  --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$route" | grep -q 'Trigger at frame 398 from .*forest_exploration_1.xml .*waterfall_top.xml'
printf '%s\n' "$route" | grep -q 'Trigger at frame 762 from .*waterfall_top.xml .*waterfall_bottom.xml'
printf '%s\n' "$route" | grep -q 'Trigger at frame 1378 from .*waterfall_bottom.xml .*underwater_cave.xml'
printf '%s\n' "$route" | grep -q 'Trigger at frame 1780 from .*underwater_cave.xml .*underwater_cave_6.xml'
printf '%s\n' "$route" | grep -q 'Simulation ended at Game/XmlFiles/levels/underwater_cave_6.xml .*orbs=15 keys=0 health=25 .*balance=15'
if printf '%s\n' "$route" | grep -q 'Player died'; then
  echo 'Unexpected death on Waterfall-to-cave-6 route' >&2
  exit 1
fi
printf '%s\n' "$route" | grep -E '^(Trigger at frame|Simulation ended)'

# Continue the same normal-control route through cave 6's authored return trigger.
# This is a transition gate, not a claim that the Hot Springs exits are reachable.
return_route=$(port/build/deathleak-linux \
  --play Game/XmlFiles/levels/forest_exploration_1.xml \
  --script R:1100,Q:40,R:900,L:500 \
  --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$return_route" | grep -q 'Trigger at frame 1780 from .*underwater_cave.xml .*underwater_cave_6.xml'
printf '%s\n' "$return_route" | grep -q 'Trigger at frame 2396 from .*underwater_cave_6.xml .*underwater_cave.xml'
printf '%s\n' "$return_route" | grep -q 'Simulation ended at Game/XmlFiles/levels/underwater_cave.xml .*orbs=15 keys=0 health=25 .*balance=15'
if printf '%s\n' "$return_route" | grep -q 'Player died'; then
  echo 'Unexpected death on cave-6 authored return route' >&2
  exit 1
fi
printf '%s\n' "$return_route" | grep -E '^(Trigger at frame|Simulation ended)'

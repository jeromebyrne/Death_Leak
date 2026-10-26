#!/bin/sh
# From repo root with port/build/deathleak-linux built. Fixed 60-Hz normal
# controls only: B = move right and fire, Q = move left and jump.
set -eu
export SDL_VIDEODRIVER=offscreen
export SDL_RENDER_DRIVER=opengl
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
route=$(port/build/deathleak-linux \
  --play Game/XmlFiles/levels/forest_exploration_1.xml \
  --script B:1100,Q:40,R:900,L:1200,R:900 \
  --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$route" | grep -q 'Trigger at frame 398 from .*forest_exploration_1.xml .*waterfall_top.xml'
printf '%s\n' "$route" | grep -q 'Trigger at frame 762 from .*waterfall_top.xml .*waterfall_bottom.xml'
printf '%s\n' "$route" | grep -q 'Trigger at frame 1378 from .*waterfall_bottom.xml .*underwater_cave.xml'
printf '%s\n' "$route" | grep -q 'Trigger at frame 1780 from .*underwater_cave.xml .*underwater_cave_6.xml'
printf '%s\n' "$route" | grep -q 'Trigger at frame 2396 from .*underwater_cave_6.xml .*underwater_cave.xml'
printf '%s\n' "$route" | grep -q 'Trigger at frame 3994 from .*underwater_cave.xml .*underwater_cave_6.xml'
printf '%s\n' "$route" | grep -q 'Simulation ended at Game/XmlFiles/levels/underwater_cave_6.xml .*orbs=15 keys=0 health=25 .*balance=15 .*fired=65 '
if printf '%s\n' "$route" | grep -q 'Player died'; then
  echo 'Unexpected death in cave round-trip combat route' >&2
  exit 1
fi
printf '%s\n' "$route" | grep -E '^(Trigger at frame|Simulation ended)'

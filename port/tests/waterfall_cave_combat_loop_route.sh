#!/bin/sh
# One uninterrupted fixed-60-Hz ordinary-control route. Functional CPU gate,
# not a combat-hit, physical-input, display, or GPU performance acceptance.
set -eu
export SDL_VIDEODRIVER=offscreen
export SDL_RENDER_DRIVER=opengl
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
route=$(port/build/deathleak-linux \
  --play Game/XmlFiles/levels/forest_exploration_1.xml \
  --script B:1100,Q:40,R:900,L:350,Q:400,L:1000,R:900 \
  --fixed-dt 0.0166667 --simulate-only)
for transition in \
  '398 .*forest_exploration_1.xml .*waterfall_top.xml' \
  '762 .*waterfall_top.xml .*waterfall_bottom.xml' \
  '1378 .*waterfall_bottom.xml .*underwater_cave.xml' \
  '1780 .*underwater_cave.xml .*underwater_cave_6.xml' \
  '2396 .*underwater_cave_6.xml .*underwater_cave.xml' \
  '2729 .*underwater_cave.xml .*waterfall_bottom.xml' \
  '2828 .*waterfall_bottom.xml .*waterfall_cave.xml' \
  '4373 .*waterfall_cave.xml .*waterfall_bottom.xml' \
  '4481 .*waterfall_bottom.xml .*underwater_cave.xml'; do
  printf '%s\n' "$route" | grep -q "Trigger at frame $transition" || {
    printf 'Missing authored transition: %s\n' "$transition" >&2
    exit 1
  }
done
printf '%s\n' "$route" | grep -q 'Simulation ended at Game/XmlFiles/levels/underwater_cave.xml .*orbs=33 keys=0 health=25 .*balance=33 .*hits=0 kills=0 fired=65 '
if printf '%s\n' "$route" | grep -q 'Player died'; then
  echo 'Unexpected death on combined combat/transition loop' >&2
  exit 1
fi
printf '%s\n' "$route" | grep -E '^(Trigger at frame|Simulation ended)'

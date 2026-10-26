#!/bin/sh
# Current-ref CPU-functional normal-control gates; not physical GPU/input acceptance.
set -eu
export SDL_VIDEODRIVER=offscreen
export SDL_RENDER_DRIVER=opengl
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1
binary=port/build/deathleak-linux
level=Game/XmlFiles/levels/forest_exploration_1.xml

forward=$($binary --play "$level" --script R:1100,Q:40,R:641,J:200,E:2200,L:48,I:10 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$forward" | grep -q 'Trigger at frame 398 from .*forest_exploration_1.xml .*waterfall_top.xml'
printf '%s\n' "$forward" | grep -q 'Trigger at frame 762 from .*waterfall_top.xml .*waterfall_bottom.xml'
printf '%s\n' "$forward" | grep -q 'Trigger at frame 1378 from .*waterfall_bottom.xml .*underwater_cave.xml'
printf '%s\n' "$forward" | grep -q 'Trigger at frame 1780 from .*underwater_cave.xml .*underwater_cave_6.xml'
printf '%s\n' "$forward" | grep -q 'Trigger at frame 3224 from .*underwater_cave_6.xml .*hot_springs.xml'
printf '%s\n' "$forward" | grep -q 'Trigger at frame 3377 from .*hot_springs.xml .*underwater_cave_4.xml'
printf '%s\n' "$forward" | grep -q 'Trigger at frame 3602 from .*underwater_cave_4.xml .*underwater_cave_3.xml'
printf '%s\n' "$forward" | grep -q 'Trigger at frame 3637 from .*underwater_cave_3.xml .*underwater_cave_4.xml'
printf '%s\n' "$forward" | grep -q 'Trigger at frame 3868 from .*underwater_cave_4.xml .*underwater_cave_5.xml'
printf '%s\n' "$forward" | grep -q 'Door at frame 4231 from .*underwater_cave_5.xml to .*forest_exploration_1.xml'
printf '%s\n' "$forward" | grep -q 'Simulation ended at .*forest_exploration_1.xml .*orbs=38 keys=0 health=25 .*balance=38 .*hits=0 kills=0 fired=0'

combat=$($binary --play "$level" --script L:1500,Q:40,L:100,F:300,L:700 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$combat" | grep -q 'Trigger at frame 300 from .*forest_exploration_1.xml .*grass_exploration_3.xml'
printf '%s\n' "$combat" | grep -q 'Trigger at frame 1093 from .*grass_exploration_3.xml .*first_ninja_arena.xml'
printf '%s\n' "$combat" | grep -q 'Trigger at frame 2292 from .*first_ninja_arena.xml .*grass_exploration_1.xml'
printf '%s\n' "$combat" | grep -q 'Simulation ended at .*grass_exploration_1.xml .*orbs=43 keys=0 health=22 .*balance=43 .*hits=1 kills=0 fired=18'

for result in "$forward" "$combat"; do
  if printf '%s\n' "$result" | grep -q 'Player died'; then
    echo 'Unexpected death on uninterrupted route' >&2
    exit 1
  fi
  printf '%s\n' "$result" | grep -E '^(Trigger at frame|Door at frame|Simulation ended)'
done

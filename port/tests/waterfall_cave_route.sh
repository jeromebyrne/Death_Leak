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

# Switch to ordinary right+jump/swim on the exact cave-6 entry tick. Waiting
# until the right-hand solid pins the grounded player does not reach this exit.
# No warp, save load, injected state, XML edit, or collision bypass is used.
hot_springs_route=$(port/build/deathleak-linux \
  --play Game/XmlFiles/levels/forest_exploration_1.xml \
  --script R:1100,Q:40,R:640,E:1500,R:1300 \
  --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$hot_springs_route" | grep -q 'Trigger at frame 1780 from .*underwater_cave.xml .*underwater_cave_6.xml'
printf '%s\n' "$hot_springs_route" | grep -q 'Trigger at frame 3037 from .*underwater_cave_6.xml .*hot_springs.xml'
printf '%s\n' "$hot_springs_route" | grep -q 'Trigger at frame 3190 from .*hot_springs.xml .*underwater_cave_4.xml'
printf '%s\n' "$hot_springs_route" | grep -q 'Trigger at frame 3415 from .*underwater_cave_4.xml .*underwater_cave_3.xml'
printf '%s\n' "$hot_springs_route" | grep -q 'Simulation ended at Game/XmlFiles/levels/underwater_cave_3.xml .*orbs=25 keys=0 health=25 .*balance=25'
if printf '%s\n' "$hot_springs_route" | grep -q 'Player died'; then
  echo 'Unexpected death on cave-6 to Hot Springs route' >&2
  exit 1
fi
printf '%s\n' "$hot_springs_route" | grep -E '^(Trigger at frame|Simulation ended)'

# Continue from cave 3 with ordinary held right+jump/swim. The authored
# cave-3 return to cave 4 then cave-4 exit to cave 5 must both occur.
cave5_route=$(port/build/deathleak-linux \
  --play Game/XmlFiles/levels/forest_exploration_1.xml \
  --script R:1100,Q:40,R:640,E:1500,R:1300,E:2000 \
  --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$cave5_route" | grep -q 'Trigger at frame 3037 from .*underwater_cave_6.xml .*hot_springs.xml'
printf '%s\n' "$cave5_route" | grep -q 'Trigger at frame 3190 from .*hot_springs.xml .*underwater_cave_4.xml'
printf '%s\n' "$cave5_route" | grep -q 'Trigger at frame 3415 from .*underwater_cave_4.xml .*underwater_cave_3.xml'
printf '%s\n' "$cave5_route" | grep -q 'Trigger at frame 4770 from .*underwater_cave_3.xml .*underwater_cave_4.xml'
printf '%s\n' "$cave5_route" | grep -q 'Trigger at frame 5001 from .*underwater_cave_4.xml .*underwater_cave_5.xml'
printf '%s\n' "$cave5_route" | grep -q 'Simulation ended at Game/XmlFiles/levels/underwater_cave_5.xml .*orbs=38 keys=0 health=25 .*balance=38'
if printf '%s\n' "$cave5_route" | grep -q 'Player died'; then
  echo 'Unexpected death on cave-3 to cave-5 route' >&2
  exit 1
fi
printf '%s\n' "$cave5_route" | grep -E '^(Trigger at frame|Simulation ended)'

# Cave 5 has an authored, key-free Door id 16 back to Forest-1. Approach it
# with ordinary right movement after the cave-4 entry, then hold interact.
# This is a real door transition, not a warp or injected progression state.
cave5_door_route=$(port/build/deathleak-linux \
  --play Game/XmlFiles/levels/forest_exploration_1.xml \
  --script R:1100,Q:40,R:640,E:1500,R:1300,E:421,R:200,I:30 \
  --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$cave5_door_route" | grep -q 'Trigger at frame 5001 from .*underwater_cave_4.xml .*underwater_cave_5.xml'
printf '%s\n' "$cave5_door_route" | grep -q 'Door at frame 5203 from .*underwater_cave_5.xml to .*forest_exploration_1.xml'
printf '%s\n' "$cave5_door_route" | grep -q 'Simulation ended at Game/XmlFiles/levels/forest_exploration_1.xml .*orbs=37 keys=0 health=25 .*balance=37'
if printf '%s\n' "$cave5_door_route" | grep -q 'Player died'; then
  echo 'Unexpected death on cave-5 authored Forest door route' >&2
  exit 1
fi
printf '%s\n' "$cave5_door_route" | grep -E '^(Trigger at frame|Door at frame|Simulation ended)'

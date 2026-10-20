#!/bin/sh
# Run from repository root after building port/build/deathleak-linux.
set -eu
export SDL_VIDEODRIVER=dummy
bin=port/build/deathleak-linux
transition=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script L:340 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$transition" | grep -q 'Entered Game/XmlFiles/levels/grass_exploration_3.xml'
route=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script L:1300 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$route" | grep -q 'Entered Game/XmlFiles/levels/grass_exploration_3.xml'
printf '%s\n' "$route" | grep -q 'Entered Game/XmlFiles/levels/first_ninja_arena.xml'
printf '%s\n' "$route" | grep -q 'orbs=26 health=25.*spawned=1.*grounded=1'
burst=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script F:60 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$burst" | grep -q 'fired=5'
water_fire=$($bin --play Game/XmlFiles/levels/test.xml --start-hazard 1 --script F:30 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$water_fire" | grep -q 'fired=0'
focus_lock=$($bin --play Game/XmlFiles/levels/test.xml --start-hazard 1 --exhaust-focus --script R:120,.:240 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$focus_lock" | grep -q 'focus=0'
focus_recover=$($bin --play Game/XmlFiles/levels/test.xml --start-hazard 1 --exhaust-focus --script R:120,.:360 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$focus_recover" | grep -q 'focus=17.3334'
combat=$($bin --play Game/XmlFiles/levels/first_ninja_arena.xml --start-encounter 0 --script F:300 --fixed-dt 0.0166667 --simulate-only --save-file port/build/integration-save.txt)
printf '%s\n' "$combat" | grep -q 'spawned=2 enemies=1 hits=17 kills=1'
resume=$($bin --load-save port/build/integration-save.txt --frames 1 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$resume" | grep -q 'health=19 focus=100'
water=$($bin --play Game/XmlFiles/levels/test.xml --start-hazard 1 --exhaust-focus --script .:1 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$water" | grep -q 'health=24.95 focus=0'
landing=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script .:120 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$landing" | grep -q 'player=(-43.0146,3206.9).*grounded=1'
jump=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script .:120,J:1 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$jump" | grep -q 'player=(-43.0146,3237.22).*grounded=0'
printf 'DL2\nGame/XmlFiles/levels/forest_exploration_1.xml\n-43.014648 3385 0 100\n' > port/build/integration-dead-save.txt
restart=$($bin --load-save port/build/integration-dead-save.txt --script .:1,T:1,.:1 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$restart" | grep -q 'Player died at frame 0'
printf '%s\n' "$restart" | grep -q 'Restarted Game/XmlFiles/levels/grass_exploration_3.xml at frame 1'
printf '%s\n' "$restart" | grep -q 'health=25 focus=100'
slope=$($bin --play port/tests/fixtures/slope_wall.xml --script R:90 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$slope" | grep -q 'player=(235.52,154.553).*grounded=1'
printf '%s\n' 'PASS: three-level route, burst/water fire, focus cooldown, encounter, save/resume, drowning, landing/jump, slope/wall, death/restart'

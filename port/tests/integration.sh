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
printf '%s\n' "$route" | grep -q 'orbs=26 keys=0 health=25.*spawned=1.*grounded=1'
arena_climb=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script L:1500,Q:40,L:100 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$arena_climb" | grep -q 'Simulation ended at Game/XmlFiles/levels/first_ninja_arena.xml player=(3756.65,-1271.93).*grounded=1'
arena_exit=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script L:1500,Q:40,L:700 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$arena_exit" | grep -q 'Trigger at frame 2008 from Game/XmlFiles/levels/first_ninja_arena.xml'
printf '%s\n' "$arena_exit" | grep -q 'Entered Game/XmlFiles/levels/grass_exploration_1.xml'
burst=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script F:60 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$burst" | grep -q 'fired=5'
water_fire=$($bin --play Game/XmlFiles/levels/test.xml --start-hazard 1 --script F:30 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$water_fire" | grep -q 'fired=0'
swim=$($bin --play Game/XmlFiles/levels/test.xml --start-hazard 1 --script J:180 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$swim" | grep -q 'player=(3147.85,1304.1).*health=25'
focus_lock=$($bin --play Game/XmlFiles/levels/test.xml --start-hazard 1 --exhaust-focus --script L:240,.:240 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$focus_lock" | grep -q 'focus=0'
focus_recover=$($bin --play Game/XmlFiles/levels/test.xml --start-hazard 1 --exhaust-focus --script L:240,.:360 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$focus_recover" | grep -q 'focus=19.3334'
combat=$($bin --play Game/XmlFiles/levels/first_ninja_arena.xml --start-encounter 0 --script F:300 --fixed-dt 0.0166667 --simulate-only --save-file port/build/integration-save.txt)
printf '%s\n' "$combat" | grep -q 'spawned=2 enemies=1 hits=17 kills=1'
resume=$($bin --load-save port/build/integration-save.txt --frames 1 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$resume" | grep -q 'health=16 focus=100'
# NPCTrigger removes a recently-fired trigger on level load (120000 ms).
# Force overlap on resume so a missing cooldown would visibly respawn it.
cooldown=$($bin --load-save port/build/integration-save.txt --start-encounter 0 --frames 1 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$cooldown" | grep -q 'spawned=0 enemies=0'
grep -q '^N:Game/XmlFiles/levels/first_ninja_arena.xml#' port/build/integration-save.txt
# A timestamp outside the original two-minute wall-clock window permits respawn.
python3 - <<'PYTEST'
from pathlib import Path
source = Path('port/build/integration-save.txt').read_text().splitlines()
Path('port/build/expired-encounter-save.txt').write_text('\n'.join(
    line.rsplit('\t', 1)[0] + '\t1' if line.startswith('N:') else line for line in source) + '\n')
PYTEST
expired=$($bin --load-save port/build/expired-encounter-save.txt --start-encounter 0 --frames 1 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$expired" | grep -q 'spawned=2 enemies=2'
ghost=$($bin --play Game/XmlFiles/levels/forest_exploration_2.xml --start-encounter 0 --script .:120 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$ghost" | grep -q 'spawned=1 enemies=1'


water=$($bin --play Game/XmlFiles/levels/test.xml --start-hazard 1 --exhaust-focus --script .:1 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$water" | grep -q 'health=24.95 focus=0'
landing=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script .:120 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$landing" | grep -q 'player=(-43.0146,3206.9).*grounded=1'
jump=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script .:120,J:1 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$jump" | grep -q 'player=(-43.0146,3224.62).*grounded=0'
printf 'DL2\nGame/XmlFiles/levels/forest_exploration_1.xml\n-43.014648 3385 0 100\n' > port/build/integration-dead-save.txt
restart=$($bin --load-save port/build/integration-dead-save.txt --script .:1,T:1,.:1 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$restart" | grep -q 'Player died at frame 0'
printf '%s\n' "$restart" | grep -q 'Restarted Game/XmlFiles/levels/grass_exploration_3.xml at frame 1'
printf '%s\n' "$restart" | grep -q 'health=25 focus=100'
slope=$($bin --play port/tests/fixtures/slope_wall.xml --script R:90 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$slope" | grep -q 'player=(235.52,154.553).*grounded=1'
combined_jump=$($bin --play port/tests/fixtures/slope_wall.xml --script .:60,Q:12 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$combined_jump" | grep -q 'player=(-68.0002,295.329).*grounded=0'
tap_jump=$($bin --play port/tests/fixtures/slope_wall.xml --script .:60,J:1,.:39 --fixed-dt 0.0166667 --simulate-only)
held_jump=$($bin --play port/tests/fixtures/slope_wall.xml --script .:60,J:40 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$tap_jump" | grep -q 'player=(0,107.449).*grounded=1'
printf '%s\n' "$held_jump" | grep -q 'player=(0,141.509).*grounded=0'
solid_side=$($bin --play port/tests/fixtures/solid_box.xml --script R:30 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$solid_side" | grep -q 'player=(95.5201,107.449).*grounded=1'
solid_top=$($bin --play port/tests/fixtures/solid_box.xml --script R:30,E:40 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$solid_top" | grep -q 'player=(235.52,407.449).*grounded=1'
wall_jump=$($bin --play port/tests/fixtures/solid_box.xml --script R:30,J:1,R:19,E:1 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$wall_jump" | grep -q 'player=(80.5201,158.244).*grounded=0'
locked=$($bin --play port/tests/fixtures/key_door.xml --script R:43,I:90 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$locked" | grep -q 'Door locked: requires granary'
printf '%s\n' "$locked" | grep -q 'Simulation ended at port/tests/fixtures/key_door.xml.*keys=0'
key_door=$($bin --play port/tests/fixtures/key_door.xml --script I:4,R:43,I:110 --fixed-dt 0.0166667 --simulate-only --save-file port/build/key-door-save.txt)
printf '%s\n' "$key_door" | grep -q 'Collected key granary at frame 2'
printf '%s\n' "$key_door" | grep -q 'Unlocked door test_gate at frame 49'
printf '%s\n' "$key_door" | grep -q 'Door at frame 132 from port/tests/fixtures/key_door.xml to port/tests/fixtures/door_destination.xml'
printf '%s\n' "$key_door" | grep -q 'player=(123,107.449).*keys=1'
key_resume=$($bin --load-save port/build/key-door-save.txt --frames 1 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$key_resume" | grep -q 'keys=1'
key_only=$($bin --play port/tests/fixtures/key_door.xml --script I:4 --fixed-dt 0.0166667 --simulate-only --save-file port/build/key-only-save.txt)
key_only_resume=$($bin --load-save port/build/key-only-save.txt --frames 1 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$key_only_resume" | grep -q 'keys=1'
# Verify the predecessor Linux save version remains readable.
python3 - <<'PYTEST'
from pathlib import Path
source = Path('port/build/key-only-save.txt').read_text().splitlines()
source[0] = 'DL3'
Path('port/build/legacy-dl3-save.txt').write_text('\n'.join(line for line in source if not line.startswith('N:')) + '\n')
PYTEST
legacy=$($bin --load-save port/build/legacy-dl3-save.txt --frames 1 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$legacy" | grep -q 'keys=1'

if printf '%s\n' "$key_only_resume" | grep -q 'Collected key'; then exit 1; fi
printf '%s\n' 'PASS: source-held-jump arena climb and exit, three-level route, burst/water fire, swim, focus cooldown, encounter, save/resume, drowning, landing/jump, combined jump, slope/wall, solid box, wall jump, death/restart, key/door gate, ghost encounter, DL3 compatibility and DL4 encounter persistence'

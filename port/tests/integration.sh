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
printf '%s\n' "$combat" | grep -q 'spawned=2 enemies=2 hits=6 kills=0'
resume=$($bin --load-save port/build/integration-save.txt --frames 1 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$resume" | grep -q 'health=25 focus=100'
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


# Original CollisionManager calls both character handlers on strict AABB
# overlap, but Character::OnCollision excludes other characters from body
# separation. Contact must not create touch damage or an NPC landing surface.
contact_side=$($bin --play port/tests/fixtures/encounter_npc_wall.xml --script R:90 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$contact_side" | grep -q 'player=(160.52,107.449).*health=25 .*grounded=1'
printf '%s\n' "$contact_side" | grep -q 'Player/NPC contacts=1 contact_frames=79'
contact_descend=$($bin --play port/tests/fixtures/encounter_npc_wall.xml --script R:30,J:15,.:23 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$contact_descend" | grep -q 'player=(160.52,200.419).*health=25 .*grounded=0'
printf '%s\n' "$contact_descend" | grep -q 'Player/NPC contacts=1 contact_frames=36'
# Player::DoMeleeAttack consumes 4 focus only from solid ground. The authored
# player animation reaches its damaging third phase after 7+15 frames;
# Character::OnCollision then hits only a front-side overlapping NPC.
melee_start=$($bin --play port/tests/fixtures/encounter_npc_wall.xml --script R:90,M:1 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$melee_start" | grep -q 'focus=96.1667 .*fired=0'
printf '%s\n' "$melee_start" | grep -q 'Melee active=1 strikes=0'
melee_early=$($bin --play port/tests/fixtures/encounter_npc_wall.xml --script R:90,M:22 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$melee_early" | grep -q 'enemies=1 .*kills=0 fired=0'
printf '%s\n' "$melee_early" | grep -q 'Melee active=1 strikes=0'
melee_hit=$($bin --play port/tests/fixtures/encounter_npc_wall.xml --script R:90,M:23 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$melee_hit" | grep -q 'enemies=0 .*kills=1 fired=0'
printf '%s\n' "$melee_hit" | grep -q 'Melee active=1 strikes=1'
melee_behind=$($bin --play port/tests/fixtures/encounter_npc_wall.xml --script R:15,L:1,M:24 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$melee_behind" | grep -q 'enemies=1 .*kills=0 fired=0'
printf '%s\n' "$melee_behind" | grep -q 'Player/NPC contacts=1'
printf '%s\n' "$melee_behind" | grep -q 'Melee active=1 strikes=0'
melee_air=$($bin --play port/tests/fixtures/encounter_npc_wall.xml --script R:30,J:15,M:25 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$melee_air" | grep -q 'enemies=1 .*kills=0 .*grounded=0'
printf '%s\n' "$melee_air" | grep -q 'Melee active=0 strikes=0'
melee_empty=$($bin --play port/tests/fixtures/encounter_npc_wall.xml --exhaust-focus --script R:1,M:1 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$melee_empty" | grep -q 'Melee active=0 strikes=0'
melee_water=$($bin --play Game/XmlFiles/levels/test.xml --start-hazard 1 --script M:1 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$melee_water" | grep -q 'Melee active=0 strikes=0'
# AIStateRangeAttack: hold a 200-unit standoff and wind up for 0.25 s.
range_warmup=$($bin --play port/tests/fixtures/encounter_open.xml --frames 90 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$range_warmup" | grep -q 'Enemy kNinja x=200 .*windup=0.049'
printf '%s\n' "$range_warmup" | grep -q 'Enemy kNinja x=200 y=100 .*vy=0 grounded=1'
falling=$($bin --play port/tests/fixtures/encounter_open.xml --frames 1 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$falling" | grep -q 'Enemy kNinja x=158 y=407.092 .*vy=-42.8572 grounded=0'
wall_enemy=$($bin --play port/tests/fixtures/encounter_npc_wall.xml --frames 90 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$wall_enemy" | grep -q 'Enemy kNinja x=175 y=100 .*grounded=1'
box_enemy=$($bin --play port/tests/fixtures/encounter_npc_box.xml --frames 90 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$box_enemy" | grep -q 'Enemy kNinja x=200 y=300 .*grounded=1'
open_shot=$($bin --play port/tests/fixtures/encounter_open.xml --frames 105 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$open_shot" | grep -q 'Shot enemy x=83.0809 y=103.445 .*vy=12.2994'
# Original projectile terrain handling: the same shot is stopped by an authored wall.
walled_shot=$($bin --play port/tests/fixtures/encounter_wall.xml --frames 105 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$walled_shot" | grep -q 'spawned=1 enemies=1 .*shots=0'
# Original Character::OnDamage kicks a non-player target 30 X units on
# projectile impact. A direct injected hit is not a projectile and stays put.
kickback=$($bin --play port/tests/fixtures/encounter_open.xml --script F:30 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$kickback" | grep -q 'Enemy kNinja x=230 y=100 .*combo=1'
direct_hit=$($bin --play port/tests/fixtures/encounter_open.xml --script .:90,H:1 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$direct_hit" | grep -q 'Enemy kNinja x=200 y=100 .*combo=1'
# Chasing while firing can still defeat the kicked-back source-health ninja.
range_kill=$($bin --play port/tests/fixtures/encounter_roll.xml --script B:330 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$range_kill" | grep -q 'spawned=1 enemies=0 .*kills=1'
# AIStateRangeAttack chooses the upward branch only above +50 Y; it chooses
# the roll branch on the same positive-acceleration gate while grounded.
jump_before=$($bin --play port/tests/fixtures/encounter_jump.xml --frames 119 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$jump_before" | grep -q 'Enemy kNinja x=175 y=100 .*jumps=0 rolls=0'
jump_after=$($bin --play port/tests/fixtures/encounter_jump.xml --frames 125 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$jump_after" | grep -q 'Enemy kNinja x=175 y=195.143 .*grounded=0 .*jumps=1 rolls=0'
roll_active=$($bin --play port/tests/fixtures/encounter_roll.xml --script R:125 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$roll_active" | grep -q 'Enemy kNinja .*jumps=0 rolls=1 roll=0.4166'
# Simulation-only H injects a direct hit through Combat::DamageEnemy, the same
# path used by knives. The skeleton's large source health keeps it alive long
# enough to inspect the NPC::OnDamage combo/stun rule and AI recovery.
five_hits=$($bin --play port/tests/fixtures/encounter_stun.xml --script .:90,H:1,.:4,H:1,.:4,H:1,.:4,H:1,.:4,H:1 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$five_hits" | grep -q 'grounded=1 combo=5 stun=0'
six_hits=$($bin --play port/tests/fixtures/encounter_stun.xml --script .:90,H:1,.:4,H:1,.:4,H:1,.:4,H:1,.:4,H:1,.:4,H:1 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$six_hits" | grep -q 'grounded=1 combo=6 stun=2'
mid_stun=$($bin --play port/tests/fixtures/encounter_stun.xml --script .:90,H:1,.:4,H:1,.:4,H:1,.:4,H:1,.:4,H:1,.:4,H:1,.:60 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$mid_stun" | grep -q 'windup=0 .*grounded=1 combo=0 stun=1'
recovered=$($bin --play port/tests/fixtures/encounter_stun.xml --script .:90,H:1,.:4,H:1,.:4,H:1,.:4,H:1,.:4,H:1,.:4,H:1,.:120 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$recovered" | grep -q 'windup=0.25 .*grounded=1 combo=0 stun=0'
combo_expired=$($bin --play port/tests/fixtures/encounter_stun.xml --script .:90,H:1,.:35,H:1 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$combo_expired" | grep -q 'grounded=1 combo=1 stun=0'
airborne=$($bin --play port/tests/fixtures/encounter_stun.xml --script H:6 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$airborne" | grep -q 'grounded=0 combo=6 stun=0'
# AIStateRangeAttack/Character: a distant ninja waits eight seconds, becomes
# unhittable for the 0.8 s teleport, then reappears at the source-range
# 500-599 X / 200-299 Y offset. SkeletonEnemy disables teleport entirely.
teleport_early=$($bin --play port/tests/fixtures/encounter_teleport.xml --script L:140,.:330 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$teleport_early" | grep -q 'Enemy kNinja x=150 y=100 .*teleport=0 teleports=0'
teleport_active=$($bin --play port/tests/fixtures/encounter_teleport.xml --script L:140,.:380 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$teleport_active" | grep -q 'Enemy kNinja x=150 y=100 .*teleport=0.1333.*teleports=0'
teleport_hit=$($bin --play port/tests/fixtures/encounter_teleport.xml --script L:140,.:380,H:1 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$teleport_hit" | grep -q 'enemies=1 hits=0'
teleport_done=$($bin --play port/tests/fixtures/encounter_teleport.xml --script L:140,.:440 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$teleport_done" | grep -q 'Enemy kNinja x=-623.54 y=100 .*teleport=0 teleports=1'
no_skeleton_teleport=$($bin --play port/tests/fixtures/encounter_teleport_skeleton.xml --script L:140,.:440 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$no_skeleton_teleport" | grep -q 'Enemy kSkeleton x=150 y=100 .*teleport=0 teleports=0'
no_near_teleport=$($bin --play port/tests/fixtures/encounter_open.xml --frames 580 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$no_near_teleport" | grep -q 'Enemy kNinja .*teleport=0 teleports=0'
# AIStateFlying leaves an authored ghost elevated and outside its close fire range.
ghost_state=$($bin --play Game/XmlFiles/levels/forest_exploration_2.xml --start-encounter 0 --frames 120 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$ghost_state" | grep -q 'Enemy kGhost x=1002.99 y=2977.91'
printf '%s\n' "$ghost_state" | grep -q 'health=25 .*shots=0'

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
printf '%s\n' 'PASS: source-held-jump arena climb and exit, three-level route, burst/water fire, swim, focus cooldown, encounter, save/resume, drowning, landing/jump, combined jump, slope/wall, solid box, wall jump, death/restart, key/door gate, ghost flight, ranged warmup/terrain shots and kill, grounded NPC fall/wall/box, NPC combo/stun/recovery, NPC teleport timing/exclusion, ranged NPC jump/roll and projectile kickback, source-faithful player/NPC contact pass-through, grounded phase-three melee and its negative gates, DL3 compatibility and DL4 encounter persistence'

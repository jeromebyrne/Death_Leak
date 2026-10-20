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
printf '%s\n' "$route" | grep -q 'orbs=27 keys=0 health=25.*spawned=1.*grounded=0'
arena_climb=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script L:1500,Q:40,L:100 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$arena_climb" | grep -q 'Simulation ended at Game/XmlFiles/levels/first_ninja_arena.xml player=(3724.65,-1271.76).*grounded=1'
arena_exit=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script L:1500,Q:40,L:700 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$arena_exit" | grep -q 'Trigger at frame 1992 from Game/XmlFiles/levels/first_ninja_arena.xml'
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
# The authored forest deflection scroll is reached from the real spawn by
# ordinary rightward movement, a held right-jump, and the 0.5-second E hold.
# These four orbs are actually collected, not supplied by a synthetic save.
authored_scroll_save=port/build/authored-scroll-route-save.txt
authored_scroll=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script R:130,E:25,I:31,P:1 --fixed-dt 0.0166667 --simulate-only --combat-state --save-file "$authored_scroll_save")
printf '%s\n' "$authored_scroll" | grep -q 'Opened kDeflection upgrade modal at frame 184 cost=375 balance=4'
printf '%s\n' "$authored_scroll" | grep -q 'Not enough orbs for kDeflection at frame 186 balance=4'
printf '%s\n' "$authored_scroll" | grep -q 'forest_exploration_1.xml player=(1200.52,3387.19) orbs=4 .*balance=4'
printf '%s\n' "$authored_scroll" | grep -q 'Deflection unlocked=0 count=0'
grep -q '^B:4$' "$authored_scroll_save"
if grep -q '^U:kDeflection$' "$authored_scroll_save"; then exit 1; fi
authored_scroll_resume=$($bin --load-save "$authored_scroll_save" --frames 1 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$authored_scroll_resume" | grep -q 'orbs=4 .*balance=4'
printf '%s\n' "$authored_scroll_resume" | grep -q 'Deflection unlocked=0 count=0'
# The same source spawn also reaches two later authored transitions with normal
# movement. This is progression evidence, not a 375-orb purchase route.
east_route=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script R:1100,Q:40,R:300 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$east_route" | grep -q 'Entered Game/XmlFiles/levels/waterfall_top.xml'
printf '%s\n' "$east_route" | grep -q 'Entered Game/XmlFiles/levels/waterfall_bottom.xml'
printf '%s\n' "$east_route" | grep -q 'Entered Game/XmlFiles/levels/underwater_cave.xml'
printf '%s\n' "$east_route" | grep -q 'underwater_cave.xml player=.*orbs=16 .*balance=16'
# A rightward held jump at the authored waterfall-top gap (frame 710)
# crosses its fall trigger and reaches forest 2 from the original forest-1
# spawn. Normal knife fire then carries the same, unmodified run into village
# central; no loaded balance, warp, or test-hit injection is used.
continuous_save=port/build/continuous-village-save.txt
continuous_route=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script R:710,E:40,B:4500 --fixed-dt 0.0166667 --simulate-only --save-file "$continuous_save")
printf '%s\n' "$continuous_route" | grep -q 'Trigger at frame 398 .*waterfall_top.xml'
printf '%s\n' "$continuous_route" | grep -q 'Trigger at frame 1058 .*forest_exploration_2.xml'
printf '%s\n' "$continuous_route" | grep -q 'Trigger at frame 1714 .*forest_exploration_barn.xml'
printf '%s\n' "$continuous_route" | grep -q 'Trigger at frame 2714 .*village_forts.xml'
printf '%s\n' "$continuous_route" | grep -q 'Trigger at frame 4037 .*village_central.xml'
printf '%s\n' "$continuous_route" | grep -q 'village_central.xml player=(7287.18,-1525.13) orbs=99 .*health=1 .*balance=99'
python3 - <<'PYTEST'
from pathlib import Path
lines = Path('port/build/continuous-village-save.txt').read_text().splitlines()
orb_ids = [line for line in lines if line.startswith('Game/XmlFiles/levels/') and '#' in line]
assert len(orb_ids) == len(set(orb_ids)) == 99, (len(orb_ids), len(set(orb_ids)))
assert 'B:99' in lines
PYTEST
# A single uninterrupted forest-spawn run combines the earned east itinerary
# with a westward return through grass 3 into the arena. The east-side 190
# includes its forest pickups; the 227 endpoint is 227 unique IDs, not 190+227.
# This 227-orb intermediate stop dies under ordinary continued left movement;
# the timed jump/fire itinerary below survives the arena and reaches sea 1.
combined_save=port/build/combined-east-west-save.txt
combined_prefix=R:710,E:40,B:4480,E:40,B:50,E:40,B:50,E:40,B:3400,L:3410,Q:40,L:4640,Q:40
combined_route=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script "$combined_prefix,L:1826,Q:40,L:260" --fixed-dt 0.0166667 --simulate-only --save-file "$combined_save")
printf '%s\n' "$combined_route" | grep -q 'Trigger at frame 17306 from .*waterfall_top.xml .*forest_exploration_1.xml'
printf '%s\n' "$combined_route" | grep -q 'Trigger at frame 17947 from .*forest_exploration_1.xml .*grass_exploration_3.xml'
printf '%s\n' "$combined_route" | grep -q 'Trigger at frame 18740 from .*grass_exploration_3.xml .*first_ninja_arena.xml'
printf '%s\n' "$combined_route" | grep -q 'first_ninja_arena.xml player=(4794.17,-1780.42) orbs=227 .*health=1 .*balance=227'
if printf '%s\n' "$combined_route" | grep -q 'Player died'; then exit 1; fi
python3 - <<'PYCOMBINED'
from pathlib import Path
lines = Path('port/build/combined-east-west-save.txt').read_text().splitlines()
orb_ids = [line for line in lines if line.startswith('Game/XmlFiles/levels/') and '#' in line]
assert len(orb_ids) == len(set(orb_ids)) == 227, (len(orb_ids), len(set(orb_ids)))
assert 'B:227' in lines
assert any('grass_exploration_3.xml#' in orb for orb in orb_ids)
assert any('first_ninja_arena.xml#' in orb for orb in orb_ids)
PYCOMBINED
combined_overrun=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script "$combined_prefix,L:1826,Q:40,L:300" --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$combined_overrun" | grep -q 'Player died at frame 19109 in Game/XmlFiles/levels/first_ninja_arena.xml'
# Normal left-fire intercepts the close arena shot; a held left+jump clears
# its wall. Timed jumps in grass 1/2 then reach cliff and sea 1. This remains
# one uninterrupted forest-spawn simulation with earned IDs only.
combined_sea_save=port/build/combined-sea1-save.txt
combined_sea_script="$combined_prefix,L:1826,Q:40,L:245,Q:80,N:100,Q:80,N:960,Q:40,N:710,Q:40,N:50,Q:40,N:160,Q:40,N:2200"
combined_sea=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script "$combined_sea_script" --fixed-dt 0.0166667 --simulate-only --save-file "$combined_sea_save")
printf '%s\n' "$combined_sea" | grep -q 'Trigger at frame 19763 from .*first_ninja_arena.xml .*grass_exploration_1.xml'
printf '%s\n' "$combined_sea" | grep -q 'Trigger at frame 20554 from .*grass_exploration_1.xml .*grass_exploration_2.xml'
printf '%s\n' "$combined_sea" | grep -q 'Trigger at frame 21713 from .*grass_exploration_2.xml .*cliff.xml'
printf '%s\n' "$combined_sea" | grep -q 'Trigger at frame 23326 from .*cliff.xml .*sea_1.xml'
printf '%s\n' "$combined_sea" | grep -q 'sea_1.xml player=(-5761.85,-1266.6) orbs=290 .*health=1 .*balance=290'
if printf '%s\n' "$combined_sea" | grep -q 'Player died'; then exit 1; fi
python3 - <<'PYSEA'
from pathlib import Path
lines = Path('port/build/combined-sea1-save.txt').read_text().splitlines()
orb_ids = [line for line in lines if line.startswith('Game/XmlFiles/levels/') and '#' in line]
assert len(orb_ids) == len(set(orb_ids)) == 290, (len(orb_ids), len(set(orb_ids)))
assert 'B:290' in lines
for level in ('grass_exploration_1.xml', 'grass_exploration_2.xml', 'cliff.xml'):
    assert any(level + '#' in orb for orb in orb_ids), level
PYSEA
# Continuing the same uninterrupted route rightward from sea 1 re-enters
# cliff and picks up six previously missed authored orbs. The lower return
# corridor stops at the authored id-30 solid wall; this is not forest return.
combined_sea_return_save=port/build/combined-sea-return-save.txt
combined_sea_return=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script "$combined_sea_script,R:1300" --fixed-dt 0.0166667 --simulate-only --save-file "$combined_sea_return_save")
printf '%s\n' "$combined_sea_return" | grep -q 'Trigger at frame 23880 from .*sea_1.xml .*cliff.xml'
printf '%s\n' "$combined_sea_return" | grep -q 'cliff.xml player=(-1500.15,-944.093) orbs=296 .*health=1 .*balance=296'
if printf '%s\n' "$combined_sea_return" | grep -q 'Player died'; then exit 1; fi
python3 - <<'PYSEARETURN'
from pathlib import Path
prior = {line for line in Path('port/build/combined-sea1-save.txt').read_text().splitlines() if line.startswith('Game/XmlFiles/levels/') and '#' in line}
lines = Path('port/build/combined-sea-return-save.txt').read_text().splitlines()
earned = [line for line in lines if line.startswith('Game/XmlFiles/levels/') and '#' in line]
assert len(earned) == len(set(earned)) == 296
assert len(set(earned) - prior) == 6
assert all('cliff.xml#' in orb for orb in set(earned) - prior)
assert 'B:296' in lines
PYSEARETURN
# The same forest-start run reverses on the upper cliff route, traverses
# grass 2/1, wall-jumps over both authored arena solids, and returns to the
# real forest-1 scroll. Its 306 unique earned orbs are insufficient for 375.
upper_return_save=port/build/upper-return-save.txt
upper_return_script="$combined_prefix,L:1826,Q:40,L:245,Q:80,N:100,Q:80,N:960,Q:40,N:710,Q:40,N:50,Q:40,N:160,Q:40,N:1550,B:2560,E:80,B:2500,E:20,B:1,E:80,B:2000,E:20,B:1,E:80,B:1250,J:25,I:31,P:1"
upper_return=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script "$upper_return_script" --fixed-dt 0.0166667 --simulate-only --combat-state --save-file "$upper_return_save")
printf '%s\n' "$upper_return" | grep -q 'Trigger at frame 24213 from .*cliff.xml .*grass_exploration_2.xml'
printf '%s\n' "$upper_return" | grep -q 'Trigger at frame 25369 from .*grass_exploration_2.xml .*grass_exploration_1.xml'
printf '%s\n' "$upper_return" | grep -q 'Trigger at frame 30321 from .*first_ninja_arena.xml .*grass_exploration_3.xml'
printf '%s\n' "$upper_return" | grep -q 'Trigger at frame 31103 from .*grass_exploration_3.xml .*forest_exploration_1.xml'
printf '%s\n' "$upper_return" | grep -q 'Opened kDeflection upgrade modal at frame 31587 cost=375 balance=306'
printf '%s\n' "$upper_return" | grep -q 'Not enough orbs for kDeflection at frame 31589 balance=306'
printf '%s\n' "$upper_return" | grep -q 'forest_exploration_1.xml player=(1176.53,3381.52) orbs=306 .*health=1 .*balance=306'
printf '%s\n' "$upper_return" | grep -q 'Deflection unlocked=0 count=0'
if printf '%s\n' "$upper_return" | grep -q 'Player died'; then exit 1; fi
python3 - <<'PYTEST'
from pathlib import Path
lines = Path('port/build/upper-return-save.txt').read_text().splitlines()
earned = [line for line in lines if line.startswith('Game/XmlFiles/levels/') and '#' in line]
assert len(earned) == len(set(earned)) == 306
assert 'B:306' in lines
assert 'U:kDeflection' not in lines
for level in ('cliff.xml', 'grass_exploration_2.xml', 'grass_exploration_1.xml', 'first_ninja_arena.xml'):
    assert any(level + '#' in orb for orb in earned), level
PYTEST

# Height-changing normal-control detours in barn, forts, shrine and
# agriculture earn 21 additional authored IDs while preserving the same
# uninterrupted forest-start scroll return. They do not reach the price.
optimized_upper_save=port/build/optimized-upper-save.txt
optimized_upper_script="R:710,E:40,B:1200,E:40,B:1060,E:40,B:2140,E:40,B:50,E:40,B:50,E:40,B:800,E:40,B:2160,E:40,B:360,Q:40,L:20,Q:40,L:250,Q:40,L:2000,Q:40,L:980,Q:40,L:4640,Q:40,L:1826,Q:40,L:245,Q:80,N:100,Q:80,N:960,Q:40,N:710,Q:40,N:50,Q:40,N:160,Q:40,N:1550,B:2560,E:80,B:2500,E:20,B:1,E:80,B:2000,E:20,B:1,E:80,B:1250,J:25,I:31,P:1"
optimized_upper=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script "$optimized_upper_script" --fixed-dt 0.0166667 --simulate-only --combat-state --save-file "$optimized_upper_save")
printf '%s\n' "$optimized_upper" | grep -q 'Trigger at frame 31103 from .*grass_exploration_3.xml .*forest_exploration_1.xml'
printf '%s\n' "$optimized_upper" | grep -q 'Opened kDeflection upgrade modal at frame 31587 cost=375 balance=327'
printf '%s\n' "$optimized_upper" | grep -q 'Not enough orbs for kDeflection at frame 31589 balance=327'
printf '%s\n' "$optimized_upper" | grep -q 'forest_exploration_1.xml player=(1176.53,3381.52) orbs=327 .*health=1 .*balance=327'
printf '%s\n' "$optimized_upper" | grep -q 'Deflection unlocked=0 count=0'
if printf '%s\n' "$optimized_upper" | grep -q 'Player died'; then exit 1; fi
python3 - <<'PYOPTIMIZED'
from pathlib import Path
old = Path('port/build/upper-return-save.txt').read_text().splitlines()
new = Path('port/build/optimized-upper-save.txt').read_text().splitlines()
ids = lambda lines: [line for line in lines if line.startswith('Game/XmlFiles/levels/') and '#' in line]
prior, earned = set(ids(old)), ids(new)
assert len(prior) == 306
assert len(earned) == len(set(earned)) == 327
assert prior <= set(earned) and len(set(earned) - prior) == 21
assert 'B:327' in new and 'U:kDeflection' not in new
for level in ('forest_exploration_barn.xml', 'village_forts.xml', 'village_shrine.xml', 'village_agri.xml'):
    assert any(level + '#' in orb for orb in set(earned) - prior), level
PYOPTIMIZED

# Further compatible timed normal-control jumps on the eastbound forts,
# central, shrine, and agriculture leg collect 16 new distinct authored IDs
# without changing the surviving upper-cliff scroll return.
refined_upper_save=port/build/refined-upper-save.txt
refined_upper_script="R:710,E:40,B:1200,E:40,B:1060,E:40,B:260,E:40,B:760,E:40,B:160,E:40,B:460,E:40,B:340,E:40,B:50,E:40,B:50,E:40,B:50,E:40,B:710,E:40,B:160,E:40,B:760,E:40,B:1060,E:40,B:60,E:40,B:360,Q:40,L:20,Q:40,L:250,Q:40,L:2000,Q:40,L:980,Q:40,L:4640,Q:40,L:1826,Q:40,L:245,Q:80,N:100,Q:80,N:960,Q:40,N:710,Q:40,N:50,Q:40,N:160,Q:40,N:1550,B:2560,E:80,B:2500,E:20,B:1,E:80,B:2000,E:20,B:1,E:80,B:1250,J:25,I:31,P:1"
refined_upper=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script "$refined_upper_script" --fixed-dt 0.0166667 --simulate-only --combat-state --save-file "$refined_upper_save")
printf '%s\n' "$refined_upper" | grep -q 'Trigger at frame 31103 from .*grass_exploration_3.xml .*forest_exploration_1.xml'
printf '%s\n' "$refined_upper" | grep -q 'Opened kDeflection upgrade modal at frame 31587 cost=375 balance=343'
printf '%s\n' "$refined_upper" | grep -q 'Not enough orbs for kDeflection at frame 31589 balance=343'
printf '%s\n' "$refined_upper" | grep -q 'forest_exploration_1.xml player=(1176.53,3381.52) orbs=343 .*health=1 .*balance=343'
printf '%s\n' "$refined_upper" | grep -q 'Deflection unlocked=0 count=0'
if printf '%s\n' "$refined_upper" | grep -q 'Player died'; then exit 1; fi
python3 - <<'PYREFINED'
from collections import Counter
from pathlib import Path
prior = Path('port/build/optimized-upper-save.txt').read_text().splitlines()
current = Path('port/build/refined-upper-save.txt').read_text().splitlines()
ids = lambda lines: [line for line in lines if line.startswith('Game/XmlFiles/levels/') and '#' in line]
old, earned = set(ids(prior)), ids(current)
assert len(old) == 327
assert len(earned) == len(set(earned)) == 343
assert old <= set(earned) and len(set(earned) - old) == 16
assert Counter(x.split('/')[-1].split('#')[0] for x in set(earned) - old) == {
    'village_forts.xml': 3,
    'village_central.xml': 10,
    'village_shrine.xml': 2,
    'village_agri.xml': 1,
}
assert 'B:343' in current and 'U:kDeflection' not in current
PYREFINED

# One continuous normal-control forest-start run earns exactly 375 distinct
# authored orbs, buys the actual forest scroll at production price, then
# revisits the authored village graph and deflects a fresh shrine ninja star.
# The separate saved snapshots prove the debit without respawning pickups.
earned_purchase_script=$(cat port/tests/routes/earned_deflection_purchase.script)
earned_purchase_save=port/build/earned-purchase-save.txt
earned_purchase=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script "$earned_purchase_script" --fixed-dt 0.0166667 --simulate-only --combat-state --save-file "$earned_purchase_save")
printf '%s\n' "$earned_purchase" | grep -q 'Opened kDeflection upgrade modal at frame 31619 cost=375 balance=375'
printf '%s\n' "$earned_purchase" | grep -q 'Purchased kDeflection at frame 31621 balance=0'
printf '%s\n' "$earned_purchase" | grep -q 'forest_exploration_1.xml player=(1176.53,3381.52) orbs=375 .*health=1 .*balance=0'
printf '%s\n' "$earned_purchase" | grep -q 'Deflection unlocked=1 count=0'
if printf '%s\n' "$earned_purchase" | grep -q 'Player died'; then exit 1; fi
earned_deflect_suffix='R:550,E:40,B:1200,E:40,B:1060,E:40,B:260,E:40,B:760,E:40,B:160,E:40,B:460,E:40,B:340,E:40,B:50,E:40,B:50,E:40,B:50,E:40,B:550,E:40,B:70,M:25,B:25,E:40,B:160,E:40,B:760,E:40,B:1060,E:40,B:60,E:40,B:360'
earned_deflect_save=port/build/earned-deflect-save.txt
earned_deflect=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script "$earned_purchase_script,$earned_deflect_suffix" --fixed-dt 0.0166667 --simulate-only --combat-state --save-file "$earned_deflect_save")
printf '%s\n' "$earned_deflect" | grep -q 'Purchased kDeflection at frame 31621 balance=0'
printf '%s\n' "$earned_deflect" | grep -q 'Trigger at frame 37304 from .*village_central.xml .*village_shrine.xml'
printf '%s\n' "$earned_deflect" | grep -q 'village_agri.xml player=(1951.18,-1278.17) orbs=383 .*health=1 .*balance=8 .*spawned=18'
printf '%s\n' "$earned_deflect" | grep -q 'Deflection unlocked=1 count=1'
if printf '%s\n' "$earned_deflect" | grep -q 'Player died'; then exit 1; fi
python3 - <<'PYEARNEDEDFLECT'
from pathlib import Path
ids = lambda lines: [line for line in lines if line.startswith('Game/XmlFiles/levels/') and '#' in line]
purchase = Path('port/build/earned-purchase-save.txt').read_text().splitlines()
deflected = Path('port/build/earned-deflect-save.txt').read_text().splitlines()
a, b = ids(purchase), ids(deflected)
assert len(a) == len(set(a)) == 375
assert len(b) == len(set(b)) == 383 and set(a) <= set(b)
assert 'B:0' in purchase and 'B:8' in deflected
assert 'U:kDeflection' in purchase and 'U:kDeflection' in deflected
assert any(line.startswith('N:Game/XmlFiles/levels/village_shrine.xml#189\t') for line in deflected)
PYEARNEDEDFLECT

# Continue the same earned-deflection run through both authored key chains:
# agriculture dojo key -> shrine/dojo doors -> hut key -> central hut door.
earned_dojo_hut_suffix=$(cat port/tests/routes/earned_dojo_hut_suffix.script)
earned_dojo_hut_save=port/build/earned-dojo-hut-save.txt
earned_dojo_hut=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script "$earned_purchase_script,$earned_deflect_suffix,$earned_dojo_hut_suffix" --fixed-dt 0.0166667 --simulate-only --combat-state --save-file "$earned_dojo_hut_save")
printf '%s\n' "$earned_dojo_hut" | grep -q 'Purchased kDeflection at frame 31621 balance=0'
printf '%s\n' "$earned_dojo_hut" | grep -q 'Collected key dojo at frame 40444'
printf '%s\n' "$earned_dojo_hut" | grep -q 'Door at frame 42198 from .*village_shrine.xml to .*dojo_front.xml'
printf '%s\n' "$earned_dojo_hut" | grep -q 'Unlocked door dojo at frame 42428'
printf '%s\n' "$earned_dojo_hut" | grep -q 'Door at frame 42511 from .*dojo_front.xml to .*dojo.xml'
printf '%s\n' "$earned_dojo_hut" | grep -q 'Collected key village_hut at frame 42658'
printf '%s\n' "$earned_dojo_hut" | grep -q 'Trigger at frame 44229 from .*village_shrine.xml .*village_central.xml'
printf '%s\n' "$earned_dojo_hut" | grep -q 'Unlocked door village_hut_entrance at frame 44747'
printf '%s\n' "$earned_dojo_hut" | grep -q 'Door at frame 44830 from .*village_central.xml to .*village_hut.xml'
printf '%s\n' "$earned_dojo_hut" | grep -q 'village_hut.xml player=(-205,-21.5506) orbs=388 keys=2 health=1 .*balance=13'
printf '%s\n' "$earned_dojo_hut" | grep -q 'Deflection unlocked=1 count=1'
if printf '%s\n' "$earned_dojo_hut" | grep -q 'Player died'; then exit 1; fi
python3 - <<'PYDOJO'
from pathlib import Path
lines = Path('port/build/earned-dojo-hut-save.txt').read_text().splitlines()
ids = [x for x in lines if x.startswith('Game/XmlFiles/levels/') and '#' in x]
assert len(ids) == len(set(ids)) == 388
assert all(x in lines for x in ('B:13', 'K:dojo', 'K:village_hut', 'D:dojo', 'D:village_hut_entrance', 'U:kDeflection'))
assert not any(x.startswith('K:has_repair_tools') for x in lines)
PYDOJO

# Guard the tested fixture against drifting away from the actual source graph.
python3 - <<'PYBOATXML'
from pathlib import Path
from xml.etree import ElementTree as ET
cliff = ET.parse('Game/XmlFiles/levels/cliff.xml').getroot()
boat = next(x for x in cliff if x.tag == 'Boat' and x.attrib.get('id') == '209')
assert abs(float(boat.find('position').attrib['x']) - (-3856.858398)) < 0.001
assert any(x.tag == 'KeyPickup' and x.find('key_props').attrib.get('id') == 'has_repair_tools'
           for x in ET.parse('Game/XmlFiles/levels/catacombs_entrance_1.xml').getroot())
assert 'sea_2.xml' in Path('Game/src/GameObjects/Boat.cpp').read_text()
PYBOATXML
# Boat::EnterBoat is the only source path into sea_2. An authored repair-tools
# key is required; neither a plain click nor a warmup-bypassing click sails.
boat_early=$($bin --play port/tests/fixtures/boat_gate.xml --script I:4,R:38,I:2 --fixed-dt 0.0166667 --simulate-only)
if printf '%s\n' "$boat_early" | grep -q 'Boat at frame'; then exit 1; fi
boat_broken=$($bin --play port/tests/fixtures/boat_gate.xml --script R:43,I:4 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$boat_broken" | grep -q 'Boat broken: requires has_repair_tools'
printf '%s\n' "$boat_broken" | grep -q 'Simulation ended at port/tests/fixtures/boat_gate.xml'
boat_repaired=$($bin --play port/tests/fixtures/boat_gate.xml --script I:4,R:43,I:4 --fixed-dt 0.0166667 --simulate-only --save-file port/build/boat-repaired-save.txt)
printf '%s\n' "$boat_repaired" | grep -q 'Collected key has_repair_tools at frame 2'
printf '%s\n' "$boat_repaired" | grep -q 'Boat at frame .* from port/tests/fixtures/boat_gate.xml to Game/XmlFiles/levels/sea_2.xml'
printf '%s\n' "$boat_repaired" | grep -q 'Simulation ended at Game/XmlFiles/levels/sea_2.xml'
# Source Player::Update freezes input in sea 2, shows the red title after
# strictly >15 s, and invokes EndStory after strictly >35 s. The Linux
# completion clears its progression save and returns to the menu state.
final_before=$($bin --play Game/XmlFiles/levels/sea_2.xml --script R:900 --fixed-dt 0.0166667 --simulate-only)
if printf '%s\n' "$final_before" | grep -q 'Final title'; then exit 1; fi
printf '%s\n' "$final_before" | grep -q 'player=(-4208.85,-1028.31)'
final_title=$($bin --play Game/XmlFiles/levels/sea_2.xml --script R:901 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$final_title" | grep -q 'Final title DEATH LEAK at frame 900'
if printf '%s\n' "$final_title" | grep -q 'EndStory'; then exit 1; fi
printf 'stale progression\n' > port/build/integration-final-wipe.txt
final_end=$($bin --play Game/XmlFiles/levels/sea_2.xml --script R:2160 --fixed-dt 0.0166667 --simulate-only --save-file port/build/integration-final-wipe.txt)
printf '%s\n' "$final_end" | grep -q 'EndStory at frame 2100'
printf '%s\n' "$final_end" | grep -q 'Main menu return event after story completion'
python3 - <<'PYMETA'
from pathlib import Path
p = Path('port/build/integration-final-wipe.txt')
assert p.read_text().splitlines() == ['DL6', 'MENU', 'C:1']
PYMETA
# Original SaveManager retains language and increments completion count across
# wipes. A metadata-only save must not be resumable as live progression.
printf 'DL6\nMENU\nC:2\nG:pt-br\nK:stale_key\n' > port/build/integration-final-wipe.txt
final_again=$($bin --play Game/XmlFiles/levels/sea_2.xml --script R:2160 --fixed-dt 0.0166667 --simulate-only --save-file port/build/integration-final-wipe.txt)
printf '%s\n' "$final_again" | grep -q 'completion count=3 language=pt-br'
python3 - <<'PYMETA'
from pathlib import Path
assert Path('port/build/integration-final-wipe.txt').read_text().splitlines() == ['DL6', 'MENU', 'C:3', 'G:pt-br']
PYMETA
if $bin --load-save port/build/integration-final-wipe.txt --frames 1 --fixed-dt 0.0166667 --simulate-only >/dev/null 2>&1; then exit 1; fi
# A fresh run using the same file keeps metadata but begins at the XML spawn.
fresh=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script .:1 --fixed-dt 0.0166667 --simulate-only --save-file port/build/integration-final-wipe.txt)
printf '%s\n' "$fresh" | grep -q 'orbs=0 keys=0 health=25'
grep -q '^C:3$' port/build/integration-final-wipe.txt
grep -q '^G:pt-br$' port/build/integration-final-wipe.txt
if grep -q '^K:stale_key$' port/build/integration-final-wipe.txt; then exit 1; fi
# The Linux CLI stands in for the source language-selection UI until menu
# parity exists; the selection persists through active save and EndStory.
locale_save=port/build/integration-language-save.txt
printf 'DL6\nMENU\nC:0\n' > "$locale_save"
$bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script .:1 --fixed-dt 0.0166667 --simulate-only --language de --save-file "$locale_save" >/dev/null
grep -q '^G:de$' "$locale_save"
$bin --load-save "$locale_save" --script .:1 --fixed-dt 0.0166667 --simulate-only >/dev/null
grep -q '^G:de$' "$locale_save"
$bin --play Game/XmlFiles/levels/sea_2.xml --script .:2160 --fixed-dt 0.0166667 --simulate-only --save-file "$locale_save" >/dev/null
python3 - <<'PYMETA'
from pathlib import Path
assert Path('port/build/integration-language-save.txt').read_text().splitlines() == ['DL6', 'MENU', 'C:1', 'G:de']
PYMETA
# mainmenu.xml New Game loads grass_exploration_3.xml. The Linux menu
# provides keyboard/controller focus and a deterministic scripted equivalent.
menu_save=port/build/integration-menu-save.txt
printf 'DL6\nMENU\nC:2\nG:pt-br\nK:stale_key\n' > "$menu_save"
menu_fresh=$($bin --menu-loop --script S:1,.:2 --fixed-dt 0.0166667 --simulate-only --save-file "$menu_save")
printf '%s\n' "$menu_fresh" | grep -q 'Menu New Game at frame 0 entered Game/XmlFiles/levels/grass_exploration_3.xml completion count=2 language=pt-br'
printf '%s\n' "$menu_fresh" | grep -q 'orbs=0 keys=0 health=25 .*balance=0'
grep -q '^C:2$' "$menu_save"
grep -q '^G:pt-br$' "$menu_save"
if grep -q '^K:stale_key$' "$menu_save"; then exit 1; fi
# Navigate from Play to Options, choose German, return to menu, then quit.
printf 'DL6\nMENU\nC:2\nG:pt-br\n' > "$menu_save"
menu_options=$($bin --menu-loop --script N:1,S:1,N:1,.:1,N:1,S:1,N:1,.:1,N:1,S:1,X:1,.:1,X:1,.:1,X:1 --fixed-dt 0.0166667 --simulate-only --save-file "$menu_save")
printf '%s\n' "$menu_options" | grep -q 'Menu language selected de at frame 9'
printf '%s\n' "$menu_options" | grep -q 'Menu idle completion count=2 language=de'
python3 - <<'PYMENU'
from pathlib import Path
assert Path('port/build/integration-menu-save.txt').read_text().splitlines() == ['DL6','MENU','C:2','G:de']
PYMENU
# Original Settings persists music/SFX separately from the bargon save.
settings_file=port/build/integration-settings.xml
rm -f "$settings_file"
settings_first=$($bin --menu-loop --script N:1,S:1,.:1,S:1,N:1,.:1,S:1 --fixed-dt 0.0166667 --simulate-only --save-file "$menu_save" --settings-file "$settings_file")
printf '%s\n' "$settings_first" | grep -q 'Menu settings music=0 sfx=1 at frame 3'
printf '%s\n' "$settings_first" | grep -q 'Menu settings music=0 sfx=0 at frame 6'
python3 - <<'PYSETTINGS'
from pathlib import Path
import xml.etree.ElementTree as ET
audio=ET.parse('port/build/integration-settings.xml').getroot().find('audio')
assert audio.attrib == {'music_enabled':'0','sfx_enabled':'0'}
assert Path('port/build/integration-menu-save.txt').read_text().splitlines() == ['DL6','MENU','C:2','G:de']
PYSETTINGS
settings_reload=$($bin --menu-loop --script N:1,S:1,.:1,S:1 --fixed-dt 0.0166667 --simulate-only --settings-file "$settings_file")
printf '%s\n' "$settings_reload" | grep -q 'Menu settings music=1 sfx=0 at frame 3'
python3 - <<'PYSETTINGS'
import xml.etree.ElementTree as ET
audio=ET.parse('port/build/integration-settings.xml').getroot().find('audio')
assert audio.attrib == {'music_enabled':'1','sfx_enabled':'0'}
PYSETTINGS
# The ending returns to an actual menu in-process. A following normal menu
# action starts a fresh authored spawn with completion metadata but no old keys.
printf 'DL6\nMENU\nC:2\nG:de\nK:stale_key\n' > "$menu_save"
menu_end=$($bin --play Game/XmlFiles/levels/sea_2.xml --menu-loop --script .:2101,S:1,.:1 --fixed-dt 0.0166667 --simulate-only --save-file "$menu_save")
printf '%s\n' "$menu_end" | grep -q 'Main menu return after story completion count=3 language=de'
printf '%s\n' "$menu_end" | grep -q 'Menu New Game at frame 2101 entered Game/XmlFiles/levels/grass_exploration_3.xml completion count=3 language=de'
printf '%s\n' "$menu_end" | grep -q 'orbs=0 keys=0 health=25 .*balance=0'
grep -q '^C:3$' "$menu_save"
if grep -q '^K:stale_key$' "$menu_save"; then exit 1; fi
menu_idle=$($bin --play Game/XmlFiles/levels/sea_2.xml --menu-loop --script .:2102 --fixed-dt 0.0166667 --simulate-only --save-file "$menu_save")
printf '%s\n' "$menu_idle" | grep -q 'Menu idle completion count=4 language=de afterEndStory=1'
python3 - <<'PYMENU'
from pathlib import Path
assert Path('port/build/integration-menu-save.txt').read_text().splitlines() == ['DL6','MENU','C:4','G:de']
PYMENU
# Full source-authored progression: one process, original forest spawn, only
# ordinary controls, no save load/state injection. The pre-boat snapshot is
# inspected separately because EndStory clears the selected progression save.
earned_full_script=$(cat port/tests/routes/earned_full_ending.script)
full_preboat=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script "$earned_full_script" --frames 57822 --fixed-dt 0.0166667 --simulate-only --save-file port/build/earned-full-preboat.txt)
printf '%s\n' "$full_preboat" | grep -q 'Collected key granary at frame 25665'
printf '%s\n' "$full_preboat" | grep -q 'Purchased kDeflection at frame 33211 balance=9'
printf '%s\n' "$full_preboat" | grep -q 'Collected key dojo at frame 42034'
printf '%s\n' "$full_preboat" | grep -q 'Collected key village_hut at frame 44248'
printf '%s\n' "$full_preboat" | grep -q 'Collected key has_repair_tools at frame 49742'
printf '%s\n' "$full_preboat" | grep -q 'Simulation ended at Game/XmlFiles/levels/cliff.xml.*orbs=406 keys=4 health=1 .*balance=31'
python3 - <<'PYTEST'
from pathlib import Path
lines = Path('port/build/earned-full-preboat.txt').read_text().splitlines()
assert lines[0] == 'DL5' and lines[1].endswith('/cliff.xml')
ids = [line for line in lines[4:] if '#' in line and not line.startswith('N:')]
assert len(ids) == len(set(ids)) == 406
assert 'B:31' in lines
assert set(x for x in lines if x.startswith('K:')) == {'K:granary','K:dojo','K:village_hut','K:has_repair_tools'}
assert 'U:kDeflection' in lines
PYTEST
printf 'stale progression\n' > port/build/earned-full-completion-save.txt
full_end=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script "$earned_full_script" --fixed-dt 0.0166667 --simulate-only --combat-state --save-file port/build/earned-full-completion-save.txt)
printf '%s\n' "$full_end" | grep -q 'Boat at frame 57822 from .*cliff.xml to .*sea_2.xml'
printf '%s\n' "$full_end" | grep -q 'Final title DEATH LEAK at frame 58723'
printf '%s\n' "$full_end" | grep -q 'EndStory at frame 59923'
printf '%s\n' "$full_end" | grep -q 'Main menu return event after story completion'
printf '%s\n' "$full_end" | grep -q 'orbs=406 keys=4 health=1 .*balance=31'
printf '%s\n' "$full_end" | grep -q 'Deflection unlocked=1 count=1'
if printf '%s\n' "$full_end" | grep -q 'Player died'; then exit 1; fi
python3 - <<'PYMETA'
from pathlib import Path
assert Path('port/build/earned-full-completion-save.txt').read_text().splitlines() == ['DL6', 'MENU', 'C:1']
PYMETA
grep -q '^K:has_repair_tools$' port/build/boat-repaired-save.txt

# A held left+jump just before grass 1's lower dead-end trigger keeps the
# player on the upper route to grass 2 and the authored cliff. Reversing on
# the reachable lower cliff path collects more distinct authored orbs; its
# invisible east solid (id 30) stops this route, so this is not a forest return.
grass_cliff_save=port/build/grass-cliff-save.txt
grass_cliff_route=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script L:1500,Q:40,L:1000,Q:40,L:2800,R:3000 --fixed-dt 0.0166667 --simulate-only --save-file "$grass_cliff_save")
printf '%s\n' "$grass_cliff_route" | grep -q 'Trigger at frame 2783 from .*grass_exploration_1.xml .*grass_exploration_2.xml'
printf '%s\n' "$grass_cliff_route" | grep -q 'Trigger at frame 3942 from .*grass_exploration_2.xml .*cliff.xml'
printf '%s\n' "$grass_cliff_route" | grep -q 'cliff.xml player=(-1500.15,-944.093) orbs=99 .*health=1 .*balance=99'
if printf '%s\n' "$grass_cliff_route" | grep -q 'Player died'; then exit 1; fi
python3 - <<'PYGRASS'
from pathlib import Path
import xml.etree.ElementTree as ET
lines = Path('port/build/grass-cliff-save.txt').read_text().splitlines()
orb_ids = [line for line in lines if line.startswith('Game/XmlFiles/levels/') and '#' in line]
assert len(orb_ids) == len(set(orb_ids)) == 99, (len(orb_ids), len(set(orb_ids)))
assert 'B:99' in lines
root = ET.parse('Game/XmlFiles/levels/cliff.xml').getroot()
wall = next(o for o in root if o.tag == 'SolidMovingSprite' and o.get('id') == '30')
left = float(wall.find('position').get('x')) - float(wall.find('collisionboxdimensions').get('x')) / 2
player = next(o for o in root if o.tag == 'Player')
half = float(player.find('collisionboxdimensions').get('x')) / 2
assert abs(left - half - (-1500.15)) < 0.02
assert wall.find('passive').get('value') == 'true'
assert wall.find('applydamage').get('flag') == 'false'
PYGRASS
# The village-central eastward death with a constant run is avoidable by ordinary
# right+jump timing: the first jump evades the ninja shot, the second clears
# the authored village obstacle, and the third stays above the optional well
# entrance. This same forest-spawn simulation reaches shrine and agriculture;
# every orb is a distinct collected authored object, never injected currency.
extended_save=port/build/extended-village-save.txt
extended_route=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script R:710,E:40,B:4480,E:40,B:50,E:40,B:50,E:40,B:3400 --fixed-dt 0.0166667 --simulate-only --save-file "$extended_save")
printf '%s\n' "$extended_route" | grep -q 'Trigger at frame 5785 .*village_shrine.xml'
printf '%s\n' "$extended_route" | grep -q 'Trigger at frame 8206 .*village_agri.xml'
printf '%s\n' "$extended_route" | grep -q 'village_agri.xml player=(2711.18,-1286.6) orbs=187 .*health=1 .*balance=187'
if printf '%s\n' "$extended_route" | grep -q 'Player died'; then exit 1; fi
python3 - <<'PYTEST'
from pathlib import Path
lines = Path('port/build/extended-village-save.txt').read_text().splitlines()
orb_ids = [line for line in lines if line.startswith('Game/XmlFiles/levels/') and '#' in line]
assert len(orb_ids) == len(set(orb_ids)) == 187, (len(orb_ids), len(set(orb_ids)))
assert 'B:187' in lines
PYTEST
# Agriculture's east edge is an authored 4,000-unit-tall SolidMovingSprite
# (id 14), not a missing LevelTrigger. Its left face stops this route at
# x=3519.8 even with a normal held right+jump. A retimed westbound jump
# avoids the optional village well. The source Player sprite bounds overlap
# the forts west trigger even though its narrower collision box is blocked.
# A further held left+jump crosses waterfall top back to forest 1.
agri_wall=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script R:710,E:40,B:4480,E:40,B:50,E:40,B:50,E:40,B:3600,E:40,R:200 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$agri_wall" | grep -q 'Player died at frame 8952 in Game/XmlFiles/levels/village_agri.xml'
printf '%s\n' "$agri_wall" | grep -q 'village_agri.xml player=(3519.8,-1288.5) orbs=191 .*health=0 .*balance=191'
return_save=port/build/agri-return-save.txt
return_route=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script R:710,E:40,B:4480,E:40,B:50,E:40,B:50,E:40,B:3400,L:3410,Q:40,L:4640,Q:40,L:800 --fixed-dt 0.0166667 --simulate-only --save-file "$return_save")
printf '%s\n' "$return_route" | grep -q 'Trigger at frame 9527 from .*village_agri.xml .*village_shrine.xml'
printf '%s\n' "$return_route" | grep -q 'Trigger at frame 11943 from .*village_shrine.xml .*village_central.xml'
printf '%s\n' "$return_route" | grep -q 'Trigger at frame 13701 from .*village_central.xml .*village_forts.xml'
printf '%s\n' "$return_route" | grep -q 'Trigger at frame 15025 from .*village_forts.xml .*forest_exploration_barn.xml'
printf '%s\n' "$return_route" | grep -q 'Trigger at frame 16010 from .*forest_exploration_barn.xml .*forest_exploration_2.xml'
printf '%s\n' "$return_route" | grep -q 'Trigger at frame 16651 from .*forest_exploration_2.xml .*waterfall_top.xml'
printf '%s\n' "$return_route" | grep -q 'Trigger at frame 17306 from .*waterfall_top.xml .*forest_exploration_1.xml'
printf '%s\n' "$return_route" | grep -q 'forest_exploration_1.xml player=(-1078.02,3110.97) orbs=190 .*health=1 .*balance=190'
# At the forts wall the player center is still outside trigger 13, but its
# authored visual rectangle overlaps. Collision and trigger bounds differ.
python3 - <<'PYGEOMETRY'
import xml.etree.ElementTree as ET
root = ET.parse('Game/XmlFiles/levels/village_forts.xml').getroot()
objects = {o.get('id'): o for o in root if o.get('id')}
wall, trigger, player = (objects[i] for i in ('11', '13', '3'))
wx = float(wall.find('position').get('x'))
wall_right = wx + float(wall.find('collisionboxdimensions').get('x')) / 2
tx = float(trigger.find('position').get('x'))
trigger_right = tx + float(trigger.find('dimensions').get('width')) / 2
visual_half = float(player.find('dimensions').get('width')) / 2
collision_half = float(player.find('collisionboxdimensions').get('x')) / 2
stopped_center = wall_right + collision_half
assert stopped_center > trigger_right  # center-only port check misses it
assert stopped_center - visual_half < trigger_right  # original trigger fires
assert wall.find('passive').get('value') == 'true'
assert wall.find('applydamage').get('flag') == 'false'
PYGEOMETRY
if printf '%s\n' "$return_route" | grep -q 'Player died\|water_well_1.xml'; then exit 1; fi
python3 - <<'PYTEST'
from pathlib import Path
lines = Path('port/build/agri-return-save.txt').read_text().splitlines()
orb_ids = [line for line in lines if line.startswith('Game/XmlFiles/levels/') and '#' in line]
assert len(orb_ids) == len(set(orb_ids)) == 190, (len(orb_ids), len(set(orb_ids)))
assert 'B:190' in lines
PYTEST
# Source SolidMovingSprite boundaries share geometry but not damage policy.
# An attempted side overlap calls OnCollision once per simulated frame here;
# unlike a water region, 2 damage is not multiplied by dt.
solid_hit=$($bin --play port/tests/fixtures/solid_boundary_damaging.xml --script R:18 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$solid_hit" | grep -q 'player=(115.52,107.449) .*health=23 '
solid_repeat=$($bin --play port/tests/fixtures/solid_boundary_damaging.xml --script R:19 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$solid_repeat" | grep -q 'player=(115.52,107.449) .*health=21 '
solid_safe=$($bin --play port/tests/fixtures/solid_boundary_harmless.xml --script R:18 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$solid_safe" | grep -q 'player=(115.52,107.449) .*health=25 '
# Forest 2's authored barn LevelEntry starts about 21 units embedded in its
# ground strip. The one-frame shallow floor resolution preserves the route.
# These independent branch checks supplement the connected 190-orb forest loop.
village_route=$($bin --play Game/XmlFiles/levels/forest_exploration_2.xml --script R:3500 --fixed-dt 0.0166667 --simulate-only --save-file port/build/village-route-save.txt)
printf '%s\n' "$village_route" | grep -q 'Trigger at frame 81 .*forest_exploration_barn.xml'
printf '%s\n' "$village_route" | grep -q 'Trigger at frame 1081 .*village_forts.xml'
printf '%s\n' "$village_route" | grep -q 'Trigger at frame 2404 .*village_central.xml'
printf '%s\n' "$village_route" | grep -q 'village_central.xml player=(6351.18,-1424.49) orbs=60 .*health=10 .*balance=60'
python3 - <<'PYTEST'
from pathlib import Path
lines = Path('port/build/village-route-save.txt').read_text().splitlines()
orb_ids = [line for line in lines if line.startswith('Game/XmlFiles/levels/') and '#' in line]
assert len(orb_ids) == len(set(orb_ids)) == 60, (len(orb_ids), len(set(orb_ids)))
assert 'B:60' in lines
PYTEST
grass_route=$($bin --play Game/XmlFiles/levels/forest_exploration_1.xml --script L:1500,Q:40,L:1800 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$grass_route" | grep -q 'Entered Game/XmlFiles/levels/grass_exploration_dead_end_below_2.xml'
printf '%s\n' "$grass_route" | grep -q 'orbs=50 .*health=22 .*balance=50'
# DojoScrollPickup ignores authored debug orb_cost=2; production feature cost is
# 375. One normal orb pickup adds to a loaded 374-orb balance. Modal pauses
# world updates, purchase debits once, hides the scroll, and survives reload.
dojo=port/tests/fixtures/dojo_deflection.xml
dojo_save=port/build/integration-dojo-save.txt
printf 'DL5\n%s\n0 107.44937 25 100\nB:373\n' "$dojo" > "$dojo_save"
dojo_short=$($bin --load-save "$dojo_save" --script I:29 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$dojo_short" | grep -q 'balance=374'
printf '%s\n' "$dojo_short" | grep -q 'Deflection unlocked=0 count=0'
printf 'DL5\n%s\n0 107.44937 25 50\nB:374\n' "$dojo" > "$dojo_save"
dojo_paused=$($bin --load-save "$dojo_save" --script I:31,.:60 --fixed-dt 0.0166667 --simulate-only)
printf '%s\n' "$dojo_paused" | grep -q 'focus=55 balance=375'
printf 'DL5\n%s\n0 107.44937 25 100\nB:373\n' "$dojo" > "$dojo_save"
dojo_insufficient=$($bin --load-save "$dojo_save" --script I:31,P:1 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$dojo_insufficient" | grep -q 'Opened kDeflection upgrade modal.*cost=375 balance=374'
printf '%s\n' "$dojo_insufficient" | grep -q 'Not enough orbs.*balance=374'
printf '%s\n' "$dojo_insufficient" | grep -q 'Deflection unlocked=0 count=0'
printf 'DL5\n%s\n0 107.44937 25 100\nB:374\n' "$dojo" > "$dojo_save"
dojo_purchased=$($bin --load-save "$dojo_save" --script I:31,P:1 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$dojo_purchased" | grep -q 'Purchased kDeflection.*balance=0'
printf '%s\n' "$dojo_purchased" | grep -q 'Deflection unlocked=1 count=0'
grep -q '^B:0$' "$dojo_save"
grep -q '^U:kDeflection$' "$dojo_save"
dojo_resume=$($bin --load-save "$dojo_save" --script I:40,P:2 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$dojo_resume" | grep -q 'balance=0'
printf '%s\n' "$dojo_resume" | grep -q 'Deflection unlocked=1 count=0'
if printf '%s\n' "$dojo_resume" | grep -q 'Opened kDeflection'; then exit 1; fi
# Cancel after modal warmup without spending; world remains paused while open.
printf 'DL5\n%s\n0 107.44937 25 100\nB:374\n' "$dojo" > "$dojo_save"
dojo_cancel=$($bin --load-save "$dojo_save" --script I:31,.:61,X:1 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$dojo_cancel" | grep -q 'Dismissed kDeflection modal'
printf '%s\n' "$dojo_cancel" | grep -q 'balance=375'
printf '%s\n' "$dojo_cancel" | grep -q 'Deflection unlocked=0 count=0'
# Source Character::WillDeflectProjectile accepts only front-side, opposing
# shots during melee phase 1 or 3. The feature flag represents an already
# purchased upgrade in the Linux save; this fixture isolates combat gates.
deflect_save=port/build/integration-deflection-save.txt
make_deflect_save() {
    printf 'DL4\n%s\n0 107.44937 25 100\nU:kDeflection\n' "$1" > "$deflect_save"
}
deflect_locked=$($bin --play port/tests/fixtures/encounter_open.xml --script .:100,M:6 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$deflect_locked" | grep -q 'health=25 .*shots=1'
printf '%s\n' "$deflect_locked" | grep -q 'Deflection unlocked=0 count=0'
printf '%s\n' "$deflect_locked" | grep -q 'Shot enemy .*deflected=0'
make_deflect_save port/tests/fixtures/encounter_open.xml
deflect_phase1=$($bin --load-save "$deflect_save" --script .:100,M:6 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$deflect_phase1" | grep -q 'health=25 .*shots=1'
printf '%s\n' "$deflect_phase1" | grep -q 'Deflection unlocked=1 count=1'
printf '%s\n' "$deflect_phase1" | grep -q 'Shot player .*vx=1350 .*deflected=1'
grep -q '^U:kDeflection$' "$deflect_save"
make_deflect_save port/tests/fixtures/encounter_open.xml
deflect_phase2=$($bin --load-save "$deflect_save" --script .:88,M:18 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$deflect_phase2" | grep -q 'Deflection unlocked=1 count=0'
printf '%s\n' "$deflect_phase2" | grep -q 'Shot enemy .*deflected=0'
make_deflect_save port/tests/fixtures/encounter_open.xml
deflect_phase3=$($bin --load-save "$deflect_save" --script .:80,M:40 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$deflect_phase3" | grep -q 'health=25 .*enemies=0 hits=1 kills=1'
printf '%s\n' "$deflect_phase3" | grep -q 'Melee active=0 strikes=0'
printf '%s\n' "$deflect_phase3" | grep -q 'Deflection unlocked=1 count=1'
make_deflect_save port/tests/fixtures/encounter_open.xml
deflect_behind=$($bin --load-save "$deflect_save" --script L:1,.:99,M:6 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$deflect_behind" | grep -q 'Deflection unlocked=1 count=0'
printf '%s\n' "$deflect_behind" | grep -q 'Shot enemy .*deflected=0'
make_deflect_save port/tests/fixtures/encounter_skeleton_open.xml
deflect_skeleton=$($bin --load-save "$deflect_save" --script .:110,M:10 --fixed-dt 0.0166667 --simulate-only --combat-state)
printf '%s\n' "$deflect_skeleton" | grep -q 'health=25 .*shots=1'
printf '%s\n' "$deflect_skeleton" | grep -q 'Deflection unlocked=1 count=0'
printf '%s\n' "$deflect_skeleton" | grep -q 'Shot enemy .*deflected=0'
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
printf '%s\n' 'PASS: source-held-jump arena climb and exit, three-level route, burst/water fire, swim, focus cooldown, encounter, save/resume, drowning, landing/jump, combined jump, slope/wall, solid box, wall jump, death/restart, key/door gate, ghost flight, ranged warmup/terrain shots and kill, grounded NPC fall/wall/box, NPC combo/stun/recovery, NPC teleport timing/exclusion, ranged NPC jump/roll and projectile kickback, source-faithful player/NPC contact pass-through, grounded phase-three melee and its negative gates, DL3/DL4 compatibility, DL5 orb-balance persistence, authentic 375-orb purchase/debit and same-run authored ninja deflection, continuous dojo/hut key-door route, repair-gated boat transition, and retained completion count/language after source-style progression wipe'

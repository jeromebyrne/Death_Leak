#!/bin/sh
# Uninterrupted normal-control encounter damage and two authored onward exits.
# CPU/offscreen functional gate only; synthetic slots are not physical display Hz.
set -eu
export SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl
export DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1 DEATHLEAK_TRACE_ENCOUNTERS=1
script='L:1500,Q:40,L:100,F:300,R:300,F:300,L:245,Q:80,N:100,Q:80,N:960,Q:40,N:710,Q:40,N:50,Q:40,N:160,Q:40,N:2200'
expected=''
for hz in 60 120 144; do
  output=$(port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script "$script" --frames 7285 --simulate-only --combat-state --test-refresh-hz "$hz" 2>&1)
  printf '%s\n' "$output" | grep -q 'Trigger at frame 300 from .*forest_exploration_1.xml .*grass_exploration_3.xml'
  printf '%s\n' "$output" | grep -q 'Trigger at frame 1093 from .*grass_exploration_3.xml .*first_ninja_arena.xml'
  printf '%s\n' "$output" | grep -q 'Trigger at frame 3277 from .*first_ninja_arena.xml .*grass_exploration_1.xml'
  printf '%s\n' "$output" | grep -q 'Trigger at frame 3860 from .*grass_exploration_1.xml .*grass_exploration_dead_end_below_2.xml'
  printf '%s\n' "$output" | grep -q 'Simulation ended at Game/XmlFiles/levels/grass_exploration_dead_end_below_2.xml .*orbs=55 keys=0 health=13 .*balance=55 spawned=3 enemies=0 hits=12 kills=0 fired=284'
  [ "$(printf '%s\n' "$output" | grep -c '^Encounter projectile hit player=')" -eq 4 ]
  if printf '%s\n' "$output" | grep -q 'Player died'; then
    echo "Unexpected death at synthetic $hz Hz" >&2; exit 1
  fi
  # Only two source increment sites exist at this ref: positive enemy damage
  # and an enemy projectile damaging the player. 12 total - 4 player hits
  # establishes eight player-to-enemy damage events, not a kill.
  state=$(printf '%s\n' "$output" | grep '^Simulation ended at ' | sed -E 's/ elapsed=[^ ]+s / /')
  transitions=$(printf '%s\n' "$output" | grep '^Trigger at frame ')
  signature=$(printf '%s\n%s\n' "$transitions" "$state")
  if [ -n "$expected" ] && [ "$signature" != "$expected" ]; then
    echo "Transition or endpoint drift at synthetic $hz Hz" >&2; exit 1
  fi
  expected=$signature
  printf 'synthetic %s Hz: four player hits, eight player-to-enemy damage events, arena and lower-grass exits verified\n' "$hz"
done

#!/bin/sh
# Source/functional diagnostics; not a physical-display or full-playthrough claim.
set -eu
python3 port/tests/technique_save_bridge_test.py
work=$(mktemp -d /tmp/deathleak-all-level-XXXXXX)
trap 'rm -rf "$work"' EXIT HUP INT TERM
flags='-std=c++17 -O0 -g0 -UNDEBUG -Iport/compat'
for file in tinyxml tinyxmlerror tinyxmlparser tinystr; do
 g++ $flags -c "Game/tinyxml/$file.cpp" -o "$work/$file.o"
done
xml_objects="$work/tinyxml.o $work/tinyxmlerror.o $work/tinyxmlparser.o $work/tinystr.o"
g++ $flags -c port/game/LevelScene.cpp -o "$work/scene.o"
g++ $flags -c port/game/PlayerMotion.cpp -o "$work/motion.o"
g++ $flags -c port/game/Combat.cpp -o "$work/combat.o"
for test in authored_platform_motion authored_ambient_actor bomb_projectile_motion bitmap_text_case upgrade_modal_layout; do
 g++ $flags "port/tests/${test}_test.cpp" $xml_objects -o "$work/$test"
 "$work/$test"
done
for test in all_level_dynamics all_level_combat_parity knife_visual_lifecycle damage_burst; do
 g++ $flags "port/tests/${test}_test.cpp" "$work/scene.o" "$work/motion.o" "$work/combat.o" $xml_objects -o "$work/$test"
 "$work/$test"
done
g++ $flags $(pkg-config --cflags sdl2) port/tests/all_level_audio_test.cpp $xml_objects -o "$work/audio" $(pkg-config --libs sdl2)
SDL_AUDIODRIVER=dummy "$work/audio"

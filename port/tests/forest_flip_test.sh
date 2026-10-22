#!/bin/sh
# Run from repository root inside deathleak-linux-build:bookworm.
set -eu
c++ -std=c++17 -O2 -Iport/compat port/tests/forest_flip_test.cpp \
    port/game/LevelScene.cpp Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp \
    Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp -o port/build/forest-flip-test
DEATHLEAK_RENDER_ONLY_XML_IDS=11 port/build/forest-flip-test 11 267.985352 3115 true 0
DEATHLEAK_RENDER_ONLY_XML_IDS=57 port/build/forest-flip-test 57 640.085083 3066 true 12.6036
DEATHLEAK_RENDER_ONLY_XML_IDS=58 port/build/forest-flip-test 58 -132.214661 3070.198242 false -12.6036
DEATHLEAK_RENDER_ONLY_XML_IDS=14 port/build/forest-flip-test 14 -2212.014648 3210 true -87.0895
DEATHLEAK_RENDER_ONLY_XML_IDS=15 port/build/forest-flip-test 15 2796.985352 3207 false 84.2247
c++ -std=c++17 -O2 $(pkg-config --cflags sdl2) \
    port/tests/sdl_rotated_flip_test.cpp -o port/build/sdl-rotated-flip-test $(pkg-config --libs sdl2)
SDL_VIDEODRIVER=dummy port/build/sdl-rotated-flip-test

CXX ?= g++
SDL_CFLAGS ?= $(shell pkg-config --cflags sdl2)
PNG_CFLAGS ?= $(shell pkg-config --cflags libpng)
SDL_LIBS ?= $(shell pkg-config --libs sdl2)
PNG_LIBS ?= $(shell pkg-config --libs libpng)
CXXFLAGS ?= -O2 -g -std=c++17 -Wall -Wextra -Wpedantic
TARGET := build/deathleak-linux

.PHONY: all clean smoke
all: $(TARGET)
$(TARGET): linux/main.cpp linux/SDLRenderer.cpp linux/SDLSoundPlayer.cpp audio/SoundPlayer.h game/Settings.h game/LevelScene.cpp game/LevelScene.h game/Combat.cpp game/Combat.h game/PlayerMotion.cpp game/PlayerMotion.h game/Hud.cpp game/Hud.h game/BitmapText.h game/PaperText.h render/Renderer.h $(wildcard ../Game/tinyxml/*.cpp)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) $(PNG_CFLAGS) -Icompat linux/main.cpp linux/SDLRenderer.cpp linux/SDLSoundPlayer.cpp game/LevelScene.cpp game/Combat.cpp game/PlayerMotion.cpp game/Hud.cpp ../Game/tinyxml/tinyxml.cpp ../Game/tinyxml/tinyxmlerror.cpp ../Game/tinyxml/tinyxmlparser.cpp ../Game/tinyxml/tinystr.cpp -o $@ $(SDL_LIBS) $(PNG_LIBS)
smoke: $(TARGET)
	cd .. && SDL_VIDEODRIVER=dummy port/$(TARGET) --frames 2 --capture port/build/smoke.bmp
clean:
	rm -rf build

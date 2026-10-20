CXX ?= g++
SDL_CFLAGS ?= $(shell pkg-config --cflags sdl2)
PNG_CFLAGS ?= $(shell pkg-config --cflags libpng)
SDL_LIBS ?= $(shell pkg-config --libs sdl2)
PNG_LIBS ?= $(shell pkg-config --libs libpng)
CXXFLAGS ?= -O2 -g -std=c++17 -Wall -Wextra -Wpedantic
TARGET := build/deathleak-linux

.PHONY: all clean smoke
all: $(TARGET)
$(TARGET): linux/main.cpp linux/SDLRenderer.cpp render/Renderer.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) $(PNG_CFLAGS) linux/main.cpp linux/SDLRenderer.cpp -o $@ $(SDL_LIBS) $(PNG_LIBS)
smoke: $(TARGET)
	cd .. && SDL_VIDEODRIVER=dummy port/$(TARGET) --frames 2 --capture port/build/smoke.bmp
clean:
	rm -rf build

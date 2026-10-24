CXX ?= g++
SDL_CFLAGS ?= $(shell pkg-config --cflags sdl2)
PNG_CFLAGS ?= $(shell pkg-config --cflags libpng)
SDL_LIBS ?= $(shell pkg-config --libs sdl2)
PNG_LIBS ?= $(shell pkg-config --libs libpng)
GL_LIBS ?= $(shell pkg-config --libs gl)
CXXFLAGS ?= -O2 -g -std=c++17 -Wall -Wextra -Wpedantic
TARGET := build/deathleak-linux

.PHONY: all clean smoke
all: $(TARGET)
$(TARGET): linux/main.cpp linux/SDLRenderer.cpp linux/RepeatX.h linux/RotatedRepeatX.h linux/UVWrap.h linux/ParallaxUV.h linux/ParallaxUVCommand.h linux/WobbleUV.h linux/FoliageSway.h linux/EffectQuadGeometry.h render/OrdinarySpriteRotation.h linux/SDLSoundPlayer.cpp audio/SoundPlayer.h game/Settings.h game/LevelScene.cpp game/LevelScene.h game/CameraShake.h render/ViewportCull.h game/ShaderClock.h game/WeatherLightning.h game/WeatherRain.h game/ParticleSpray.h game/Combat.cpp game/Combat.h game/PlayerMotion.cpp game/PlayerMotion.h game/Hud.cpp game/Hud.h game/BitmapText.h game/PaperText.h render/Renderer.h $(wildcard ../Game/tinyxml/*.cpp)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) $(PNG_CFLAGS) -Icompat linux/main.cpp linux/SDLRenderer.cpp linux/SDLSoundPlayer.cpp game/LevelScene.cpp game/Combat.cpp game/PlayerMotion.cpp game/Hud.cpp ../Game/tinyxml/tinyxml.cpp ../Game/tinyxml/tinyxmlerror.cpp ../Game/tinyxml/tinyxmlparser.cpp ../Game/tinyxml/tinystr.cpp -o $@ $(SDL_LIBS) $(PNG_LIBS) $(GL_LIBS)
smoke: $(TARGET)
	cd .. && SDL_VIDEODRIVER=dummy port/$(TARGET) --frames 2 --capture port/build/smoke.bmp
clean:
	rm -rf build

.PHONY: fractional_parallax_uv_test
fractional_parallax_uv_test: build/fractional_parallax_uv_test
	./build/fractional_parallax_uv_test

build/fractional_parallax_uv_test: tests/fractional_parallax_uv_test.cpp linux/ParallaxUV.h linux/UVWrap.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -UNDEBUG tests/fractional_parallax_uv_test.cpp -o $@

.PHONY: fog-mip-cpu-test
fog-mip-cpu-test:
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) $(PNG_CFLAGS) -Icompat tests/fog_mip_filter_cpu_test.cpp -o build/fog_mip_filter_cpu_test $(SDL_LIBS) $(PNG_LIBS) $(GL_LIBS)
	./build/fog_mip_filter_cpu_test

.PHONY: rotated-repeat-x-cpu-test
rotated-repeat-x-cpu-test: build/rotated_repeat_x_cpu_test
	./build/rotated_repeat_x_cpu_test

build/rotated_repeat_x_cpu_test: tests/rotated_repeat_x_cpu_test.cpp linux/RotatedRepeatX.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -UNDEBUG tests/rotated_repeat_x_cpu_test.cpp -o $@

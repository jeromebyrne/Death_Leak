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
$(TARGET): linux/main.cpp linux/FilmFrameSelector.h linux/FixedStepPacer.h linux/RefreshSlotScheduler.h linux/PresentationCommandPacket.h linux/MixedCommandInterpolationGate.h linux/SDLRenderer.cpp linux/LogicalViewport.h linux/DisplaySurfaceGeometry.h linux/RepeatX.h linux/RotatedRepeatX.h linux/UVWrap.h linux/ParallaxUV.h linux/ParallaxUVCommand.h linux/WobbleUV.h linux/FoliageSway.h linux/EffectQuadGeometry.h render/OrdinarySpriteRotation.h linux/SDLSoundPlayer.cpp audio/SoundPlayer.h game/Settings.h game/LevelScene.cpp game/LevelScene.h game/CameraShake.h render/ViewportCull.h game/ShaderClock.h game/WeatherLightning.h game/WeatherRain.h game/WeatherTextureResidency.h game/FilmFrameResidency.h game/LoadingOverlayPacket.h game/ParticleSpray.h game/Combat.cpp game/Combat.h game/PlayerMotion.cpp game/PlayerMotion.h game/Hud.cpp game/Hud.h game/HudPresentationPacket.h game/BitmapText.h game/PaperText.h render/Renderer.h $(wildcard ../Game/tinyxml/*.cpp)
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

.PHONY: logical-viewport-cpu-test
logical-viewport-cpu-test: build/logical_viewport_cpu_test
	./build/logical_viewport_cpu_test

build/logical_viewport_cpu_test: tests/logical_viewport_cpu_test.cpp linux/LogicalViewport.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -UNDEBUG tests/logical_viewport_cpu_test.cpp -o $@

.PHONY: hud-packet-cpu-test
hud-packet-cpu-test:
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -UNDEBUG $(SDL_CFLAGS) $(PNG_CFLAGS) -Icompat tests/hud_const_presentation_cpu_test.cpp game/Hud.cpp -o build/hud_packet_cpu_test $(SDL_LIBS) $(PNG_LIBS) $(GL_LIBS)
	./build/hud_packet_cpu_test

.PHONY: presentation-command-packet-test
presentation-command-packet-test: build/presentation_command_packet_test
	./build/presentation_command_packet_test

build/presentation_command_packet_test: tests/presentation_command_packet_test.cpp linux/PresentationCommandPacket.h render/Renderer.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -UNDEBUG tests/presentation_command_packet_test.cpp -o $@

.PHONY: refresh-slot-scheduler-test
refresh-slot-scheduler-test: build/refresh_slot_scheduler_test
	./build/refresh_slot_scheduler_test
build/refresh_slot_scheduler_test: tests/refresh_slot_scheduler_test.cpp linux/RefreshSlotScheduler.h linux/FixedStepPacer.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -UNDEBUG tests/refresh_slot_scheduler_test.cpp -o $@

.PHONY: display-surface-geometry-cpu-test
display-surface-geometry-cpu-test: build/display_surface_geometry_cpu_test
	./build/display_surface_geometry_cpu_test

build/display_surface_geometry_cpu_test: tests/display_surface_geometry_cpu_test.cpp linux/DisplaySurfaceGeometry.h linux/LogicalViewport.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -UNDEBUG tests/display_surface_geometry_cpu_test.cpp -o $@

build/display_surface_physical_probe: tests/display_surface_physical_probe.cpp linux/LogicalViewport.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) tests/display_surface_physical_probe.cpp -o $@ $(SDL_LIBS)

.PHONY: interpolation-topology-oracle-test
interpolation-topology-oracle-test: build/interpolation_topology_oracle_test
	./build/interpolation_topology_oracle_test

build/interpolation_topology_oracle_test: tests/interpolation_topology_oracle_test.cpp linux/InterpolationTopologyOracle.h linux/FixedStepPacer.h game/LevelScene.h game/ParticleSpray.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -UNDEBUG tests/interpolation_topology_oracle_test.cpp -o $@

.PHONY: real-scene-topology-capture-test
real-scene-topology-capture-test: build/real_scene_topology_capture_test
	cd .. && port/build/real_scene_topology_capture_test

build/real_scene_topology_capture_test: tests/real_scene_topology_capture_test.cpp linux/RealSceneTopologyCapture.h game/LevelScene.cpp game/LevelScene.h game/ParticleSpray.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -UNDEBUG -Icompat tests/real_scene_topology_capture_test.cpp game/LevelScene.cpp ../Game/tinyxml/tinyxml.cpp ../Game/tinyxml/tinyxmlerror.cpp ../Game/tinyxml/tinyxmlparser.cpp ../Game/tinyxml/tinystr.cpp -o $@

.PHONY: full-frame-interpolation-gate-test
full-frame-interpolation-gate-test: build/full_frame_interpolation_gate_test
	cd .. && port/build/full_frame_interpolation_gate_test
build/full_frame_interpolation_gate_test: tests/full_frame_interpolation_gate_test.cpp linux/FullFrameInterpolationGate.h linux/PresentationCommandPacket.h game/LevelScene.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -UNDEBUG -Icompat tests/full_frame_interpolation_gate_test.cpp game/LevelScene.cpp ../Game/tinyxml/tinyxml.cpp ../Game/tinyxml/tinyxmlerror.cpp ../Game/tinyxml/tinyxmlparser.cpp ../Game/tinyxml/tinystr.cpp -o $@

.PHONY: mixed-command-interpolation-gate-test
mixed-command-interpolation-gate-test: build/mixed_command_interpolation_gate_test
	../port/build/mixed_command_interpolation_gate_test
build/mixed_command_interpolation_gate_test: tests/mixed_command_interpolation_gate_test.cpp linux/MixedCommandInterpolationGate.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -UNDEBUG -Icompat tests/mixed_command_interpolation_gate_test.cpp -o $@

.PHONY: combat-material-identity-test
combat-material-identity-test: build/combat_material_identity_test
	cd .. && port/build/combat_material_identity_test
build/combat_material_identity_test: tests/combat_material_identity_test.cpp game/Combat.cpp game/Combat.h game/LevelScene.cpp game/ParticleSpray.h linux/RealSceneTopologyCapture.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -UNDEBUG -Icompat tests/combat_material_identity_test.cpp game/Combat.cpp game/LevelScene.cpp ../Game/tinyxml/tinyxml.cpp ../Game/tinyxml/tinyxmlerror.cpp ../Game/tinyxml/tinyxmlparser.cpp ../Game/tinyxml/tinystr.cpp -o $@

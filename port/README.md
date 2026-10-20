# Linux renderer foundation

This branch starts the native Linux port. It now runs a limited original-level gameplay prototype, but it is **not** a complete playable port. The current Visual Studio game uses Direct3D 10/DXUT directly in 114 source files, plus Windows-specific audio, input, Steam and window code. This milestone makes a renderer boundary concrete and verifies a Linux backend against art from the repository.

## Build and run on Linux

Install a C++17 compiler, `make`, `pkg-config`, SDL2 development headers and libpng development headers. On Debian/Ubuntu:

```sh
sudo apt install build-essential pkg-config libsdl2-dev libpng-dev
make -C port
./port/build/deathleak-linux
```

Run from the repository root so `Game/Media` resolves. Arrow keys or WASD move the silhouette; Escape closes the window. For an automated render check on a machine without a display:

```sh
make -C port smoke
```

This uses SDL's dummy video driver, renders two frames, and writes `port/build/smoke.bmp` (ignored by Git).

## Rendering seam

`render/Renderer.h` is the platform-neutral contract. Game-side code submits source/destination rectangles, tint, rotation and flips using `TextureId`; it never sees SDL, Direct3D or Metal objects. `linux/SDLRenderer.cpp` owns the SDL2 window, textures, input polling and presentation. `CreateRenderer()` is the backend selection point. A macOS backend can implement this contract (or the same SDL backend can be compiled there initially); no macOS-specific code is required in callers.

## Next integration milestones

1. Move game sprites/UI onto the backend-neutral draw commands, including render ordering and camera transforms. Direct3D handles currently leak through `Sprite`, `DrawableObject`, `TextureManager`, `Effect*`, and `Game`.
2. Represent each shader/effect by portable parameters and implement visual parity in the Linux backend. Current post-processing, bump/reflection/wobble and particle paths are Direct3D-specific.
3. Separate OS input/window, audio and Steam integrations. Port save/config paths and remove Windows-only precompiled-header dependencies.
4. Build and playtest the actual levels on Linux; compare visual output and input behavior with Windows.

The renderer API is deliberately small for this slice. Extend it alongside actual game migration; do not emulate Direct3D types in it.

## Original-level rendering milestone (in progress)

The executable can now parse original XML level files through the repository's TinyXML source, resolve mixed Windows/POSIX asset paths, cache PNG textures within a level, depth-sort objects, and render a static scene using the backend-neutral renderer. For example:

```sh
./port/build/deathleak-linux --level-preview Game/XmlFiles/levels/forest_exploration_1.xml
```

WASD/arrow keys pan the preview camera. This **does not run game simulation**. Animated objects use only the first animation frame. This preview mode does not simulate collision, movement, transitions, HUD or audio; `--play` contains partial implementations described below. The current preview also has known vertical-flip and repeat/parallax visual differences from the original. Do not package it as a playable game.

To build without modifying host development packages, use the reproducible Linux build image:

```sh
docker build -t deathleak-linux-build:bookworm -f port/Dockerfile port
docker run --rm -u "$(id -u):$(id -g)" -v "$PWD:/src" -w /src deathleak-linux-build:bookworm make -C port
docker run --rm -u "$(id -u):$(id -g)" -v "$PWD:/src" -w /src -e SDL_VIDEODRIVER=dummy deathleak-linux-build:bookworm \
  port/build/deathleak-linux --level-preview Game/XmlFiles/levels/forest_exploration_1.xml \
  --frames 2 --capture port/build/forest-preview.bmp
```

## Experimental gameplay slice

`--play Game/XmlFiles/levels/forest_exploration_1.xml` now enables a prototype player simulation: the player starts at the original XML position, moves with A/D or left/right, jumps with W/space/up, collides with `SolidLineStrip` segments, follows the camera, and enters `LevelTrigger` destinations. SDL game-controller left stick/D-pad and A button are also mapped. Escape or controller Start exits. The original level assets still render through `Renderer.h`.

This is **not yet a complete or release-ready Death Leak port**. The current encounter slice parses authored `NPCTrigger` and `WaterBlock` objects, spawns simplified ninja/ghost/skeleton opponents, runs knife and enemy projectiles, applies damage, and models focus depletion before drowning. It only approximates original Character/Player physics and does **not** reproduce full collision/AI and attack patterns, melee, most NPC interactions, complete UI/menus, audio, complete saves, shaders, particles, or animation; level entry placement and door semantics are incomplete. Keyboard play is an added Linux convenience (the Windows release build's keyboard gameplay path is disabled). The default executable screen remains a renderer demo, not the game's original menu.

Headless integration smoke:

```sh
docker run --rm -u "$(id -u):$(id -g)" -v "$PWD:/src" -w /src -e SDL_VIDEODRIVER=dummy deathleak-linux-build:bookworm \
  port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml --frames 120 --capture port/build/gameplay.bmp
```

The dummy SDL software renderer is slow on large transparent source art; on this development host, 120 frames took 73.5 s (~1.6 fps), including a frame capture. That is not an acceptable performance result. A GPU-backed window needs a separate real-display benchmark, and texture/culling/render-path profiling remains necessary.

## Deterministic integration and limited progress persistence

For a fast headless **simulation** check (no rendering), run from the repository root:

```sh
SDL_VIDEODRIVER=dummy port/build/deathleak-linux \
  --play Game/XmlFiles/levels/forest_exploration_1.xml \
  --script L:340 --fixed-dt 0.0166667 --simulate-only
```

This scripted left movement enters `grass_exploration_3.xml` through an authored trigger and places the player at that level's matching `LevelEntry`. `--script` supports `L`, `R`, `J` (jump), `Q` (left+jump), `E` (right+jump), `F` (fire), `B` (right+fire), `N` (left+fire), `T` (restart after death), and `.` (idle), each followed by `:frame-count`, comma-separated. This harness validates simulation and transitions, **not** rendered frame rate or human playability.

Experimental `--save-file <path>` writes the current level, position, and collected currency-orb IDs when the process exits. `--load-save <path>` resumes these values and updates the same file on exit. These are a limited Linux-port save format, not the original game's complete save/upgrade system. Currency orbs currently use the original 100-world-unit pickup radius and disappear on collection, but do not yet implement tracking animation, audio, upgrades, or the original UI.

## Encounter integration and renderer profiling

Build with the Docker image above, then run `port/tests/integration.sh` in that image from the repository root. The test drives an authored level transition, the first `NPCTrigger` in `first_ninja_arena.xml`, combat damage and save/resume, and focus-exhausted drowning in the authored `test.xml` water region. `--start-encounter N`, `--start-hazard N`, and `--exhaust-focus` are deterministic integration harness controls, not normal gameplay shortcuts. In interactive `--play`, Down/S fires the knife; left/right and jump continue to work.

The Linux-only save format is now `DL2` (level, position, health, focus, collected orb IDs); `DL1` remains readable. It is not the original Windows save format, and encounter state is not persisted.

For a measured headless render run, use `--profile` and `DEATHLEAK_RENDER_PROFILE=1`. On this Docker/dummy-video host, a five-frame forest sample reported 226 SDL draw calls, ~0.00008 s in `SDL_RenderCopy*`, and ~3.005 s in `SDL_RenderPresent`; update time was ~0.00002 s. This isolates the slow path to deferred software rasterization/presentation. It **does not** measure GPU performance, and there is no demonstrated optimization yet.

## Player motion, death, and partial HUD continuation

`PlayerMotion` now uses original player collision dimensions and source-scaled movement/jump/gravity/friction values at a 60 Hz target. Static `SolidLineStrip` slopes support landing and ground following; near-vertical segments block horizontal movement. This remains an approximation: moving solids, platform drop-through, wall-jump, water movement, other collision types, and per-level upgrade/state behavior are not implemented.

At zero health the original `you_died.png` is shown. Press R to reload `grass_exploration_3.xml`, the destination hardcoded in the original death-screen UI action. The HUD currently shows original-art health and focus meters plus an orb icon; it does not yet show orb count, text, or menus. The original XP meter is repurposed as focus, as in `UIGameHudScreen`.

The combat slice now enforces the original five-knife burst, 0.5-second burst pause, 0.25-second knife lifetime, water firing block, and five-second focus recharge lockout after depletion. Last facing direction persists while standing still. The separate `audio::SoundPlayer` seam has an SDL2 backend for knife-throw variants and orb-collection effects. Audio device failure is nonfatal; music, positional ambience, and audio settings remain unported.

Run `sh port/tests/integration.sh` for deterministic transition, three-level forest→grass→arena traversal, burst/water firing, focus cooldown, encounter, save/resume, water, jump/landing, slope/wall, and death/restart checks. The script includes a synthetic geometry fixture at `port/tests/fixtures/slope_wall.xml`; passing it does not imply all original collision cases work. The three-level route reaches the arena but does not complete it; continuing left without a jump falls from a platform.

The latest five-frame dummy-video software run spent 3.210 s in present. Offscreen SDL OpenGL took 0.010 s for five frames, but the sandbox cannot open the DRM render devices and Vulkan sees only llvmpipe, so this is **not GPU-backed gameplay performance evidence**. A real display benchmark remains mandatory.

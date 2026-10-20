# Linux renderer foundation

This branch starts the native Linux port. It does **not** run Death Leak gameplay yet. The current Visual Studio game uses Direct3D 10/DXUT directly in 114 source files, plus Windows-specific audio, input, Steam and window code. This milestone makes a renderer boundary concrete and verifies a Linux backend against art from the repository.

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

WASD/arrow keys pan the preview camera. This **does not run game simulation**. Animated objects use only the first animation frame. Collision, player movement, transitions, UI, audio, shaders, repeated textures and parallax behavior remain unported. The current preview also has known vertical-flip and repeat/parallax visual differences from the original. Do not package it as a playable game.

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

This is **not yet a complete or release-ready Death Leak port**. It does not reproduce original Character/Player physics, combat, enemies, pickups, NPCs, UI/menus, audio, saves, shaders, particles, or animation; level entry placement and door semantics are incomplete. Keyboard play is an added Linux convenience (the Windows release build's keyboard gameplay path is disabled). The default executable screen remains a renderer demo, not the game's original menu.

Headless integration smoke:

```sh
docker run --rm -u "$(id -u):$(id -g)" -v "$PWD:/src" -w /src -e SDL_VIDEODRIVER=dummy deathleak-linux-build:bookworm \
  port/build/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml --frames 120 --capture port/build/gameplay.bmp
```

The dummy SDL software renderer is slow on large transparent source art; on this development host, 120 frames took 73.5 s (~1.6 fps), including a frame capture. That is not an acceptable performance result. A GPU-backed window needs a separate real-display benchmark, and texture/culling/render-path profiling remains necessary.

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

This is **not yet a complete or release-ready Death Leak port**. The current encounter slice parses authored `NPCTrigger` and `WaterBlock` objects, spawns simplified ninja/ghost/skeleton opponents, runs knife and enemy projectiles, applies damage, and models focus depletion before drowning. It also supports a bounded player melee hit against NPCs. It only approximates original Character/Player physics and does **not** reproduce full collision/AI and attack patterns, projectile deflection, most NPC interactions, complete UI/menus, audio, complete saves, shaders, particles, or animation; key/door interactions and entry placement are partial. Keyboard play is an added Linux convenience (the Windows release build's keyboard gameplay path is disabled). The default executable screen remains a renderer demo, not the game's original menu.

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

This scripted left movement enters `grass_exploration_3.xml` through an authored trigger and places the player at that level's matching `LevelEntry`. `--script` supports `L`, `R`, `J` (jump), `Q` (left+jump), `E` (right+jump), `F` (fire), `B` (right+fire), `N` (left+fire), `M` (melee), `T` (restart after death), `I` (interact), and `.` (idle), each followed by `:frame-count`, comma-separated. This harness validates simulation and transitions, **not** rendered frame rate or human playability.

Experimental `--save-file <path>` writes the current level, position, and collected currency-orb IDs when the process exits. `--load-save <path>` resumes these values and updates the same file on exit. These are a limited Linux-port save format, not the original game's complete save/upgrade system. Currency orbs currently use the original 100-world-unit pickup radius and disappear on collection, but do not yet implement tracking animation, audio, upgrades, or the original UI.

## Encounter integration and renderer profiling

Build with the Docker image above, then run `port/tests/integration.sh` from the repository root. The test drives an authored level transition, the first `NPCTrigger` in `first_ninja_arena.xml`, combat damage and save/resume, and focus-exhausted drowning in the authored `test.xml` water region. `--start-encounter N`, `--start-hazard N`, and `--exhaust-focus` are deterministic integration harness controls, not normal gameplay shortcuts. In interactive `--play`, Down/S fires the knife, C/controller right shoulder starts a grounded melee attack for 4 focus, and E/controller X holds interaction for pickups and doors; left/right and jump continue to work. Melee hits have no dedicated animation or sound yet.

The Linux-only save format is now `DL4` (level, position, health, focus, collected orb IDs, owned keys, unlocked door IDs, and last-fired encounter timestamps); `DL1`, `DL2`, and `DL3` remain readable. It is not the original Windows save format. A trigger that fired less than 120 seconds ago is suppressed after level reload or save/resume; active enemy positions and health are not saved.

For a measured headless render run, use `--profile` and `DEATHLEAK_RENDER_PROFILE=1`. On this Docker/dummy-video host, a five-frame forest sample reported 226 SDL draw calls, ~0.00008 s in `SDL_RenderCopy*`, and ~3.005 s in `SDL_RenderPresent`; update time was ~0.00002 s. This isolates the slow path to deferred software rasterization/presentation. It **does not** measure GPU performance, and there is no demonstrated optimization yet.

## Player motion, death, and partial HUD continuation

`PlayerMotion` now uses original player collision dimensions and source-scaled movement/jump/gravity/friction values at a 60 Hz target. Static `SolidLineStrip` slopes support landing and ground following; near-vertical segments block horizontal movement. Authored `SolidMovingSprite` collision boxes are parsed for static side/top/underside contact, and pressing jump again at a wall applies the source `Character::WallJump` speeds and duration. This is an adaptation: the original method has no live caller, so the input mapping is not demonstrated Windows parity. Water now limits horizontal and vertical speed, reduces gravity, and maps held jump to swim-up; full water exits, down-swim, and WaterJump are incomplete. Moving solids, platform drop-through, other collision types, and per-level upgrade/state behavior remain incomplete.

At zero health the original `you_died.png` is shown. Press R to reload `grass_exploration_3.xml`, the destination hardcoded in the original death-screen UI action. The HUD currently shows original-art health and focus meters plus an orb icon; it does not yet show orb count, text, or menus. The original XP meter is repurposed as focus, as in `UIGameHudScreen`.

The combat slice now enforces the original five-knife burst, 0.5-second burst pause, 0.25-second knife lifetime, water firing block, and five-second focus recharge lockout after depletion. Last facing direction persists while standing still. The separate `audio::SoundPlayer` seam has an SDL2 backend for knife-throw variants and orb-collection effects. Audio device failure is nonfatal; music, positional ambience, and audio settings remain unported.

Run `sh port/tests/integration.sh` for deterministic transition, three-level forest→grass→arena traversal, burst/water firing, focus cooldown, encounter, save/resume, water, jump/landing, slope/wall, and death/restart checks. The script includes a synthetic geometry fixture at `port/tests/fixtures/slope_wall.xml`; passing it does not imply all original collision cases work. The three-level route reaches the arena but does not complete it; continuing left without a jump falls from a platform.

The latest five-frame dummy-video software run spent 3.210 s in present. Offscreen SDL OpenGL took 0.010 s for five frames, but the sandbox cannot open the DRM render devices and Vulkan sees only llvmpipe, so this is **not GPU-backed gameplay performance evidence**. A real display benchmark remains mandatory.

## Key and door progression slice

Original `KeyPickup`/`Door` XML is parsed. Press E (controller X) while overlapping a key or a grounded, warmed-up door. Keys persist across level changes and DL4 save/resume; a required key gates a door, and a matching `LevelEntry` must match both the source level and door ID. The door warmup (0.75 s), interaction hold (0.05 s), unlocking pause (1 s), and follow-up hold (0.4 s) come from the Windows source. `port/tests/fixtures/key_door.xml` proves locked/released routes, key persistence, and door-ID-specific entry selection. This does not yet implement interaction prompts, key description modals, door SFX/animation, or original save-file compatibility.

## Earned deflection to the keyed village branch and finale gate

`port/tests/routes/earned_deflection_purchase.script` plus the ordinary-control suffixes in `port/tests/integration.sh` form one uninterrupted fixed-step forest-spawn route. It earns and spends the source-authored 375 orbs on the actual deflection scroll, deflects a shrine ninja projectile, collects the agriculture `dojo` key, enters `dojo_front` and `dojo`, collects the `village_hut` key, and opens the central village-hut door. The final save has 388 distinct authored orb IDs, a spendable balance of 13, both keys, both unlocked doors and `kDeflection`. `port/tests/routes/earned_dojo_hut_suffix.script` contains the second continuation. This is a deterministic headless simulation route, not a real-display playtest or a full-game clear.

The original finale is not a level-trigger edge: `Boat::EnterBoat` switches the cliff boat to `sea_2.xml` only when `SaveManager::HasRepairTools()` is true. The corresponding authored key is `has_repair_tools` in `catacombs_entrance_1.xml`. The Linux slice now parses `Boat`, enforces its 0.75 s grounded warmup, shows a broken-boat diagnostic without that key, and enters sea 2 with the persisted key. `port/tests/fixtures/boat_gate.xml` tests this bounded behavior. The source's text modal, effects, final-scene HUD, sea-2 timed title/end-story callback and save reset are not implemented. No uninterrupted route from the earned-deflection run to the repair-tools key, cliff boat, or actual ending has been demonstrated. The port remains partial.

## Uninterrupted earned repair-tools and sea-2 ending route

`port/tests/routes/earned_full_ending.script` is a fixed-1/60-second, single-process route from the original forest-1 spawn. It collects 384 distinct authored orb IDs before buying the 375-orb deflection upgrade, takes the `granary` key from the grass-1 lower dead end, completes the dojo/village-hut key chain, unlocks the granary-gated barn door, takes `has_repair_tools` in the catacombs entrance, returns through the authored level links, and boards the cliff boat. The pre-boat snapshot has 406 distinct collected IDs, all four keys, one earned deflection, health 1, and spendable balance 31. It contains only normal movement, jump, fire, melee, interact, purchase, and idle inputs; it uses no load-save, warp, injected item/currency, or debug hit. `port/tests/routes/earned_granary_purchase.script` and `earned_repair_tools.script` preserve inspectable intermediate prefixes.

The sea-2 player state freezes normal controls/combat, reveals the red `DEATH LEAK` title after 15 seconds, and fires `EndStory` after 35 seconds, matching the source thresholds. The Linux run emits a menu-return event and clears its selected progression save. The `final_scene_hud` film matte/title are approximated with the renderer's available textures and bitmap font; source film animation, Impact typography, rain, sound cue, completion-count/language retention, and Steam achievement are not yet ported. The headless route and regression are a **deterministic progression clear of this partial port**, not source-equivalent full-game acceptance. No real display is present and `/dev/dri` render access is unavailable here, so human-display/GPU performance acceptance remains unproven.

Reproduce after building: `sh port/tests/integration.sh`. The full-ending test inspects a pre-boat DL5 save for 406 unique IDs and four keys, then independently reruns the entire script through the title, completion callback, and save wipe.

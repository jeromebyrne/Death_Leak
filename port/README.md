# Linux renderer foundation

This branch starts the native Linux port. It now runs a limited original-level gameplay prototype, but it is **not** a complete playable port. The current Visual Studio game uses Direct3D 10/DXUT directly in 114 source files, plus Windows-specific audio, input, Steam and window code. This milestone makes a renderer boundary concrete and verifies a Linux backend against art from the repository.

## Build and run on Linux

Install a C++17 compiler, `make`, `pkg-config`, SDL2 development headers and libpng development headers. On Debian/Ubuntu:

```sh
sudo apt install build-essential pkg-config libsdl2-dev libpng-dev
make -C port
./port/build/deathleak-linux
```

Run from the repository root so `Game/Media` resolves. Fresh launch opens the Linux main-menu slice. Up/Down changes focus, E/controller X activates Play, Options, or Quit; Escape closes the menu. Mouse hover and left-button press/release use the authored menu/options widget rectangles; releasing over a different widget cancels the action. Music and SFX checkboxes use their original on/off/focus art and persist to a separate settings XML. The authored Play action starts `grass_exploration_3.xml`. This remains a partial port. For an automated render check on a machine without a display:

```sh
make -C port smoke
```

This uses SDL's dummy video driver, renders two frames, and writes `port/build/smoke.bmp` (ignored by Git).

## Linux input controls

Keyboard/mouse and an attached SDL gamepad coexist; attaching a pad does not disable keyboard or mouse. Unplugging/hot-plugging is supported. Gamepad: left stick/D-pad move, A jump/accept/restart, right stick aim/fire, X interact, Y purchase, left shoulder roll, right shoulder melee/dash, left trigger Focus, left-stick click bomb, Start pause/exit, B close dialogs/back in menus.

Keyboard/mouse: A/D or Left/Right move; W/Up/Space jump or swim up; S/Down crouch; move the mouse and hold left click to aim/fire. F fires in the facing direction. E interacts, C melees/dashes, Left Shift rolls, Ctrl holds Focus, B throws a bomb, Y purchases upgrades, R restarts after death, and Escape exits. Menu focus uses Up/Down or mouse hover; Enter/E/Space activates, and Escape/Backspace goes back. Clicking a menu item activates it on release over the same item.

Focus, roll, dash and bombs require their authored purchased unlocks; the bomb scroll additionally requires a completed game. The keyboard/mouse mapping is a Linux-port accessibility adaptation. Display-attached human input acceptance remains separate from scripted and offscreen checks.

## All-level audit and source parity repairs (2026-10-29)

See [the full level-by-level audit](ALL_LEVEL_REFERENCE_AUDIT_20261029.md) for51 definitions,33 positively identified reference-video scenes, exact timestamps, shared-mechanic repairs, all51 strict hardware-offscreen scene captures and the remaining attended physical-display gate. All8 scrolls, readable original upgrade descriptions/authored modal bounds, orb rewards, authored water/actor/projectile boundaries, moving platforms, breakables, ambient actors and final raft dynamics are now implemented through the existing Linux runtime. A fresh ordinary-input replay earns396 real original pickups and purchases375-orb Deflection (balance21), without loaded/synthetic state; this is not a whole-game ending certification. Historical sections below describe earlier milestones and do not supersede this audit or prove a current earned full-game ending. Original Windows sources/build projects remain unchanged; no master merge is included.

Source-backed diagnostics: `sh port/tests/all_level_parity.sh`; synthetic purchase fixtures: `python3 port/tests/all_level_progression_test.py` (requires a built binary and explicitly selected diagnostic/display environment). Neither substitutes for physical GPU/input acceptance or earned progression.

## Desktop/remote-input repair (2026-10-29)

`./Play-DeathLeak.sh` uses isolated backed-up state and distinguishes active DL6
progress from completed MENU metadata. Escape exits; Ctrl+Q and Alt+F4 are
unconditional emergency exits, and the window close button is supported.
Original menu cursor art is rendered into the frame for remote capture. Remote
key/mouse press edges survive a press/release delivered within one update;
focus loss clears held state. Local hardware and actual remote transport remain
separate attended acceptance checks. See `PLAYTEST_REPAIR_20261029.md` for source
parity, reproduced render-target projection errors and exact verification scope.

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

WASD/arrow keys pan the preview camera. This **does not run game simulation**. Authored CurrencyOrb sprites in preview/gameplay cycle their `body/Still` frames at the XML frame rate; the preview player remains on a static first frame; gameplay now advances the authored player body sequences. Other animated objects still use a static first frame. This preview mode does not simulate collision, movement, transitions, HUD or audio; `--play` contains partial implementations described below. The current preview also has known vertical-flip and repeat/parallax visual differences from the original. Do not package it as a playable game.

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

This is **not yet a complete or release-ready Death Leak port**. The current encounter slice parses authored `NPCTrigger` and `WaterBlock` objects, spawns simplified ninja/ghost/skeleton opponents, runs knife and enemy projectiles, applies damage, and models focus depletion before drowning. It also supports a bounded player melee hit against NPCs. It only approximates original Character/Player physics and does **not** reproduce full collision/AI and attack patterns, projectile deflection, most NPC interactions, complete UI/menus, audio, complete saves, shaders, particles, or complete animation; key/door interactions and entry placement are partial. Keyboard play is an added Linux convenience (the Windows release build's keyboard gameplay path is disabled). The default executable now presents a bounded main-menu implementation using original menu art and button order; it is not a full reproduction of the Windows UI.

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

This scripted left movement enters `grass_exploration_3.xml` through an authored trigger and places the player at that level's matching `LevelEntry`. `--script` supports `L`, `R`, `J` (jump), `Q` (left+jump), `E` (right+jump), `F` (fire), `B` (right+fire), `N` (left+fire), `Y` (right+jump+fire), `Z` (left+jump+fire), `M` (melee), `T` (restart after death), `I` (interact), and `.` (idle), each followed by `:frame-count`, comma-separated. This harness validates simulation and transitions, **not** rendered frame rate or human playability.

For route triage, `--simulate-wall-clock` requires `--simulate-only --fixed-dt` and advances only the NPC encounter cooldown clock by gameplay dt. This test-only clock reproduces the village-central trigger's 120-second respawn and the current earned-route death at frame 12837 in seconds. Ordinary fast simulation still completes the ending because its actual elapsed wall time is below the cooldown. Neither simulated clock mode nor a scripted physical-display run substitutes for human/controller/speaker acceptance.

On this development machine, `port/tools/run_physical_xorg_route.sh probe` runs a two-frame strict Xorg/DRM check in the build image. `gameplay` runs 6,000 frames of active rendered gameplay across authored scenes and requires a complete frame profile; it does **not** require scripted survival or an ending. `prefix` (13,000 frames) and `full` (60,000 frames) remain optional earned-route diagnostics; their nonzero status on death means that scripted route ended, not that the Linux port failed. Logs are saved under the ignored `port/build/physical-xorg-route/` directory. The checks explicitly require `x11` and the physical `nouveau / NVE7` OpenGL device so offscreen llvmpipe cannot be mistaken for display-GPU evidence. They mount the host LightDM `:0` authority and expect the current host display and Docker image; they are machine-specific. Input remains scripted and audio uses SDL dummy; passing `gameplay` does not verify human play, a controller, or speakers.

`port/tools/run_physical_xorg_audio.sh` is a machine-specific real-ALSA smoke test on the physical Xorg GPU. It opens the host PCH playback device (`hw:1,0`) with SDL/ALSA, streams the authored forest ambience, and dispatches knife cues during active scripted gameplay. The default ALSA device does not open in this Docker environment, so the gate selects the card explicitly. It proves the non-dummy device/playback path initialized, **not** that the connected speakers were audible or sounded correct to a listener. Logs stay under ignored `port/build/physical-xorg-audio/`.

`port/tools/run_physical_xorg_save_load.sh` is a separate, machine-specific real-display smoke test: it saves after the authored forest→waterfall transition, exits, reloads on strict `nouveau / NVE7` OpenGL, and checks that ordinary scripted play advances with the saved health and orb count. Its logs and temporary save stay under ignored `port/build/physical-xorg-save-load/`. This checks the actual display/GPU save/load path but still uses scripted input and dummy audio; it is not controller, speaker, or human acceptance.

Experimental `--save-file <path>` writes the current level, position, and collected currency-orb IDs when the process exits. `--load-save <path>` resumes these values and updates the same file on exit. These are a limited Linux-port save format, not the original game's complete save/upgrade system. Currency orbs currently use the original 100-world-unit pickup radius, cycle the authored ten-frame 10 FPS idle visual, and disappear on collection, but do not yet implement authored hover/tracking motion, upgrades, or the original UI. Their collection sound is implemented separately below.

## Encounter integration and renderer profiling

Build with the Docker image above, then run `port/tests/integration.sh` from the repository root. The test drives an authored level transition, the first `NPCTrigger` in `first_ninja_arena.xml`, combat damage and save/resume, and focus-exhausted drowning in the authored `test.xml` water region. `--start-encounter N`, `--start-hazard N`, and `--exhaust-focus` are deterministic integration harness controls, not normal gameplay shortcuts. In interactive `--play`, F fires the knife independently while grounded, Down/S crouches on the ground and still fires while airborne; right-stick aim also fires. C/controller right shoulder starts a grounded melee attack for 4 focus, and E/controller X holds interaction for pickups and doors; left/right and jump continue to work. Melee now draws the four authored body phases, but has no dedicated sound yet; the source arm/skeleton composition is not ported.

The Linux-only save format is now `DL4` (level, position, health, focus, collected orb IDs, owned keys, unlocked door IDs, and last-fired encounter timestamps); `DL1`, `DL2`, and `DL3` remain readable. It is not the original Windows save format. A trigger that fired less than 120 seconds ago is suppressed after level reload or save/resume; active enemy positions and health are not saved.

For a measured headless render run, use `--profile` and `DEATHLEAK_RENDER_PROFILE=1`. On this Docker/dummy-video host, a five-frame forest sample reported 226 SDL draw calls, ~0.00008 s in `SDL_RenderCopy*`, and ~3.005 s in `SDL_RenderPresent`; update time was ~0.00002 s. This isolates the slow path to deferred software rasterization/presentation. It **does not** measure GPU performance, and there is no demonstrated optimization yet.

## Player motion, death, and partial HUD continuation

`PlayerMotion` now uses original player collision dimensions and source-scaled movement/jump/gravity/friction values at a 60 Hz target. Static `SolidLineStrip` slopes support landing and ground following; near-vertical segments block horizontal movement. Authored `SolidMovingSprite` collision boxes are parsed for static side/top/underside contact, and pressing jump again at a wall applies the source `Character::WallJump` speeds and duration. This is an adaptation: the original method has no live caller, so the input mapping is not demonstrated Windows parity. Water now limits horizontal and vertical speed, reduces gravity, and maps held jump to swim-up; full water exits, down-swim, and WaterJump are incomplete. Moving solids, platform drop-through, other collision types, and per-level upgrade/state behavior remain incomplete.

At zero health the original `you_died.png` is shown. Press R to reload `grass_exploration_3.xml`, the destination hardcoded in the original death-screen UI action. The HUD currently shows original-art health and focus meters plus an orb icon; it does not yet show orb count, text, or menus. The original XP meter is repurposed as focus, as in `UIGameHudScreen`.

The combat slice now enforces the original five-knife burst, 0.5-second burst pause, 0.25-second knife lifetime, water firing block, and five-second focus recharge lockout after depletion. Last facing direction persists while standing still. The separate `audio::SoundPlayer` seam has an SDL2 backend for knife-throw variants and orb-collection effects. Audio device failure is nonfatal. The Options SFX toggle now mutes these cues and persists separately from game progress; level music and the separate `pos_audio_props` positional system remain unported; authored `AudioObject` WAV ambience is covered below.

Run `sh port/tests/integration.sh` for deterministic transition, three-level forest→grass→arena traversal, burst/water firing, focus cooldown, encounter, save/resume, water, jump/landing, slope/wall, and death/restart checks. The script includes a synthetic geometry fixture at `port/tests/fixtures/slope_wall.xml`; passing it does not imply all original collision cases work. The three-level route reaches the arena but does not complete it; continuing left without a jump falls from a platform.

The latest five-frame dummy-video software run spent 3.210 s in present. Offscreen SDL OpenGL took 0.010 s for five frames, but the sandbox cannot open the DRM render devices and Vulkan sees only llvmpipe, so this is **not GPU-backed gameplay performance evidence**. A real display benchmark remains mandatory.

## Key and door progression slice

Original `KeyPickup`/`Door` XML is parsed. Press E (controller X) while overlapping a key or a grounded, warmed-up door. Keys persist across level changes and DL4 save/resume; a required key gates a door, and a matching `LevelEntry` must match both the source level and door ID. The door warmup (0.75 s), interaction hold (0.05 s), unlocking pause (1 s), and follow-up hold (0.4 s) come from the Windows source. `port/tests/fixtures/key_door.xml` proves locked/released routes, key persistence, and door-ID-specific entry selection. This does not yet implement interaction prompts, key description modals, door SFX/animation, or original save-file compatibility.

## Earned deflection to the keyed village branch and finale gate

`port/tests/routes/earned_deflection_purchase.script` plus the ordinary-control suffixes in `port/tests/integration.sh` form one uninterrupted fixed-step forest-spawn route. It earns and spends the source-authored 375 orbs on the actual deflection scroll, deflects a shrine ninja projectile, collects the agriculture `dojo` key, enters `dojo_front` and `dojo`, collects the `village_hut` key, and opens the central village-hut door. The final save has 388 distinct authored orb IDs, a spendable balance of 13, both keys, both unlocked doors and `kDeflection`. `port/tests/routes/earned_dojo_hut_suffix.script` contains the second continuation. This is a deterministic headless simulation route, not a real-display playtest or a full-game clear.

The original finale is not a level-trigger edge: `Boat::EnterBoat` switches the cliff boat to `sea_2.xml` only when `SaveManager::HasRepairTools()` is true. The corresponding authored key is `has_repair_tools` in `catacombs_entrance_1.xml`. The Linux slice now parses `Boat`, enforces its 0.75 s grounded warmup, shows a broken-boat diagnostic without that key, and enters sea 2 with the persisted key. `port/tests/fixtures/boat_gate.xml` tests this bounded behavior. The source's text modal, effects, final-scene HUD, sea-2 timed title/end-story callback and save reset are not implemented. No uninterrupted route from the earned-deflection run to the repair-tools key, cliff boat, or actual ending has been demonstrated. The port remains partial.

## Uninterrupted earned repair-tools and sea-2 ending route

`port/tests/routes/earned_full_ending.script` is a fixed-1/60-second, single-process route from the original forest-1 spawn. It collects 384 distinct authored orb IDs before buying the 375-orb deflection upgrade, takes the `granary` key from the grass-1 lower dead end, completes the dojo/village-hut key chain, unlocks the granary-gated barn door, takes `has_repair_tools` in the catacombs entrance, returns through the authored level links, and boards the cliff boat. The pre-boat snapshot has 406 distinct collected IDs, all four keys, one earned deflection, health 1, and spendable balance 31. It contains only normal movement, jump, fire, melee, interact, purchase, and idle inputs; it uses no load-save, warp, injected item/currency, or debug hit. `port/tests/routes/earned_granary_purchase.script` and `earned_repair_tools.script` preserve inspectable intermediate prefixes.

The sea-2 player state freezes normal controls/combat, reveals the red `DEATH LEAK` title after 15 seconds, and fires `EndStory` after 35 seconds, matching the source thresholds. The completion path now writes a metadata-only `DL6` save (`MENU`, completion count, optional language) instead of deleting the save entirely. A fresh run using `--save-file` retains that metadata while starting at the XML spawn; `--load-save` correctly refuses a completed/menu save. Active `DL5` saves remain readable and carry optional `C:`/`G:` metadata. `--language en-gb|pt-br|de` remains a CLI override; the Linux Options slice also selects these locales with keyboard focus, though it does not reproduce the original settings screen exactly. The regression proves a second completion increments the count and retains language, with no stale keys/progression. This is a Linux-only format, not original Windows `bargon.xml` compatibility.

The final scene now cycles the original four film-frame textures at the authored 11 FPS and quarter opacity and dispatches `boomy_intro.wav` when the title appears. The source's frame bars remain, but title lettering still uses the bitmap-font fallback rather than Impact; rain, Steam achievement, full menu navigation, and audible-speaker quality are not ported/accepted. An offscreen 1280×720 capture verifies the film changes between frames and the title appears. It is **not** a human-display/GPU performance test. The headless route and regression are a deterministic progression clear of this **partial** port, not source-equivalent full-game acceptance. No real display is present and `/dev/dri` render access is unavailable here.

Reproduce after building: `sh port/tests/integration.sh`. The full-ending test inspects a pre-boat DL5 save for 406 unique IDs and four keys, then independently reruns the entire script through the title, completion callback, and metadata-only progression wipe.

## Current main-menu loop (2026-10-19)

Fresh launch (no level argument) shows source `mainmenu.xml` background, title, frame and Play/Options/Quit button art. Up/Down and E/controller X navigate; Escape quits or backs out of Options. Play starts the source-authored `grass_exploration_3.xml`. Options now has source-style Music and SFX on/off choices, plus the Linux locale submenu (`en-gb`, `pt-br`, `de`); original `language_select_settings.xml` is disabled in `UI.xml`. SFX mute immediately suppresses/clears the implemented SDL cues; Music persists its choice but currently has no authored level score to control (the referenced `japanese2.mp3` is absent). Settings use the original `<settings><audio music_enabled="0|1" sfx_enabled="0|1"/></settings>` shape in a **separate** Linux `deathleak-settings.xml` (override with `--settings-file PATH`) and are not wiped on ending/New Game. The original `Game/XmlFiles/settings.xml` and other Windows assets are untouched. `--menu-loop` opts explicit `--play` runs into in-process menu return after sea-2 `EndStory`; this is useful for testing the ending without changing older direct-play harness behavior. After completion the port writes the metadata-only `DL6 MENU` save, clears progression, displays the menu, and permits New Game in the same process. A New Game starts without stale keys/orbs while retaining completion count/language. `--script` adds `S` for menu activate, `J`/`N` for menu up/down, and `X` for menu back/quit; existing gameplay script actions are unchanged. The game still lacks source Windows `bargon.xml` save-format compatibility, mouse-based menu widgets, exact original options art/layout and menu animation. Headless integration and offscreen rendering do not establish real-display or GPU acceptance.

## Explicit Windows save bridge (permanent-data migration)

`port/tools/save_bridge.py` converts the supported **permanent** fields of the original encrypted `bargon.xml` wrapper. It uses the source `babson` Vigenère/base64 codec and typed `<save_data>` fields. It never edits the input or overwrites an existing output.

```sh
python3 port/tools/save_bridge.py import path/to/bargon.xml path/to/new.dl5 --level Game/XmlFiles/levels/grass_exploration_3.xml --fresh-session
python3 port/tools/save_bridge.py export path/to/save.dl5 path/to/new-bargon.xml --discard-runtime
port/build/deathleak-linux --load-save path/to/new.dl5
```

Import deliberately starts at the selected authored Player spawn with default health/focus and zero orbs. Windows `SaveManager::WriteSaveFile` saves no temporary currency/orb/NPC state or position, so it cannot resume an exact Windows session. Export deliberately drops Linux position, health/focus, balance, collected-orb IDs, and trigger timestamps; the required flag acknowledges that loss. Completion count, language, four authored keys, three keyed doors, and feature ID 4 (`kDeflection`) have Linux mappings. The shipped save's other **known permanent** values (feature IDs 1/2/5/6/7, nine paper-pickup IDs, max health, two smashable flags, two health-devil flags and reward count) are validated and carried in a `W1:` typed XML/base64 migration record inside DL5, including through normal Linux resaves. Export restores those fields; an absent completion count is normalized to zero. The bridge also emits active `H:` max-health and `U:kSpeedIncrease` lines for source field `player_max_health` and feature ID 5; the Linux health cap/HUD and movement use them, while `W1:` retains the original typed fields for exact export and detects conflicts. The imported current health still starts at 25, matching a fresh Linux runtime, but its cap is 65 for the shipped save. Imported feature IDs 6/7 also become active `U:kProjectileDamageIncrease` and `U:kProjectileSpeedIncrease` flags: knives deal 4 rather than 1.5 damage, and the speed unlock shortens reload, removes the five-shot burst pause, and reduces player-projectile gravity. Export rejects conflicts between these active flags and preserved `W1:` fields. Imported IDs 1/2 now activate downward dash (airborne right shoulder/C after 0.25 s, 50 focus) and roll (grounded left shoulder/Left Shift, 2 focus, with a 0.25 s landing buffer); the headless script uses M and O respectively. The motion uses existing swept collision and resets on landing/level change. Roll blocks knife fire/melee. Its 0.5-second animation-finish stand-in and the port’s broader collision/animation gaps remain approximations. Authored HealthDevils now use grounded held interaction and level-keyed `V:` reward flags plus `J:` count; the reward raises max health by 20 without healing current health. The bridge activates the shipped two flags/count and can export a newly earned third cliff reward with its increased `H:` cap. Health-devil interaction now plays the authored first/third voiceover where audio is available; the reward flag/count and a separate rendered, moving pickup appear only after live playback completes (or after authored PCM duration in deterministic headless simulation). Holding interaction for 0.05 s while overlapping that pickup raises the cap by 20; merely touching it does not, and current health stays unchanged. Collection plays the original drink_health_upgrade.wav cue. The pickup follows the source's delayed gravity, faster near-apex descent, vertical speed limit and horizontal resistance, and passes through ground segments and solid boxes as the original pickup collision callbacks do. The original siren, particle/shake, exact water/update-zone timing and full interaction state, and achievement remain unported. Interactable contention now uses authored depth/XML order with spawned pickups appended and retains a held target while overlapping; this is a bounded arbitration model, not byte-identical Windows update timing. As in the original, a spawned but uncollected pickup is transient on level exit/reload, while its already-earned reward flag/count prevent duplication. Shipped smashable flags now activate `Z:` level records: an upgraded downward dash can break an authored stone and its broken state uses the `Damage4` frame and no collision on reload; the bridge exports newly broken flags. This bounded collision path is enabled only with dash ownership to preserve the established non-upgraded route, whereas the original stone is always solid. Random orb reward, debris, SFX, shake, vibration and achievement remain unported. The Windows `EndStory` wipe clears all progression including `W1:`; a `DL6 MENU` export contains metadata only. Unknown permanent fields, feature IDs, paper IDs, or Linux permanent lines fail closed. This is data migration for the shipped file, **not** full save/gameplay compatibility. The bridge fixture is included in `port/tests/integration.sh`.

### Authored ambient WAV slice (partial)

The Linux SDL audio boundary now loads the shipped WAV `AudioObject` sources from each
level, loops those marked `repeat`, and applies their camera-center fade rectangles.
On level transitions it stops the previous level's ambient voices and starts the new
ones; the persisted **SFX** option mutes them, consistent with Windows
`AudioObject::Update` calling `PlaySoundEffect`. Audio initialization failure remains
nonfatal. Dummy SDL integration proves forest's two WAV objects swap to the one
in grass exploration 3; it does not establish speaker quality or true stereo pan.
MP3 ambient objects are skipped. The shipped authored music levels reference
`music\\japanese2.mp3`, which is absent from `Game/Media/Audio/music`; only
`japanese1.wav` exists there, and it is a player-triggered sound effect. The
persisted **Music** option therefore still has no level score to control. A next
slice requires the licensed original `japanese2.mp3`, then a streaming decoder,
`MusicManager`'s per-level initial-delay/repeat-gap scheduling, clean stop on
transition/menu/EndStory, and live Music mute/unmute behavior. Current WAV ambient
loading decodes whole files into memory and should become streaming for release.
The SDL renderer currently requests acceleration first but permits a software fallback;
software rendering is **not acceptable for release**. Dummy/headless checks are functional
only. Real-display testing must prove an accelerated backend and GPU behavior.

## Real-display GPU smoke (2026-10-20)

A host Xorg `:0` run via the build container selected SDL's strict accelerated OpenGL renderer (`accelerated=1 software=0`). `glxinfo -B` inside the same display/device configuration identified direct-rendered **nouveau NVE7**, not llvmpipe. A 600-frame active forest gameplay run with normal right/left input and five earned orbs completed in 10.0383 s (~59.77 FPS with vsync), with 0.146579 s in game update and 9.87605 s in draw/present. This is the first measured hardware-display sample; it does not verify full-game visual/control/audio parity or worst-case GPU performance. The port remains partial.

## Player body animation slice (2026-10-20)

Gameplay loads the original `player_anim_default.xml` body frames for Still, Running,
JumpingUp, JumpingDown, SlidingDown, Roll, and all four Melee phases. The selected
pose follows the native motion/combat states, and the running frame rate follows
`Character::UpdateAnimations`' speed multiplier and authored min/max rate. Idle
and running loop; one-shot sequences hold their last frame until state changes.
SDL drawing corrects the player's authored vertical-flip flag for its top-down
texture coordinates; facing follows the last movement input. This is a bounded
visual port: separate arm/skeleton animation, source landing/crouch/swim/intro
sequences, exact animation-controlled combat timing, and other character/NPC
animation are still missing. Physical X11/OpenGL capture and deterministic
integration prove execution, not human visual or full-game acceptance.

## Physical-GPU frame profiling

`--profile` now reports aggregate update and draw/present time plus frame-time
p50/p95/p99, maximum, and counts above 25 and 33.333 ms. Frame-time samples
exclude the first 120 frames to avoid startup warmup, but include gameplay
updates and vsynced presentation; they are **not GPU-only timings**. Use the
strict renderer on a physical display and confirm its reported OpenGL vendor
and renderer. The test-only software override and headless runs do not satisfy
GPU acceptance. The 2026-10-20 multiscene measurements and limitations are
recorded in `CHECKPOINT_STATUS.md`.

Profiled physical-display runs now print `Profile slow_frame` for post-warmup
frames above 33.333 ms, including the frame index, active level, update time,
and draw/present time. Capture frames include image-write overhead and should
not be used to judge ordinary frame pacing. The first 1,500 frames of the
unmodified earned ending route on physical Xorg/`nouveau NVE7` revealed
transition/update stalls at frames 398 (131.5 ms update) and 1058 (38.4 ms
update); this instrumentation does not eliminate them. See
`CHECKPOINT_STATUS.md` for the measured route and release limitations.

### Scene-entry texture timing

Set `DEATHLEAK_TRACE_SCENE_LOAD=1` to time level parsing and object loading, and
`DEATHLEAK_TRACE_TEXTURE_LOAD=1` to log each first-use PNG's header read,
RGBA decode, and SDL texture creation/upload phases in milliseconds. Texture
tracing is diagnostic and does not alter rendering or cache behavior. Use a
physical Xorg/accelerated-OpenGL run for GPU timing; offscreen/software timings
are not representative of hardware upload cost. The first waterfall-top entry
on 2026-10-21 measured approximately 32 ms decode / 11 ms upload for
`underwater_cave/cave_background.png`, and 13 ms / 2 ms for `waterfall_4.png`
on direct nouveau/NVE7. These are single-run measurements, not performance
budgets or a fixed transition-stall mitigation.

## Current Linux earned ending and bounded fidelity continuation — 2026-10-29

See [Linux completion checkpoint](LINUX_EARNED_COMPLETION_CHECKPOINT_20261029.md).
`make -C port current-earned-ending-test` replays a fresh ordinary-input all4-key
ending, legitimate375-orb Deflection purchase and natural deflection, validating
source pickup IDs, separate runtime rewards, survival and DL6/MENU/C:1 reset.
It explicitly uses a test encounter clock; no loaded/injected progression.
`bounded-ai-test`, `bomb-strip-contact-test`, `bomb-positional-audio-test` and
`ambient-damage-bridge-test` cover the concrete source repair boundaries.
Historical20261027/older integration itineraries and exact combat/health counts
are not current release acceptance; use the current earned gate and named
source fixtures. Physical display/input/audio/rumble and live pacing remain
attended gates. No original Windows source or assets changed, no master merge.

# Death Leak native Linux port checkpoint — 2026-10-19

**Status: partial; not a playable, performant complete game.** Branch `feature/linux-renderer-foundation`. The original Windows sources and assets remain unchanged. The backend-neutral `render::Renderer` boundary remains intact.

## Verified slices

- Original XML levels render through SDL and authored `LevelTrigger` transitions work. Deterministic `--script`, `--fixed-dt`, and `--simulate-only` controls support repeatable scenarios. Original `CurrencyOrb` pickup and limited DL2 save/resume preserve level, position, collected orbs, health, and focus.
- Original `NPCTrigger` objects spawn simplified ninja/ghost/skeleton encounters. Knife shots, enemy shots/damage, water focus consumption and drowning, and authored damage regions are connected; AI/projectile behavior is still a prototype rather than source-equivalent.
- This continuation replaces the generic player movement block with `PlayerMotion`. It uses the source `Player` collision dimensions (128.959793×214.898743), 8 world units/tick max horizontal speed, 21-unit jump, 0.75 gravity amount, 0.55 vertical resistance, 0.80 idle friction, and 0.3 s edge-jump grace, scaled from the original 60 Hz target update. Landing follows authored sloped `SolidLineStrip` segments, and vertical segments stop horizontal penetration. One-way lines do not block ascent from below. This is an approximation, not a full port of `MovingSprite`/`CollisionManager`.
- Zero health now freezes active gameplay and shows original `you_died.png`. R (or scripted `T`) reloads `grass_exploration_3.xml`, matching the level hardcoded in original `XmlFiles/UI/you_died.xml`; this is a keyboard adaptation of its button action. A minimal renderer-neutral health bar and orb icon use assets and coordinates from `game_hud.xml`, but XP/count/focus text and the rest of the HUD are absent.
- This continuation adds the original five-knife burst limit, 0.5 s burst pause, 0.25 s knife lifetime, firing prohibition in water, and five-second focus recharge lockout after depletion. Idle firing now retains last facing direction. The renderer-neutral HUD also displays the original `xp_meter_bar_middle.png` as a focus meter, matching `UIGameHudScreen`'s repurposing of the XP meter. These are source-grounded rules in the existing simplified combat model, not full combat parity.
- A separate backend-neutral `audio::SoundPlayer` boundary now has an SDL2 implementation for the original three `knifeThrow*.wav` variants and an orb pickup WAV. It mixes overlapping effects and fails to silent play if no audio device is present. This is a limited SFX slice; music, positional/ambient audio, settings, and actual-speaker listening remain open.

## Reproduction and evidence

- Build: `docker run --rm -v "$PWD":/src -w /src deathleak-linux-build:bookworm make -C port -j2` — passed (only legacy TinyXML warnings).
- `sh port/tests/integration.sh` — passed. In addition to prior checks it now drives a 1300-frame authored forest→grass→first ninja arena route, collecting 26 orbs and reaching the arena grounded; verifies five shots in 60 frames, no shooting while submerged, and focus lockout/recharge after leaving water.
- Docker build passed with only existing TinyXML warnings. A two-frame SDL dummy-video/dummy-audio run loaded the four WAV assets, initialized the audio device without an error, fired a knife, and captured `port/build/audio-hud-smoke.bmp` (ignored artifact). This proves device initialization and cue dispatch, **not audible output quality**.
- A 1280×720 offscreen OpenGL capture with HUD was generated at `port/build/hud-preview.bmp` (ignored build artifact); this is not a human playtest.
- Five forest frames with `SDL_VIDEODRIVER=dummy`: `SDL renderer=software`, elapsed 3.21048 s, 226 copies 0.0000877 s, present 3.21026 s. With `SDL_VIDEODRIVER=offscreen SDL_RENDER_DRIVER=opengl`: `SDL renderer=opengl`, elapsed 0.01001 s, copies 0.00984 s, present 0.000026 s. The latter is an offscreen Mesa fallback, **not a GPU/display benchmark**. Vulkan enumerates llvmpipe only; this sandbox has no DISPLAY/WAYLAND_DISPLAY and cannot open `/dev/dri/renderD*` (permission denied). Therefore no real GPU frame-rate claim or release performance acceptance is possible here.

## Remaining acceptance work

- Port complete Player/Character state, collision manager (platforms, moving solids, drop-through, wall-jump, water movement, knockback, fall/death), and per-level physics/upgrades; current motion approximates one collision box and static lines.
- Replace simplified combat/NPC logic with source-equivalent archetypes, projectiles, melee, deflection, and encounter persistence. Finish hazards, doors, interactions, pickups, animations, effects, menus, HUD count/text, music/ambient audio, save/settings.
- Script representative full-level routes with jumps/combat and conduct a real-display human playtest. The new route enters the arena but is **not a completion route**: continuing left after about frame 1500 falls from the lower arena platform; route controls still lack combined move+jump. Profile and optimize on a GPU-capable display context; verify a sustained frame-rate target. The default executable remains a renderer demo, and no end-to-end game acceptance is claimed.

The parent session handles user-facing completion/blocker notification. Continue from this checkpoint; do not report the port complete.

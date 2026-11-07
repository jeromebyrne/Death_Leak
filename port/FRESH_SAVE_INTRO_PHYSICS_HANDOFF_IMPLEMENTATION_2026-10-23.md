# Fresh-save intro physics handoff implementation — 2026-10-23

## Verdict and scope

**Bounded source patch implemented; focused CPU gate PASS.** The fresh-save `!hasPulledSword` branch now performs one `motion.Update(scene, dt, 0, false)` and immediately copies `motion.X/Y` to `worldX/Y` before checking body completion, threshold burst/flight spawn, and intro camera follow (`port/linux/main.cpp:640-679`). No ordinary movement, jump, dash, fire, interaction, or target-input branch was opened during the intro. Loaded/already-pulled gameplay remains on its existing path. Only `port/linux/main.cpp`, new private `port/tests/fresh_save_intro_physics_cpu_test.cpp`, and this report were changed in this scope; no shared build, GPU :0, existing intro animation fixture, Combat, LevelScene, status files, commit, or push were touched.

## Source and ordering evidence

- Windows `Player::Update` calls `Character::Update(delta)` before testing `!HasPulledSwordFromStomach`, updating intro, and returning (`Game/src/GameObjects/Player.cpp:275-284`). `Character::Update` delegates to `SolidMovingSprite::Update` (`Game/src/GameObjects/Character.cpp:137-175`); the motion layer integrates gravity/collision and records grounded state (`port/game/PlayerMotion.cpp:53-68,147-203`). Forest-1 authored player spawn is Y=3385 (`Game/XmlFiles/levels/forest_exploration_1.xml:33-50`). The port previously left that position frozen in its intro branch while ordinary motion advanced only after intro completion.
- The new call is above `IntroBodyFinished()` so the completion tick gets one physics step even though the ordinary `else` is not entered until the *next* tick (`port/linux/main.cpp:640-649,687-742`). Its copied coordinates are consumed by the burst and blood flight (`:659-665`) and follow camera (`:673-679`) on the current tick. The existing strict `introHeldTime > 2.75f` predicate, Looped→Pull animation transition, and visual/preceding-rendered-delta cadence were not changed.
- The existing paper/upgrade modal branches precede the intro branch (`:614-640`), so either modal skips this update. The intro branch sits outside the later `simulateOnly` rendering skip (`:1178-1184`), hence simulation-only still steps once and advances the terminal visual signal. Ordinary camera follow remains before ordinary motion (`:715-742`), preserving that path's prior-target behavior. Intro follows the freshly integrated position as requested; exact Windows camera timing remains a matched-trace gate. The completion branch continues to skip an intro camera follow on its terminal tick, pre-existing behavior not expanded here.

## Private verification

- Compiled from repository root (output only in `/tmp`): `g++ -O2 -std=c++17 -UNDEBUG -Iport/compat port/tests/fresh_save_intro_physics_cpu_test.cpp port/game/LevelScene.cpp port/game/PlayerMotion.cpp Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp -o /tmp/fresh_save_intro_physics_cpu_20261023 && /tmp/fresh_save_intro_physics_cpu_20261023`. Exit 0: `fresh-save intro physics CPU test passed contact_tick=12 spawn_y=3385 ground_y=3206.9`.
- Fixture uses the actual Forest-1 scene and `PlayerMotion`: first-tick descent, contact by tick 12 at 60 Hz, 180 grounded hold ticks without drift, horizontal nonmovement, strict mathematical `<`, `=`, `>2.75` threshold predicate, burst spray offsets from the *updated* motion coordinate, and further grounded completion/first ordinary no-shot ticks. A source-seam guard asserts exactly one intro `motion.Update`, its order before coordinate copy/completion/burst/camera, and modal/simulate-only branch placement. It does **not** execute the whole main loop or prove Windows camera parity; completion and ordinary steps are driven explicitly in the fixture.
- `g++ -std=c++17 -fsyntax-only -Iport/compat port/linux/main.cpp` passed. `git diff --check -- port/linux/main.cpp` passed. New fixture and report are untracked, so Git's diff check does not cover them; they were compiled/read directly. No unrelated dirty file was altered in this scope.

## Outstanding shared gates

1. Integration owner should run the current shared-tree build and full fresh-save scripted/simulate-only flow after other active owners release their files; compare real main-loop transition trace and pause/modal behavior. This private fixture checks source placement and physics, not full-loop execution.
2. Matched Windows fixed-delta trace for contact timing, pull/flight origin and camera on threshold/completion ticks; the current 12-tick Linux contact result is not a Windows parity claim.
3. Physical GPU :0 capture for body/camera/burst/occlusion and the first ordinary no-shot draw, after GPU owner releases the device. Existing arm fixture remains independent; active Combat directed-impact repair must be integrated separately.

## Ownership release

**EXPLICIT RELEASE:** Exclusive ownership of `port/linux/main.cpp`, `port/tests/fresh_save_intro_physics_cpu_test.cpp`, and this report is released. No commit or push was made; shared-tree integration may review and take over these exact changes.

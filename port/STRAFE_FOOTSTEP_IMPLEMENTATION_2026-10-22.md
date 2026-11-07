# Strafe material footstep implementation handoff — 2026-10-22

## Scope and ownership

At start, HEAD was `8f8d817` on `feature/linux-renderer-foundation`; `port/game/LevelScene.h/.cpp` were clean (file mtimes 20:45/20:05 PDT), with no observed process naming/touching them. The newest coordinator status assigned this bounded scene slice to this worker and reserved a different GPU/Docker aim-line worker with no scene-source ownership. Existing unrelated dirty status/integration files and untracked reports were left untouched. No Docker, GPU, Xorg or capture was used.

## Source contract and changes

Windows `Game/src/GameObjects/Character.cpp:1006-1063,1119-1179` calls the animation effect after horizontal frame advance for Running or either strafe sequence, compares the numeric frame with one `mLastRunFramePlayed`, and stamps that index on every horizontal tick. Directed particles use `m_direction.X`, not the strafe aim/body orientation. `Game/XmlFiles/animation/player_anim_default.xml:366-522` marks StrafeForward sequence indices 0/10 and StrafeBack indices 9/19 (the latter displays reverse-order textures 010/000). Forest-1 player/ground XML identifies grassground contact and `Game/XmlFiles/materials.xml` maps it to `rock_particle.png`.

`LevelScene.h/.cpp` now pass an explicit movement sign to the existing material emitter, dispatch on Running plus StrafeForward/StrafeBack, and share the existing numeric `lastRunningFrame_` guard/stamp across all three. Running keeps its old facing sign and particle constants; strafe body facing still follows aim. Contact validity, exact water-material suppression, depth, three-particle form and RNG remain in the emitter.

One new focused CPU fixture `port/tests/strafe_footstep_cpu_test.cpp` checks both marked frames per sequence, repeated/unmarked frames, same-index sequence switch suppression, opposed aim/body versus movement spray direction, invalid/airborne/water contact, and a Running positive control. It loads original forest XML and writes/removes a temporary water-material XML copy only during the test.

## Verification and blocker

- Final focused host-only C++17 fixture build and execution: **PASS** (`PASS: authored strafe footstep frames, shared guard, movement direction, contact/water, Running`). No renderer/GPU use.
- Scoped `git diff --check`: **PASS**.
- Existing `port/tests/strafe_body_cpu_test.cpp` fixture, unchanged, was built and run: **FAIL** at line 159, the old assertion that 50 grounded StrafeBack ticks emit zero sprays. Its other zero-strafe assertion near line 173 is also now stale. This is a test-expectation conflict introduced by the intended behavior, not a source compile failure. The task allowed only one focused CPU fixture, so this worker did not edit the pre-existing broad body fixture or `port/tests/integration.sh`. The full integration gate will fail until the coordinator assigns/permits updating that fixture's obsolete negative assertions and reruns it.
- Therefore **no commit or push**: the software is left isolated in the two scene files plus one new fixture for coordinator integration, not represented as a green full gate. No visual acceptance claim.

SHA-256 (final files):

- `port/game/LevelScene.h`: `06faaf977c298b59066d92fedf3b2ddc50c7fe6998a8ee17e88d630a686e8c02`
- `port/game/LevelScene.cpp`: `fe7ea09950e1b991d47924670616eb75cdfff758e6a51012fdffe354be90c2ec`
- `port/tests/strafe_footstep_cpu_test.cpp`: `3b7d864df50215471d01a73d8fe220755127458b347cb967999ddf40725f841a`

# Forest-1 ID2 Running frame-rate order — 2026-10-23 21:36 UTC

## Bounded correction

Original `Character::UpdateAnimations` selects Running, calls `AnimationPart::AnimateLooped()` using its sequence's stored rate, then calls `SetFrameRate` from current velocity (`Game/src/GameObjects/Character.cpp:1006-1056`, `Game/src/Animation/AnimationPart.cpp:142-196`). `SetSequence` restarts frame/elapsed but leaves that rate in the sequence. Forest ID2's authored Running initial rate is 60 fps and min/max 4/60 (`Game/XmlFiles/animation/player_anim_default.xml:188-207`).

`port/game/LevelScene.cpp` now uses `sequence.currentRate` to advance Running, then clamps/stores the velocity-derived rate after the frame decision, exactly as its existing strafe branches do. Re-entry still resets only frame/elapsed; no rate reset was introduced. Still, other body modes, preceding-rendered-delta clock, and one-frame-per-update logic are unchanged.

## Live CPU draw-command gate

Unique test: `port/tests/running_rate_order_cpu_test_RUNNING_RATE_ORDER_20261023T2136Z.cpp`. It loads the actual Forest-1 XML into production `LevelScene`, maps `LoadTexture` IDs back to paths, filters to ID2, and checks the body path emitted by `LevelScene::Draw` after live `UpdatePlayerVisual` calls. All updates are grounded, dry, non-strafing, facing right; current `dt=.003` differs from explicit prior rendered deltas.

- 800 velocity / .016 prior: `run_default_000.png` (authored 60 fps, .016 carry).
- 60 velocity / .008 prior: `run_default_001.png` (stored 60 advances, then 4.77 fps is stored).
- Continued 60 velocity / .100 and .105 priors: frames 001 then 002, proving low-rate use and fractional carry.
- .500 then .001 priors: frames 003 then 004, proving one-step maximum and excess carry.
- Exit to Still; re-enter at 800 / .016: frame 000 (stored 4.77 persists while cursor resets); next .001: frame 001 (60 was stored on re-entry).

Verification from repository root: scoped host `g++ -O2 -std=c++17 -UNDEBUG -Iport/compat` compile of the unique test plus `LevelScene.cpp`, `PlayerMotion.cpp`, and four TinyXML translation units **PASS**; executable **PASS**. Same scoped CPU compile/run for existing `port/tests/strafe_body_cpu_test.cpp` **PASS**. `git diff --check -- port/game/LevelScene.cpp` **PASS**. The source diff is only the Running/strafing rate selection and post-advance assignment, plus its explanatory comment.

No physical GPU/Xorg `:0`, Docker, global build/integration, pixel capture, commit, push, shared checkpoint/status file, or unrelated dirty file was touched. These CPU command results do not establish original-Windows same-state pixel parity; that matched rendered-frame gate remains pending. Scoped implementation and test are handed back for coordinator integration after the reserved Waterfall ID46 lane is released.

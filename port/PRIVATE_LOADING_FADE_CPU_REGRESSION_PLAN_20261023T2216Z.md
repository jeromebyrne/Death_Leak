# Forest→Waterfall fade: CPU regression fixture plan (read-only lane)

## Verdict and scope

The next actionable correction is **entry-latched gameplay pause plus opaque hold-exit draw** in `port/linux/main.cpp`; this is the same correction identified in `PRIVATE_LOADING_FADE_INDEPENDENT_SOURCE_QA_20261023T2212Z.md` and specified in `PRIVATE_LOADING_FADE_FIX_DESIGN_20261023T2214Z.md`, not a new independent defect. This report contributes only a nonduplicative regression-fixture specification. It does not implement or claim to run a test. No source, build, GPU, git index, or shared checkpoint was touched.

## Original Windows oracle

- `Game/src/UI/UIGameLoading.cpp:7-8,40-58`: `StartLoading` pauses gameplay, zeroes alpha, and starts FadeIn; `kMinTimeToLoad=.05f`, `kFadeTime=.20f`.
- `Game/src/UI/UIGameLoading.cpp:60-93`: exclusive `if / else if` chain with strict `>` thresholds. On the update that leaves the hold, `FinishedLoading()` runs but the FadeOut alpha branch **does not** run; the already-opaque sprite remains opaque for that draw.
- `Game/src/UI/UIGameLoading.cpp:96-129`: `FinishedLoading()` unpauses; `Load()` starts FadeOut time at destination load, **before** the minimum hold. Do not reset this clock at hold exit.
- `Game/src/Game.cpp:154-255`: game objects and collision update before `UIManager::Update()`. Thus hold-exit itself uses the entry pause; the following FadeOut-entry frame runs gameplay. `Game/src/GameObjects/GameObjectManager.cpp:205-257` additionally gates weather and objects by pause. `Game/src/program.cpp:148-175` increments total time after `Game::Update`, so this plan asserts ordering and strict thresholds, not wall-clock parity or identical frame ordinals.
- Port sites to observe: `port/linux/main.cpp:675-711` (phase/timer/scene), `:1313-1339` (weather/scene/player/combat visual gates), `:1505-1513` (splash tint). These line numbers are the pre-correction snapshot; find the corresponding code after implementation.

## Minimal connected CPU fixture

Add one **future** dedicated test, e.g. `port/tests/private_loading_fade_cpu_regression_20261023.py`, using a private compile-time observer/renderer spy around the production `main.cpp` frame loop. It must execute the real Forest-1→Waterfall authored trigger and production scene replacement, not a copied fade state machine or regex over source. Match the existing CPU/software test precedent in `port/tests/private_body_slow_frame_command_gate_20261023.py`: compile into a temporary directory, run with `SDL_VIDEODRIVER=dummy`, `SDL_RENDER_DRIVER=software`, `SDL_AUDIODRIVER=dummy`, and `DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1`; use a fixed delta and an authored-route scripted input or narrowly scoped test fixture placing the player immediately before the actual trigger. Keep its observer passive: per presented frame record `phaseOnEntry`, `phaseAfterUpdate`, `loadingPhaseTime` before/after, destination-install count, gameplay-arm count, weather/rain update count, `scene.UpdateVisuals` count, player/combat visual update count, and final splash draw command alpha (`DrawSprite` tint or `FillRect` alpha). A software command spy is enough; no saved image or physical GPU.

**Important seam limitation:** `--simulate-only` is insufficient because `main.cpp:1320-1339` skips ordinary scene/player/combat visuals and drawing, so it cannot prove the full contract. A standalone unit test of `LevelScene` also cannot prove the main-loop pause or splash. The fixture requires the proposed passive observer or equivalent production-path instrumentation. There is no currently runnable one-liner that proves these conditions without adding it.

Assertions, in event-relative terms (not a fragile fixed ordinal):

1. On trigger-start frame, the ordinary gameplay arm and its visuals finish; the outgoing Forest remains. During FadeIn entry and OpaqueHold entry, gameplay/weather/scene/player/combat updates do not increment. Destination installation happens once, only when FadeIn elapsed is **strictly greater** than `.20f`, and the first destination/hold draw is opaque.
2. A hold frame at elapsed `<= .05f` stays in OpaqueHold. On the first hold frame with elapsed `> .05f`, `phaseOnEntry=OpaqueHold`, `phaseAfterUpdate=FadeOut`, gameplay and all gated visual counters remain unchanged, and submitted splash alpha is exactly `255` (or normalized `1`). This is the essential pause-on-entry and opaque hold-exit discriminator.
3. On the **next** frame, `phaseOnEntry=FadeOut`, gameplay and weather/scene/player/combat visual counters each advance once, and submitted splash alpha is below `255`, computed from the unreset clock that began at `Load()`. Further FadeOut frames continue gameplay under the splash. On the first `loadingPhaseTime > .20f` frame, FadeOut retires, splash is absent, and gameplay/visuals still advance once. At `<= .20f`, the splash remains.
4. An unrelated authored transition remains immediate (one control route); do not broaden the FadeIn scope merely to satisfy this fixture.

For a simple 60 Hz scenario, use `--fixed-dt 0.016666666666666666` and compare measured timer values against thresholds using the production float values. Add a second boundary scenario with fixture-controlled deltas that exactly straddle `.05f` and `.20f` (or explicitly seed phase time just below/equal/above each threshold); `main.cpp:269` caps `--fixed-dt` at `.05f`, and float accumulation makes assertions about a hard-coded ordinal unreliable. The observer should report the actual pre/post times, enabling strict-boundary assertions without assuming 3 or 12 increments equal decimal constants exactly.

**Proposed invocation after that fixture exists (not executed here):**

```sh
cd /srv/openclaw/workspace/death_leak_analysis/source
python3 port/tests/private_loading_fade_cpu_regression_20261023.py
```

The script should build its instrumented binary under `tempfile.TemporaryDirectory`, invoke the connected route under the above dummy/software environment, parse exactly one observation row per frame, assert all conditions, and exit nonzero with the offending row on mismatch. Do not use `port/build` or screenshots for this CPU gate.

## Existing coverage gap

`port/tests/integration.sh:637-640` asserts the long simulation route triggers a transition **from** `forest_exploration_2.xml` to `waterfall_top.xml` and later returns to Forest-1; it neither exercises the special Forest-1→Waterfall FadeIn nor inspects phase, pause, visual updates, or splash alpha. A search of `port/tests` found no test references to `loadingPhase`, `LoadingPhase`, or `UIGameLoading`; other fade tests concern unrelated foliage, blood, fog, etc. Existing integration and `--simulate-only` therefore do **not** cover either regression discriminator. A future observer test is justified; no test result is claimed.

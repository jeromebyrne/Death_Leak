# Forest follow-X temporal CPU discriminator — 2026-10-22

## Verdict

A read-only XML-driven CPU trace confirms the *source-predicted* transform/UV split for forest-1 IDs 37 and 40. After a settled camera at X=1000, a single +8-world-unit camera step puts the Windows cached quad center at **634.666667**, versus Linux's **640.000000** (−5.333333 logical/backbuffer pixels at 2/3 scale), while both use the *current* camera's equivalent UV phase. The next update with X held at 1008 converges to 640. This is a deterministic code-path prediction, not an observed Windows render or a newly measured pixel delta. Existing within-port ID37 GPU A/B (`FOREST_FOLLOW_LAG_GPU_DISCRIMINATOR_2026-10-22.md`) found a small moving-frame image effect and zero held-frame difference; it is separate evidence, not part of this CPU run. Retain present Linux behavior pending a matched original-Windows transform/frame trace if exact legacy parity warrants the change.

## Source contract and trace method

- XML `Game/XmlFiles/levels/forest_exploration_1.xml:612-635,666-689`: both IDs are always-updated, follow X but not Y, unflipped, non-autoscrolling `ParallaxLayer`s, width 2048, camera-X UV multiplier 0.000690, alpha 0.95, authored X=1316.985352. ID37 is 2048×512 `shrubbery.png`; ID40 is 2048×677 `trees_top.png`.
- Normal unpaused `Game::Update` follows the camera before `GameObjectManager::Update` (`Game/src/Game.cpp:154-180`; `GameObjectManager.cpp:221-257`). `ParallaxLayer::Update` calls `Sprite::Update` before assigning camera X to `m_position.X` (`ParallaxLayer.cpp:26-35`). Through `DrawableObject`, the base `GameObject::Update` builds `m_world` from the *entering* position (`GameObject.cpp:160-212`); `ParallaxLayer` disables ordinary object parallax in its constructor. `ParallaxLayer::Draw` then computes U from current camera X; `Sprite::Draw_effectLightTexture` sends the cached world matrix (`ParallaxLayer.cpp:37-83`; `Sprite.cpp:626-637`). The manager's later draw pass is `GameObjectManager.cpp:302-330`.
- Linux `port/game/LevelScene.cpp:1170-1171,1217-1253` uses `worldX=cameraX` and `uvShiftX=+cameraX*0.000690` in one draw. The positive SDL shift is equivalent to Windows's negative X shift with reversed vertex-U ordering; it is not a sign disagreement.
- I ran a Python 3 standard-library, in-memory trace reading the two actual XML nodes and asserting their follow/flip/autoscroll flags. For each draw, the model first cached the entering follow X as Windows world X, then assigned the current camera X for the *next* update, and computed current-camera UV. It also computed Linux's current-camera destination/UV. No script or test file was written. Assumptions: one ordinary object update per draw, source/port camera X equal at draw, layer visible, no zoom/shake, logical 1280×720 and scale 2/3. This is a source-equivalent arithmetic trace, not execution of the Windows game.

## Exact A / A+8 / A+8 result (identical X result for IDs 37 and 40)

A=1000, starting after a settled update at A. Quad width is 1365.333333 screen pixels. `Win U` gives left/right vertex U; `Linux UV` is its equivalent SDL phase. Screen delta means Windows center minus Linux center. Y differs by authored ID but has no temporal follow-X change.

| Draw | Camera X | Windows pre-follow X → post-follow X | Windows center; left/right edges | Win U left/right | Linux center; UV | Screen delta |
| --- | ---: | --- | --- | --- | --- | ---: |
| A, held | 1000 | 1000 → 1000 | 640.000000; −42.666667 / 1322.666667 | 0.310000 / −0.690000 | 640.000000; +0.690000 | 0 |
| A+8, moving | 1008 | 1000 → 1008 | 634.666667; −48.000000 / 1317.333333 | 0.304480 / −0.695520 | 640.000000; +0.695520 | −5.333333 |
| A+8, held | 1008 | 1008 → 1008 | 640.000000; −42.666667 / 1322.666667 | 0.304480 / −0.695520 | 640.000000; +0.695520 | 0 |

Formula for ordinary successive updates: Windows-minus-Linux destination center = `(previousFollowX-currentCameraX)*2/3`; UV phase change from A to A+8 is +0.005520 in Linux convention (Windows `xShift` decreases by 0.005520). This is a horizontal destination translation only; it cannot explain a persistent dark band's vertical extent.

## Initial, reset, and skipped-update boundaries

`GameObject::Initialise` constructs its first world matrix from the authored X (`GameObject.cpp:79-88`). With first camera X=1000, the first ordinary update still builds from XML X=1316.985352 before following camera; the source-equivalent first-draw center is **851.323568** (delta +211.323568 versus Linux 640), with current U 0.310000/−0.690000. The second update held at 1000 converges. This first-draw calculation is conditional on actual load/update/draw ordering and visibility; it is not a captured launch frame. A reload/new scene must discard old follow history. If an implementation elects exact first-update source parity, initialize each layer's cached pre-follow X from its authored X; if it deliberately suppresses the first-frame jump, seed it from the initial camera X and document that as a Linux-specific normalization. Never carry a previous level's camera X into a new level. If object updates are skipped, the world matrix may remain stale longer than one frame; do not label the effect categorically as a one-frame lag.

## Conditional implementation/decision gate — no shared-source edit made

Only if a matched Windows trace and visual priority justify parity: add per-following-layer previous-follow-X state to the port scene sprite/visual-update path; before each eligible visual update, cache the layer's entering follow X, then set its followed X to current camera X. In `LevelScene::Draw`, use the cached X **only** for that follow-X parallax layer's destination; leave `cameraX` for `uvShiftX`, and leave Y, opacity, depth, XML, art, other sprites and active strafe-footstep code untouched. Initialize/reset state per the explicitly chosen first-draw contract above. A CPU render-command gate should assert both IDs' three rows here, held convergence, and fresh-load behavior; then compare a same-history original-Windows frame/transform before promoting the patch. The existing `FOREST_FOLLOW_X_WINDOWS_TRACE_GATE_2026-10-22.md` says that original-Windows gate remains unavailable locally. If the measured effect remains immaterial or no matched trace is obtained, close as a low-priority source difference.

Scope: only this unique report written. No LevelScene/test/shared-checkpoint edit; no build, Docker, GPU, Xorg, or capture. The concurrent strafe-footstep integration worker retains ownership of LevelScene/tests.

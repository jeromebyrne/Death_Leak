# Forest-1 camera-follow position lag: CPU source audit (2026-10-22)

## Verdict

**Yes, Windows has a persistent one-update transform lag for forest-1 ParallaxLayer IDs 37 and 40 while the camera moves.** Their texture UVs use the *current* camera X, but their draw/world matrix retains the X assigned during the previous object update. During steady rightward movement at the authored player's ordinary maximum speed, this makes each Windows layer quad **8 world/screen pixels left of true camera-centering at 1920×1080, 60 updates/s** (5.33 backbuffer pixels at 1280×720). It is an ongoing velocity-dependent offset, not accumulating drift. Linux draws these layers at the current camera position and does not reproduce this position lag; it retains the current-camera UV shift. No GPU/display test was run.

## Source path and ordering

- `Game/XmlFiles/levels/forest_exploration_1.xml:612-635,666-689`: both layers are updateable/always-update, `followcamx=true`, `followcamy=false`, width 2048, `cameraparallaxmultiplierx=0.000690`, X auto-scroll false. ID 37 is `shrubbery.png` (Y 2800); ID 40 is `trees_top.png` (Y 4100). Their authored initial X 1316.985352 is **not** their moving draw X after initialization.
- `Game/src/Game.cpp:155-181,259`: `FollowTargetObjectWithLag()` moves camera before `GameObjectManager::Update`; `Camera2D::Update()` builds the view later. `Game/src/Core/Camera2D.cpp:173-211` applies the per-update X lag divisor: authored `xLag=15` from forest XML line 3, or 40 when enemies exist, with zoom 1.
- `Game/src/GameObjects/GameObjectManager.cpp:205-250`: always-update layers receive one object update per unpaused, non-damage-paused game update. `Game/src/GameObjects/ParallaxLayer.cpp:26-34` calls `Sprite::Update(delta)` **before** assigning `m_position.X = m_camera->X()`.
- `Game/src/GameObjects/Sprite.cpp:358-366`, `DrawableObject.cpp:41-43`, `GameObject.cpp:160-212`: the base update constructs `m_world` from the *old* `m_position`. Its later `UpdateToParent()` does not rebuild that matrix. `Sprite.cpp:626-633` sends this cached world matrix to `effectlighttexture`, the effect authored for both IDs. Thus after object update `n`, position reports `C_n` but matrix translation is `C_(n-1)`.
- `Game/src/GameObjects/ParallaxLayer.cpp:37-85,159`: draw reads `C_n` for X UV phase (`-C_n × 0.000690` for these unflipped layers) and then invokes `Sprite::Draw` with the stale matrix. Therefore the geometry center's screen X is `C_(n-1) - C_n = -ΔC_n`; ideal centered geometry is zero. The UV phase itself is **not** one frame late.
- `port/game/LevelScene.cpp:951-985`: the Linux port uses `worldX=cameraX` for `followCameraX`, subtracts the same cameraX in its draw destination, and uses current cameraX for UV shift. Its quad center remains screen-centered. `port/linux/main.cpp:663-680` follows using the same authored per-update lag, before motion update. Linux materially differs only in this layer-geometry timing, not in intended camera motion/UV phase.

## Predicted 60 Hz offsets

`Game/XmlFiles/levels/forest_exploration_1.xml:53-59` authors the player's ordinary maximum horizontal velocity at 8 units per target update. At 60 Hz this is 480 world units/s; after the camera's first-order lag reaches steady following, `ΔC=+8` world units/update independent of the lag divisor (15 normally, 40 with enemies). Accordingly both ID 37 and 40 have identical predictions:

| State | Camera step | Windows quad-center error vs current camera / Linux | UV texture phase step |
|---|---:|---:|---:|
| Steady rightward, 1920×1080 | +8 px/update | −8 px (left) | `2048 × 0.000690 × 8 = 11.30496` texture pixels left/update |
| Steady rightward, 1280×720 | +5.333 px/update | −5.333 px (left) | ≈7.537 backbuffer px/update |
| Camera stationary | 0 | 0 | 0 |

The 1280×720 figures assume the game's uniform 2/3 level scale (`GameObjectManager.cpp:450-466`, `ParallaxLayer.cpp:223-231`); its X multiplier is inversely scaled, preserving authored motion in proportion to display size. During acceleration, stopping, boundary clamping, or camera shake, substitute the actual `C_n-C_(n-1)`; neither `xLag=15` nor enemy divisor 40 directly means a fixed 15- or 40-pixel error. A natural player/camera follow lag exists on **both** platforms and is distinct from this extra Windows layer-transform lag.

## Focused deterministic check and minimal fix

A CPU-only regression can instantiate a ParallaxLayer with the forest fields, set camera X to `100`, call `Update(1/60)`, then move camera X to `108` and call `Update(1/60)` again. Inspect the world-matrix X translation and reported `Position().X` after the second update: current Windows should give matrix X `100`, reported X `108`; the expected corrected values are both `108`. For an end-to-end frame check, also assert that the UV phase is calculated from `108` and that Linux's quad destination remains centered. This deterministic state test needs no texture/device or gameplay capture if matrix/UV accessors are isolated in the test harness.

The narrow Windows source fix is to assign followed X/Y **before** `Sprite::Update(delta)` in `ParallaxLayer::Update`, removing the assignments after it. This lets `GameObject::Update` build its world matrix from the current camera while preserving UV logic in `Draw`. Do not shift the Linux quad to imitate the stale Windows transform unless pixel-for-pixel legacy bug compatibility is the explicit target; that would introduce a visible 8-pixel motion-dependent error. This report is an audit only: no source edits, build, render, commit, or push.

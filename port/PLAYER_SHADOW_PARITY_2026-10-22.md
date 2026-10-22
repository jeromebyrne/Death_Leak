# Player ground-shadow parity slice — 2026-10-22

## Source and implementation

Windows `GameObjectManager::LoadLevel` calls `Character::AddShadowSprite()` after loading the XML objects and before depth sorting (`Game/src/GameObjects/GameObjectManager.cpp:438-450`). `Character.cpp:1856-1886` creates a non-native 200×128 sprite from `Media\\characters\\player\\shadow.png`, attaches it to the player at `(0,-150)` without parent orientation, and assigns `kGround`. `Character::Update` switches its alpha between 1 on a solid surface and 0 otherwise (`:195-226`). The 128×128 RGBA source has a low-alpha black footprint; it is not forest ID46's XML `leaves_shadow.png`.

The port now appends one runtime shadow after XML sprites, retains stable kGround painter ordering ahead of the kPlayer body, follows player position without camera-depth parallax or facing flip, and draws only while `UpdatePlayerVisual` reports grounded. It uses the existing texture-alpha, SDL vertical-UV, and scene sepia path. The synthetic diagnostic ID `__player_shadow` permits an isolated hide/only selection without changing XML-ID2 arm tests. `SpriteCount()` continues to report authored XML sprites (71 in forest-1), excluding this runtime child. No art, XML, ID37/ID15, blend settings, shared status, or checkpoint files were changed in this slice.

## Verification and evidence

- Docker `deathleak-linux-build:bookworm` final `make -C port -j2`: pass. Focused `port/tests/player_shadow_parity_test.cpp`: pass for texture load, authored sprite count, kGround-before-body order, 200×128 dimensions, `(0,-150)` attachment, full tint, no facing flip, camera-follow position, and airborne suppression/reappearance. Existing player-arm parity regression: pass. Final log: `port/build/player-shadow-parity-2026-10-22/final-build-test.log`.
- Physical Xorg `:0`, nouveau `NVE7`, accelerated OpenGL and GPU Sepia.fx: two deterministic forest-1 frame-121 captures with fixed 1/60-second dt and idle script `.:121`. Both logs finish at player `(-43.0146,3206.9)`, `grounded=1`; the baseline only sets `DEATHLEAK_RENDER_HIDE_XML_IDS=__player_shadow`. The 1280×720 RGB difference is confined to `(573,546)-(707,584)` (exclusive right/bottom), **2,444 changed pixels**. This shows a local grounded shadow contribution, not a broad foreground change. Logs and BMP/crop artifacts are under ignored `port/build/player-shadow-parity-2026-10-22/`.
- Baseline BMP SHA-256: `b16c3b8372819026c5d437a8d8d3eb06e30706c5bc9facd4fef9e6c1c2884422`; shadow BMP SHA-256: `34cb98148cf625afeb4f01847a20be1830c9df20db1a99598e17d2242e37663b`. The captures preceded the later independent parallax-phase commit; the final shadow build/test passed after that commit. Its parallax change is outside this shadow slice.

## Limit and next gap

This is source-semantic parity plus Linux same-frame GPU evidence, **not** visual acceptance or exact Windows pixel parity. A matched original Windows camera/player/grounded frame is still unavailable. The shadow cannot account for the larger ID37 dark band, ID15 pale shaft, or global color mismatch; those remain independent visual gaps. The separate next-source audit owns prioritization beyond this slice.

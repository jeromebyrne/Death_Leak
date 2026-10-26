# Third runtime atlas family — exact-pixel rejection, 2026-10-26

Base: clean detached `be37d071a2a67e2ce2c7976e24b4663593709e60` (`origin/feature/linux-renderer-foundation`); sole source/build/physical-NVE7 owner. Primary seven unrelated dirty tracked paths and all untracked artifacts were preserved. This report is the only accepted repository change; both candidate runtime/asset edits were reverted.

## Selection and residency

Six `Game/Media/objects/branch_1..6.png` assets are ordinary nonrepeating `effectlighttexture` Sprites at 16 authored scene references, including active Forest-1 IDs 27–31 and Waterfall-top IDs 52–54. The existing scene-branch opt-in page is scene-resident; gameplay-wide icon/HUD pages are not compatible places for large scene-only art. It was regenerated with `--full-rgba` as a cropped 1746×607 page, 2-pixel extrusion, source SHA validation, and byte-exact reconstruction including transparent RGB. Candidate only switched `DEATHLEAK_ATLAS_SCENE_BRANCHES` default on; `off` remained dedicated. Painter order, tint, source rects, rotation, virtual UV, and dedicated wrap/UV-shift/private-shader fallback were unchanged.

Distinct bounded alternative: `Game/Media/cart_tracks.png`, `objects/cart.png`, `objects/cart_dark.png` use an existing scene-resident cart/prop page. Twenty-seven authored cart-track ordinary Sprite references span levels; Forest-1 has visible cart tracks at the B:180 moving-camera endpoint. Full-RGBA regeneration produced one cropped 1528×516 page, with the same SHA and reconstructed-byte verification. `DEATHLEAK_ATLAS_CART_PROPS=off` was the dedicated control. No new page was allocated. Repeated `horse_tracks.png`, private-shader/wobble/foliage, particle, parallax/wrap, and render-target assets remain outside this ordinary sprite family.

## Build and exact physical pixels

Baseline and each candidate built from isolated worktrees with `deathleak-linux-build:bookworm`, excluding the primary dirty `SDLRenderer.cpp`. Strict serial X11/OpenGL SDL accelerated=1/software=0 vsync=1 on physical Mesa nouveau NVE7 at 1280×720, 180 fixed-dt Forest-1 frames, `B:180` moving route. Both A/B/A runs ended at identical player `(1368.99,3107.3)`, five orbs, 25 health, and counted 15,990 actual SDL sprite submission calls per arm. BMP payload comparisons exclude only the 54-byte header.

- **Branch A/B/A:** controls exactly equal SHA-256 `06145c2fccbb75152d47aa5132cb6cbe6a09ee9d00d68786c568c3cec90794d6`; candidate `6c4544a2af9c4d327b5a66d948dac5ad69f20c84a22ffd6024bf1e2a24b52fee` differed in 71 RGB payload bytes, 49 pixels across the branch screen band. Rejected.
- **Cart A/B/A:** controls again exactly equal; candidate `a4026a01368976432a33d21b6f760dcd0575c9f662f1b4c8b3322b234dbe2814` differed in two one-LSB bytes at `(1172,609)` and `(1180,610)`. A test-only transparent-cart-track suppression changed 5,651 RGB pixels over x416..1209,y607..633, proving this was a positive visible cart branch. Original source PNG was restored and its SHA-256 `92608676...a085ae1` rechecked. Rejected.

The small differences are consistent with sampling/rounding changes from atlas UVs, but the precise GL/SDL cause was not isolated. **Neither default mapping is accepted under strict exact-pixel parity.** All candidate source/assets were reverted. No full-frame film-clock synchronization was necessary for these gates: each control pair was byte-identical.

## Actual GL calls, not inferred batching

`deathleak-apitrace:bookworm` traced baseline and cart candidate serially on the same physical NVE7 B:180 route. The cart candidate trace allocated its 1528×516 page. Across steady frames 121–180, calls delimited by `glXSwapBuffers` were identical:

- `glDrawArrays`: **6,235 / 6,235** (median 104 per frame)
- private `glBegin`: **360 / 360** (median 6 per frame)
- `glBindTexture`: **8,970 / 8,970** (median 149.5 per frame)

The whole 180-frame trace also had 17,294 `glDrawArrays`, 1,080 `glBegin`, and 25,988 `glBindTexture` calls in each arm. Atlas identity did not lower actual submissions or binds on this route; the draw pipeline still respects painter/blend/shader ordering. Apitrace overhead prohibits timing conclusions. No FPS, VRAM, 8K/high-Hz or release claim follows.

## Remaining coverage and next discriminator

Default atlas coverage remains **15/896** media PNGs (nine gamepad icons, six HUD sprites); **881** have no default atlas mapping. Branch, cart/prop, orange-light remain opt-in; menu buttons previously failed exact pixel parity. A next atlas slice should isolate SDL/OpenGL sampling-coordinate precision for a visible ordinary sprite before another default switch; any adjustment needs strict serial exact-RGBA A/B/A on the real route. Reordering painter calls or treating a shared page as automatic draw batching is not justified.

Ignored reproducible evidence: primary `port/build/atlas-third-family-20261026/` (`run.sh`, `trace.sh`, BMPs/logs, GL traces/call dumps, `gl-stats.json`, branch-rejected directory). These are not part of this report commit.

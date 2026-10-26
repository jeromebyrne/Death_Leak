# First runtime-integrated atlas family — 2026-10-26

Base: clean detached remote `e9f41296b23dcb48ac547c47447eaa6b6173984c`; sole source/build/physical-NVE7 owner. Primary unrelated dirty paths were not source/build inputs.

## Inventory and implementation

Physical `:0` GLX/Mesa nouveau NVE7 reports accelerated GL 4.3, `GL_MAX_TEXTURE_SIZE=16384`, and 506 MiB video memory. The project ceiling remains 4096×4096. `Game/Media` contains 896 PNGs and 451,933,320 source pixels; 21 have an edge >2048, none >4096. Loading converges on `SDLRenderer::LoadTexture`; ordinary submission is `DrawSprite`, scene traversal is `LevelScene::Draw`, and explicit texture loads occur in `main.cpp`, game classes and authored XML. Repeat-X/UV shifts and private-GL shader paths sample outside an ordinary region and retain dedicated fallback. Render targets are separately allocated SDL textures, not PNG atlas candidates.

The existing deterministic free-rectangle packer `port/tools/atlas_pack.py` was exercised as the production asset pipeline. It tries every existing compatible page before opening another, caps each at 4096, trims emitted page bounds, adds 2-pixel edge extrusion, records SHA-256/rect/original dimensions/offsets in JSON and verifies reconstruction. For the first family, `--full-rgba` retains even invisible RGB so the complete original image bytes reconstruct exactly. Nine UI gamepad icons (main startup key-Esc/Y and fresh-save X/Deck; XML UI also references this family) occupy one 2156×320 page, 68.0% useful sprite-rectangle occupancy. Raw RGBA page residency is 2.76 MiB versus 1.88 MiB summed dedicated source rectangles, a ~0.88 MiB trade for shared texture identity. This is not a claimed VRAM saving.

`SDLRenderer::LoadTexture` now selects this verified family by default; `DEATHLEAK_ATLAS_ICONS=off` is an explicit dedicated-texture control. Other experimental families remain opt-in. Virtual IDs map to the same page but retain source rect, destination/flip/rotation/tint/filtering and painter slot. Repeating/shifted/private-shader sprites still take the dedicated source path. No draw-count reduction is claimed: each icon remains one SDL submission until compatible consecutive batching is separately proven.

## Gates

- `atlas_pack.py --full-rgba` regenerated all nine icons and verified byte-exact RGBA reconstruction and source hashes. Candidate and clean baseline separately passed the Docker production compile; final native-source build and diff check passed after test fixture removal.
- Strict serial physical X11/OpenGL NVE7, SDL `accelerated=1 software=0 vsync=1`, 1280×720 fixed-dt Forest-1 fresh-save prompt-visible frame 500 A/B/A full post-HUD BMPs were byte-identical, SHA-256 `b76f07ce387f9cc1779b0647cf85df6de9ef5531a27cd22f7b75a5c8cd148b14`, zero changed pixels. Simulation endpoint/player/vitals match. Same test-only frame-indexed film clock was used in both isolated binaries; removed before final build. Candidate prompt suppression changed 2,953 RGB pixels (bounds x=567..664, y=429..481), proving that the icon draw was actually visible in the parity route.
- Evidence (build logs, A/B/A logs/BMPs and capture script) remains under ignored primary `port/build/atlas-first-family-20261026/`. Native arbitrary film-phase parity, p50/p95/p99 gain, transition/thermal acceptance, 8K/high-Hz and release readiness are not established.

## Disposition and remaining scope

Accepted: one real atlas-backed ordinary icon family, full-RGBA manifest/page and default runtime mapping, conditional on final scoped commit/ref verification. Rejected: treating a pre-existing opt-in page as shipped integration; claiming the atlas itself batches draws or saves VRAM; atlas-mapping repeat/wrap/private-shader cases without virtual UV and filter proof.

Remaining unatlased media PNGs: 887 of 896, including 417 under `characters`, 123 `objects`, 105 `levels`, 53 other `UI` assets and 189 elsewhere. Existing opt-in HUD/scene-branch/cart-prop/orange-light pages are **not** counted as accepted default coverage. Each remaining runtime family needs use-site filtering/source-rect/wrap/mipmap/shader classification, scene-aware residency, exact-frame GPU parity and a memory/performance gate before default enablement. Large source rectangles plus 506 MiB VRAM exclude an all-pages-resident design.

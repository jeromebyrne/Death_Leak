# Forest IDs 17/19: current CPU render-command gate — 2026-10-23

## Scope and verdict

Read-only reconstruction from the **current dirty source**, immutable XML/PNG, historical fixed-state `R:180` logs, and already saved current-tree stills. **The Y/depth/alpha/flip/culling contract is determined; a literal actual same-frame command trace is not present in the saved evidence.** In particular, the `R:180` logs record player `(1368.99,3107.3)` but **not camera X/Y or the submitted commands**. The XML/normal `followy=false` route fixes camera Y at 3400 absent a camera override; `R:180` has no logged override. Camera X follows with lag and cannot be replaced by player X. The existing 0556 current-tree MP4/stills have no render ordinal/camera sidecar. Thus X tile intervals below are an explicitly labeled **diagnostic reconstruction**, not a falsely claimed runtime emission. No build, game/display/GPU invocation, gameplay recording, source edit, shared checkpoint edit, or commit.

## Current CPU command contract at the fixed forest-1 route

At 1280×720, scale 2/3, `cameraY=3400`, for camera X **C**, `LevelScene::Draw` (`port/game/LevelScene.cpp:1186-1312`) and the XML (`Game/XmlFiles/levels/forest_exploration_1.xml:269-313`) produce:

| ID / painter slot | Destination `(left, top, width, height)` px | Texture / command state |
| --- | --- | --- |
| **19**, depth 5500 `kMiddleBackground`, XML order 19, drawn **first** | `(-1740.144745 - C/3, 401.429235, 4902.746419, 682.666667)` | `ground_blur.png` RGBA 734×1024; `repeatX=true`, `flipX=true`, SDL `flipY=true` (authored verticalflip=false), rotation 0, `uvShiftX/Y=0`, ordinary light-texture path; white tint `(255,255,255,255)`, opacity 1 |
| **17**, depth 5000 `kNearBackground`, XML order 17, drawn **after 19** | `(-1701.880534 - C/2, 464.826667, 4902.746419, 682.666667)` | `ground.png` RGBA 734×1024; same repeat/flip/shader/tint/opacity state |

Both textures pass asset load in the historical/current logs (71 sprites, zero missing). Neither has an occlusion-fade effect, animation, foliage/pixel shader or rotation. Dynamic painter items above/below these depths can overlap later; admission is not last-writer proof. `ViewportCull.h:10-22` admits 17 when approximately `-5963.76 <= C <= 6401.73` and 19 when `-9060.43 <= C <= 9487.81` (Y overlap always passes). Thus both pass for the known forest-1 authored camera X 0 and any plausible `R:180` follow position, but a live culling bit was **not logged**. `LevelScene.cpp:620,1222-1230` stably sorts descending depth, so ID19 precedes ID17. The current `SDLRenderer.cpp:223-252` dispatches both into the ordinary unrotated repeat-X SDL-copy branch with linear scale mode, not the fractional-parallax branch.

`RepeatX.h:16-40` makes 11 tile slots, nominal tile width `734×2/3=489.333333` screen px. For a visible screen X, flipped tile index is `floor((destinationRight-X)/489.333333)`; nominal tile target runs from `destinationRight-(i+1)×489.333333` to `destinationRight-i×489.333333`, clipped at the authored quad's left. Its source interval is `[0,734)` for each **full** tile, with horizontal SDL flip. The exact sampled source column depends on unlogged C. The following shows one concrete phase, **C=1213.8**, selected only because its predicted alpha≥128 rows fit historical isolated color-onset observations; it must not be cited as measured camera X:

| ID | screen X | repeat tile / target X interval px | source interval / approx sampled X | PNG first `alpha>0` / `alpha>=128` screen Y |
| --- | ---: | --- | --- | --- |
| 19 | 320 | 4 / `[311.335,800.668)` | `[0,734)` / 721 | 447.43 / 451.43 |
| 19 | 640 | 4 / `[311.335,800.668)` | `[0,734)` / 241 | 460.76 / 464.10 |
| 19 | 960 | 3 / `[800.668,1290.002)` | `[0,734)` / 495 | 451.43 / 457.43 |
| 17 | 320 | 4 / `[147.299,636.633)` | `[0,734)` / 474 | 520.16 / 520.16 |
| 17 | 640 | 3 / `[636.633,1125.966)` | `[0,734)` / 728 | 516.83 / 516.83 |
| 17 | 960 | 3 / `[636.633,1125.966)` | `[0,734)` / 248 | 528.83 / 528.83 |

The table's sampled X is point-center arithmetic from the full-tile SDL flip, **not** a claim of exact linear-filtered GPU sampling. The alpha threshold is read from the immutable RGBA PNG bottom-up because SDL `flipY=true` places PNG bottom at destination top. `C=0` instead gives sample columns 17 `[651,171,425]`, 19 `[593,113,367]`, showing why unlogged camera X matters. The diagnostic `C≈1214` fit (threshold-128 predicted rows 19 `[451.4,464.1,457.4]`, 17 `[520.2,516.8,528.8]`) is close to old isolated color proxy `[451,463,457]` / `[520,516,529]`, but color proxy, linear filtering, sepia and process differences prevent inversion of those rows into an actual camera trace.

## Camera-X-independent alpha corridor and current capture comparison

Across **all 734 source columns** (the immutable-PNG scan already documented in `FOREST_MIDDLE_BAND_SOURCE_DISCRIMINATOR_2026-10-23.md`), at camera Y=3400 the first nonzero-alpha support is ID19 **y439.43–462.76**, ID17 **y504.83–530.16**. First alpha≥128 support is ID19 **y445.43–466.76**, ID17 **y505.49–530.16**. Allow at least ~1–2 screen rows for linear filtering/rounding and recognize that opaque post-sepia RGB is not raw alpha. If actual camera Y differs by ΔY, shift ID19 by `(2/3)×0.97×ΔY` and ID17 by `(2/3)×0.99×ΔY`. X phase changes exact rows within these broad envelopes, not the envelope itself.

The saved **current-tree**, lossy H.264-derived 0556 stills (`port/build/current-tree-gpu-review-0556-2026-10-23/still-{1,5,9}s.png`) are 1280×720 but have no matched camera/effect ordinal. A read-only top-down Rec.709 luma finite-difference scan at x320/640/960 inside broad y437–470 (ID19) and y501–535 (ID17) finds, for example, 1-second strongest absolute peaks **ID19 y460/459/457** and **ID17 y519/521/503**, and 9-second peaks **ID19 y440/468/442**, **ID17 y511/521/532**. Most are within/near the wide predicted ranges; y503/532 are outside the raw alpha corridors but are merely strongest *composited* gradients amid other layers and compression, not a measured ID-specific onset. This is **source-consistent at best**, not evidence that either strip is the present line's last writer. The prior historical post-V-fix isolated frames have stronger color-onset agreement but are separate processes and not current HEAD (`FOREST_MIDDLE_BAND_EDGE_CPU_REVIEW_2026-10-23.md`). Do not cross-subtract them with pre-fix hide frames.

## Decision / precise remaining gate

No evidence here rejects the ID17/19 authored-alpha explanation for a line in the above Y bands; none proves it for the composited current image or a Windows parity defect. A line demonstrably outside **both** camera-adjusted corridors would reject their alpha onset as its cause. The missing literal-command fields are **same-render camera X/Y, filter state and the submitted SDL tile commands**, plus a lossless same-frame current screenshot with line coordinates. A separately owned CPU spy at `LevelScene::Draw`/`ForEachRepeatXTile` on the fixed route can supply those without changing art or draw semantics; any composited hide/last-writer comparison belongs to the exclusive capture owner. A matched original-Windows state remains necessary to call a surviving line a Linux regression.

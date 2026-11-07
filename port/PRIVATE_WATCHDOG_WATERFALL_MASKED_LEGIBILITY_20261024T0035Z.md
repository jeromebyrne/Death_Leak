# Waterfall player alpha-mask legibility discriminator — 2026-10-24 00:35Z

## Decision

The original run-frame alpha **does permit a source-backed approximate player segmentation** in the saved Linux capture. Within that mask, the baseline player's silhouette remains locally distinguishable from its adjacent scene: mean absolute paired edge luminance difference is **24.0/255** (median 24.2); the player-side edge is darker by 9.8 on average. Hiding ID29 barely changes this edge contrast (24.1; +0.1). Hiding ID46 raises it to 29.1 (+5.1), but darkens both silhouette and adjacent background; this is a Linux-only legibility discriminator, **not a finding that ID46 is wrong or should be hidden**. No matched original-Windows packet exists, so there is no parity verdict or acceptance of a renderer/art change.

## Inputs and registration

Read `PRIVATE_WATCHDOG_WATERFALL_PLAYER_OCCLUSION_GPU_20261024T0031Z.md` first. Used its saved final 1280×720 BMPs `baseline`, `hide29`, `hide46`, `repeat` and packet JSON under `port/build/private-watchdog-waterfall-player-occlusion-gpu-20261024t0031z/`; no GPU/display/build execution. All packets retain the same draw-ordinal-16 player command: `Game/Media/characters/player/run_default/run_default_004.png`, source `[0,0,0,0]` (full image), destination `(46.8589478,211.752106,120.929184,143.568024)`, unrotated, no X/Y flip, white tint and opacity 1. The actual RGBA asset is 641×811, with nonzero-alpha bbox `(41,196,472,805)`. Pixel-center inverse-mapping of this full image into its destination yields a **threshold-alpha ≥128** solid mask in integer ROI `[46,211,168,356)`: 2,283 mask pixels of 17,690 box pixels. It excludes the 15,407 transparent-box/background pixels that diluted the previous box metric. The source alpha is a justified silhouette prior, not a renderer-generated ID buffer: bilinear sampling, fractional placement, anti-aliasing, sprite outline transparency, postprocess, and later foreground compositing make its exact final coverage uncertain. The draw packet's `bodyArmClipFrame` field is null, but the ordered draw supplies the concrete sprite path and frame, so the mask is reconstructible to this approximation. The shadow and aim-line are separate draw commands and are intentionally excluded.

## CPU measurement

For each final BMP, define a 2-pixel **inner edge band** as mask minus 5×5-eroded mask (970 pixels), and a 2-pixel **outer adjacent band** as 5×5-dilated mask minus mask (1,017 pixels). Pair each inner-band pixel with its nearest outer-band pixel within four pixels (970 pairs). Luminance is Rec.709 sRGB-weighted code value, `0.2126R+0.7152G+0.0722B`, not linear-light luminance. The absolute paired difference measures local outline visibility, not perceptual readability. Signed difference is inside minus outside.

| Final BMP | Mean solid luminance | Mean outer-band luminance | Paired signed edge | Paired absolute edge | Median absolute edge |
|---|---:|---:|---:|---:|---:|
| baseline | 58.90 | 65.87 | −9.77 | 23.99 | 24.20 |
| hide29 | 57.91 | 65.30 | −9.81 | 24.11 | 24.49 |
| hide46 | 44.23 | 51.70 | −11.69 | 29.11 | 29.20 |
| repeat | 58.90 | 65.87 | −9.77 | 23.99 | 24.20 |

The inner-edge band's baseline-to-hide29 mean absolute luminance change is 2.54, versus 2.03 in the adjacent outer band and 2.52 in the solid mask; its signed mean change is +0.82 versus +0.56 outside. This is a weak local modulation, not an erased silhouette. For hide46, baseline minus hidden is +16.18 in the inner band, +14.17 in adjacent outer band, +14.68 in the solid mask; absolute changes are 16.26, 14.18, 14.79 respectively. ID46 is a broad brightening effect crossing the body/background boundary. Its removal increases measured outline contrast mainly because the dark player edge separates more from a darkened surrounding scene, while both regions' code values fall; this does not establish that the result is more legible to a human. Repeat matches baseline exactly, supporting capture determinism for this state.

## Boundary and next gate

The approximate alpha mask resolves the prior *box-is-not-player* limitation, but these metrics cannot decide original intent or parity. The single first-postfade frame is especially weak evidence of human readability over motion. Keep authored XML nonfade flags and render ordering unchanged. The acceptance gate for any narrow correction remains a lossless matched native Windows D3D10 foreground-overlap frame/packet with same camera, pose/frame, ordinal/clocks, postprocess stage, and ID29/46 draw state, followed by registered player-edge and adjacent-background comparison against a provenance-identified Linux binary. Only a source-contract mismatch plus matched residual would justify naming a correction. No such mismatch emerges here; no shared source, tests, or status files edited.

Reproduction of this CPU metric used only Pillow; scratch script `/tmp/waterfall_masked_legibility_0035.py` and an enlarged threshold mask `/tmp/waterfall-player-alpha-mask-0035.png` are isolated temporary outputs. No image processing or package install touched the saved captures.

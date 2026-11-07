# Forest-1 lower-foreground / pale-shaft renderer differential — 2026-10-22

## Decision

No bounded renderer correction is justified by the current evidence. Independent source and physical-GPU-frame checks support the existing attribution: the near-black lower screen is chiefly authored parallax ID37 (`shrubbery.png`, alpha 0.95), and the right pale slab is chiefly authored rotated sprite ID15 (`sun_shaft_2.png`, alpha 1). The conspicuous colors and silhouettes are present in the original assets and XML, and the reviewed Windows/SDL paths agree on the material transformations that would make them visible. Do not recolor, hide, move, or change these objects merely to resemble an unmatched Steam still. This is a diagnostic slice only; no code, XML, texture, or test was changed.

## Source-level differential

| Contract | Windows source | Linux port | Finding |
| --- | --- | --- | --- |
| ID37 placement | Forest-1 XML lines 612–635: `(1316.985,2800)`, 2048×512, `kMiddleForeground`, follow camera X, not Y; X phase coefficient `0.000690`; alpha `.95`. `ParallaxLayer::Update/Draw` follows X, sets one repeat-X UV span, retains world Y and fixes V bottom=0/top=1 when Y parallax is off. | `LevelScene::Draw` lines 728–785 follows X, retains Y, uses one UV span and `-cameraX*.000690` phase, and reverses ordinary SDL image Y for original D3D vertex convention. | No sign, position, V-orientation, or alpha error found for ID37. |
| ID15 geometry | XML lines 237–251: `(2796.985,3207)`, 512×512, rotation `-1.469999` radians, `kGroundFront`, no flips, alpha 1. `Sprite::RecalculateVertices` maps one quad; its world transform applies authored rotation. | Ordinary `SDL_RenderCopyExF` path receives source radians converted to degrees, with SDL Y inversion accounted for in the port's vertical image orientation. It is not passed through pixel/foliage wobble. | No supported placement, rotation-sign, or effect-selection error found for ID15. |
| Sampling and composite | `BasicLightTexture.fx` samples RGBA and multiplies sample alpha by object alpha; source-alpha-over RGB blend in `Graphics.cpp`; `Sepia.fx` transforms the scene. Its light-texture sampler specifies linear minification, point magnification, wrap. | SDL texture color/alpha modulation and blend mode; GPU sepia pass translates the original matrix/gating. SDL ordinary-sprite copies use the platform texture sampler. | Large-area darkness and slab shape are not explained by a demonstrated blend/postprocess mismatch. A possible one-pixel edge-filter difference cannot justify changing texture sampling without a matched Windows frame or a direct sampler probe. |
| Painter order | Original depth function is always-pass, so ordered draws govern overlap. | Depth-sorted draw sequence and alpha blend. | No newly identified ID37/ID15 occlusion-order defect. |

## Independent frame and asset verification

I re-decoded the *existing* lossless, same-frame physical-GPU A/B captures with Pillow (`ImageChops.difference` on RGB), separate from the earlier report's luminance mask. These were captured on x11/OpenGL, strict accelerated `nouveau/NVE7` with GPU sepia; respective baseline/hide pairs use identical deterministic simulation settings. This review did not occupy the GPU or make a new capture because the same-frame isolated evidence already discriminates both objects.

- ID37, `port/build/id37-parity-followup-2026-10-22/{baseline,hide,only}.bmp`: hiding ID37 changes 92,285 RGB pixels, bounding box `[0,597,1280,720)`. At `(640,700)`, baseline `(5,4,3)` versus hidden `(94,82,59)`; at y590 there is no difference. The earlier 90,958 count uses a luminance-difference mask, not this any-channel RGB criterion.
- ID15, `port/build/pale-patch-gpu-audit-2026-10-22/{baseline,hide-15,only-15}.bmp`: hiding ID15 changes 81,525 RGB pixels, bounding box `[985,323,1280,612)`. At `(1100,350)`, baseline `(149,129,93)` versus hidden `(95,82,58)`; the only-ID frame has the same flat-topped region. The other fog IDs are not its geometry.
- Direct RGBA read of the original 2048×512 forest `shrubbery.png`: at x512, rows 64/128/256 are opaque black; row384 is black with alpha219, row448 alpha56, row511 alpha0. This predicts a sharp transition to near-black where the quad extends down the viewport at alpha .95.
- Direct RGBA read of original 512×512 `sun_shaft_2.png`: at x256, alpha grows 9/53/97/140/162 at rows 100/200/300/400/450, then falls to 82/21 at rows 480/490. The colored source field is broad and rectangular, so a long straight edge is not by itself a port artifact.

## Remaining discriminator / release limit

The available Steam forest screenshot and post-rotation gameplay frames have unmatched camera, scene state, time and crop; therefore they cannot establish Windows-vs-Linux pixel/color parity for either object. The next meaningful renderer test is an original-Windows forest-1 frame at the same viewport, camera XY/zoom, object/effect time and state as one deterministic Linux capture, ideally with ID37 and ID15 isolated. Compare transformed quad bounds, UV orientation/phase, edge sampling, and post-sepia RGB on those paired frames. Only a measured discrepancy should drive a scoped correction. No Windows parity or Jerome acceptance is claimed.

GPU lane is released; no game/capture process was started by this diagnostic review. No software change, build, commit or push applies to this slice.

# Forest-1 ID46 CPU interpolation discriminator — 2026-10-23

## Verdict

At the specified fixed forest-1 `R:98` state, the original single-quad triangles and Linux per-repeat-tile triangles give **different interior V** (about 0.66–1.37 source texels at sampled points) and a smaller U-phase difference (about 0.04–0.16 source texel). This is a CPU geometry/interpolation finding, not a measured visual seam or Windows parity result. There is also a correction to the proposed crop: the feasibility report's **x≈233 seam omitted the screen-center `+640`** in `LevelScene.cpp:1259`. With its predicted camera X=593.214240, the visible unswayed repeat boundary is **x≈872.906**, and its deformed edge runs x≈869.210 at top to x≈872.906 at bottom. At y=640 the boundary is x≈870.954. x=233 is inside a repeat tile, not at a boundary. Do not crop x225–241 as the seam test.

## Method and inputs

Read-only arithmetic (Python double-precision `sin` and barycentric calculations; no game, GPU, build, source/test edit). The input is XML ID46 (`Game/XmlFiles/levels/forest_exploration_1.xml:765-778`): center (2086.573242,2976), width 10000, height 252, flipX, repeatX, top-source TL (7,6), TR (8,9), bottom (0,0), time multiplier 1, 2048×252 texture. The source flip puts source TR at **screen TL**, so screen TL gets (8,9), screen TR gets (7,6). Source UV after flip: screen left U=10000/2048=4.8828125, right U=0; V=1 at screen top and 0 at bottom. The original `Sprite.cpp:181-191` strip `2,1,3,0` becomes screen triangles `(TL,BL,TR)` and `(TR,BL,BR)`, diagonal BL–TR.

Assume the documented no-enemy/pre-motion `R:98` camera prediction (593.214240,3400), 1280×720 view, 2/3 world-to-screen scale, 98 fixed updates of `dt=0.0166667` => shader-clock time 1.6333366 s (`port/linux/main.cpp:460-464,1110`; `ShaderClock.h`). Weather's foliage multiplier remains its initial 1 before its 180-second threshold (`WeatherLightning.h`); authored multiplier is 1. For each original corner, apply `x'=x+sin(0.1y+T)*Ix`, then `y'=y+sin(0.1x'+T)*Iy`, as in `FoliageSway.fx:41-65` / `FoliageSway.h`. Project x with **`640+(worldX-cameraX)*2/3`** and y with `360+(cameraY-worldY)*2/3`. The resulting `(screen x,y)` are TL `(-1701.644296,564.388961)`, TR `(4965.507824,559.535702)`, BL `(-1697.760665,726.666667)`, BR `(4968.906001,726.666667)`.

For Linux, reproduce `SDLRenderer.cpp:564-592` and `RepeatX.h:16-40`: tile screen corners are lerps along the *deformed* top and bottom whole-quad edges at tile fractions; each tile uses triangles `(TL,TR,BL)` and `(TR,BR,BL)` (the same diagonal orientation, but many local diagonals). Right-anchored 2048-world-unit tiles have local U=1 at each full tile's left and U=0 at its right. For comparison, add the tile's integer repeat index to its local U (`i=3` left of this boundary, `i=2` right); integer offsets otherwise misleadingly look like 4096–6144 texel differences under wrap. Evaluate both triangle meshes barycentrically at identical screen points. All tabulated points lie inside both meshes.

The boundary is the third full-tile edge from the right: world X=942.573242, whole-quad fraction 0.3856. Its deformed top/bottom points are `(869.209561,562.517545)` and `(872.906001,726.666667)`; x at y=600/640/680 is 870.054/870.954/871.855. The original and tile mesh share this *edge geometry*; the distinction is interior interpolation.

## Numerical samples

`Δ` means Linux minus original, after removing integer U repeat offsets. `ΔU·2048` and `ΔV·252` are source-texel units, useful only as scale indicators (not a rendered color difference). Values below are rounded; they come from double-precision arithmetic on the stated camera prediction, not a captured draw.

| Screen sample | Original (U,V) | Linux local (U,V), tile | ΔU·2048 | ΔV·252 |
|---|---:|---:|---:|---:|
| y600, x866.054 (4 px left of edge) | (3.0029682, .7690409) | (.0029297, .7716561), i=3 | −0.079 | +0.659 |
| y600, x874.054 (4 px right) | (2.9971094, .7690051) | (.9970706, .7716383), i=2 | −0.079 | +0.664 |
| y640, x866.954 (4 px left) | (3.0030096, .5225502) | (.0029297, .5279752), i=3 | −0.164 | +1.367 |
| y640, x874.954 (4 px right) | (2.9971508, .5225143) | (.9970706, .5279575), i=2 | −0.164 | +1.372 |
| y640, x948 (interior control) | (2.9436554, .5221866) | (.9435751, .5276335), i=2 | −0.164 | +1.373 |
| y680, x867.855 (4 px left) | (3.0030044, .2792221) | (.0029297, .2842943), i=3 | −0.153 | +1.278 |
| y680, x875.855 (4 px right) | (2.9971450, .2792221) | (.9970706, .2842766), i=2 | −0.153 | +1.274 |
| y680, x948 (control) | (2.9443046, .2792221) | (.9442348, .2839567), i=2 | −0.143 | +1.193 |
| y640, x233 (**not** seam) | (3.4672898, .5253939) | (.4672517, .5279752), i=3 | −0.078 | +0.651 |

The change is **not a sharp discontinuity at the tile edge**: nearby left/right U phases nearly coincide after wrap, and the V discrepancy persists at x948. That pattern supports a triangulation/interpolation difference, not a demonstrated repeat-addressing seam. The texture's edge guard (entire columns 0–5 and 2035–2047 have zero alpha per `ID46_WRAP_REFERENCE_FEASIBILITY_2026-10-23.md`) further reduces the expected composited signal immediately beside the boundary. A diagnostic nearest-texel lookup into the actual PNG at the y640 ±4-pixel samples lands on transparent alpha-zero columns on both paths (original: columns 6/2042, Linux: 6/2042, rows 131 versus 133). At the y640 x948 control, nearest lookup changes alpha 207→201 (rows 131→132, source column 1932); multiplied by the sprite's 0.5 opacity, this is at most about 3/255 **for that nearest-sample diagnostic**, before filtering, overdraw and postprocessing. Nearest lookup is not the D3D sampler contract and is not a pixel prediction.

## Limits and next gate

Camera X is a source-derived prediction, not a logged/captured R:98 camera. Floating-point shader/renderer precision, raster sample-center rules, interpolation perspective details, mip selection, actual GL wrap/min/mag state, texture sampling, occlusion, and sepia are not established by this CPU calculation. The Steam JPEG is not a same-state original frame. Consequently neither the 1.37-texel V difference nor the alpha lookup justifies a source change or visible acceptance claim.

When the reserved physical GPU lane is available, the existing ID46 owner should first verify the actual R:98 camera and log ID46's deformed corners/tile UVs. Center the ID46-only and unfiltered lossless comparison near **the measured deformed edge around x870–873**, with an interior control near x948, rather than the feasibility report's x233 crop. Then inspect *draw-time* bound sampler/wrap/mip state and composited seam/control pixels as specified in `ID46_WRAP_REFERENCE_FEASIBILITY_2026-10-23.md`. A same-state Windows frame is still required for visual parity. This work did not use or interfere with the reserved waterfall Y-cadence GPU worker.

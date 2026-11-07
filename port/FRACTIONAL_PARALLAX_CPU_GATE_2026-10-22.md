# Fractional parallax: focused CPU regression gate — 2026-10-22

## Boundary and evidence

Read-only review of `FRACTIONAL_PARALLAX_UV_IMPLEMENTATION_PLAN_2026-10-22.md`, `FRACTIONAL_PARALLAX_TEXTURE_SPAN_AUDIT_2026-10-22.md`, and current source; this report is the sole deliverable. No renderer/test/checkpoint/status edit, build, Xorg, or GPU operation was performed. The fog-mip physical-GPU worker owns that lane. This specifies a *future* gate, not a passed test or a Windows pixel-parity claim.

Forest XML IDs 37 (`shrubbery.png`, 2048×512) and 40 (`trees_top.png`, 2048×677) are unflipped 2048-world-unit-wide ParallaxLayers with `cameraParallaxX=.000690`, no Y scroll, `alpha=.95`, and `repeatX=true`. `LevelScene.cpp:1234-1245` already suppresses ordinary repeat-X and hands the renderer `shiftX=cameraX*.000690*(flipX ? -1 : +1)`. The current `SDLRenderer.cpp:246-261` sends that float to `UVWrap.h:15-26`, whose `floor(fract(shift)*pixels)` produces an integer source cut. The original `ParallaxLayer.cpp:37-83` retains continuous UVs. The missing behavior is sub-cut motion, not an established broad color/lighting fix. ID40's 677-pixel Y size does **not** establish its SDL GL binding span.

## CPU contract to gate

For finite `shiftX,shiftY`, define `p(a)=a-floor(a)`, in `[0,1)`. The float helper's **screen** corners must be `TL=(pX,pY)`, `TR=(pX+sx,pY)`, `BR=(pX+sx,pY+sy)`, `BL=(pX,pY+sy)`, where `sx=flipX?-1:+1`, `sy=flipY?-1:+1`. These coordinates must remain floats until GL submission; neither `cut/pixels` nor rounding may enter this helper. Assert `TR.u-TL.u=sx`, `BL.v-TL.v=sy`, and that top/bottom and left/right shared coordinates agree. Do not reverse the already SDL-facing shift a second time on a flip. This is the screen-corner convention of the existing SDL-bound custom quads (`SDLRenderer.cpp:310-324,382-390`); it is not a direct assertion about PNG row order. Use a float-appropriate absolute tolerance (e.g. `2e-6` UV), with tighter arithmetic checks in double if the helper accepts double.

### Required deterministic cases

The table applies independently to **both** IDs: both have texture width 2048, destination width `2048×2/3 = 1365.333333...` logical pixels, and Y phase zero. `U_R=U_L+1` for the authored unflipped IDs; `V_T=0,V_B=1`. The `cut` column is a diagnostic of the **old** integer path, not an expected float-helper output.

| Camera X | `shiftX=U_L` | `U_R` | `U_L×2048` texels | old X cut |
|---:|---:|---:|---:|---:|
| 1000.0 | .690000 | 1.690000 | 1413.120000 | 1413 |
| 1000.5 | .690345 | 1.690345 | 1413.826560 | 1413 |
| 1000.6 | .690414 | 1.690414 | 1413.967872 | 1413 |
| 1000.7 | .690483 | 1.690483 | 1414.109184 | 1414 |

- **Sub-cut motion:** 1000→1000.5 changes U at *every* X corner by `+.000345`, source phase by `+.70656` texel, and a fixed texture feature's screen position by `-.47104` logical pixel (`-1365.333333...×.000345`) while the old cut stays 1413. Assert nonzero float delta, not pixel color or rasterized displacement. `destination.x/y/width/height` must be unchanged for the two held-camera-follow draws (scene handoff is already separately covered by `parallax_speed_test.cpp`). ID40 uses the same X values despite its different height.
- **Cut seam crossing:** 1000.6→1000.7 changes U by `+.000069`, source phase by `+.141312` texel, and feature position by `-.094208` logical pixel, although the old cut changes 1413→1414. Assert the float delta equals this small amount (no one-texel jump), corner span stays exactly +1, and modulo-1 phase is valid on both sides. This is a geometry invariant, not proof that GL filtering makes a seamless image.
- **Normalized wrap and signed input:** for X shifts `0,.999,1,1.001,-.25`, require `(U_L,U_R)` respectively `(0,1),(.999,1.999),(0,1),(.001,1.001),(.75,1.75)`. Check `p(-1.25)=.75` too. The UVs at `.999` and `1` differ by approximately `-.999` as raw normalized coordinates but represent a continuous modulo-1 sample step `+.001`; tests must compare circular phase near a wrap, not raw subtraction. The full one-span edge never collapses to zero.
- **Flip X / signed scene shift:** with `flipX=true`, the scene-signed shifts for cameras 1000 and 1000.5 are `-.690000,-.690345`. Require `(U_L,U_R)=(.310000,-.690000),(.309655,-.690345)`; their mapped leading positions are `634.88000,634.17344` texels, both old cuts 634. All X corners advance `-.000345`, with edge span `-1`; a fixed feature still moves left by `.47104` logical pixel in magnitude. Across 1000.6→1000.7 require phases `.309586→.309517`, leading positions `634.032128→633.890816`, old cuts `634→633`, and float delta `-.000069` without a one-texel jump. A pure helper should also exercise `shiftX=-.25,flipX=true` yielding `(.75,-.25)` and a +.25 shift with flip yielding `(.25,-.75)`; never negate the supplied shift again.
- **Flip Y and combined flips:** independent synthetic Y cases `shiftY=.25,flipY=false` give `(V_T,V_B)=(.25,1.25)`; `shiftY=-.25,flipY=true` give `(.75,-.25)`. With X=`-.25`, X flip=true and Y=`-.25`, Y flip=true, all four corner pairs must be `(.75,.75),(-.25,.75),(-.25,-.25),(.75,-.25)` in TL/TR/BR/BL order. At exact texel-aligned phases on a tiny texture (e.g. `pixels=8,shift=.25`, cut 2), compare the helper's nearest-sample order modulo 8 against `ForEachUVWrapSegment` for each axis, including flip, as an orientation cross-check; do not demand this equivalence for fractional phases, where the legacy cut is intentionally lossy.

## CPU dispatch/fallback decision cases

Factor a pure eligibility predicate (or test an equivalently isolated decision) accepting successful binding, renderer GL availability, finite returned SDL spans, and relevant GL NPOT-repeat capability. The proposed documented span tolerance is `abs(span-1.f)<=1e-6f` **on both axes**, in addition to `span>0`; reject nonfinite values. This tolerance is for representation noise only, not permission to sample padding. Synthetic decision cases:

| Inputs (`texw,texh`; other conditions true unless stated) | Expected path |
|---|---|
| `(1,1)` for ID37 and ID40 | Direct float quad eligible |
| `(1,.75)` for ID40; `(.75,1)` for either ID | Legacy integer SDL-copy fallback |
| `(1,.9999)`; `(0,1)`; `(NaN,1)`; `(1,+Inf)` | Fallback |
| `(1,1)` but SDL bind failed, non-GL renderer, or NPOT repeat unsupported for ID40 | Fallback |
| `(1,1)` with a tiny representational deviation, e.g. `(0.9999999,1)` | Eligible under the stated tolerance |

The CPU predicate cannot discover actual `SDL_GL_BindTexture` spans, prove hardware NPOT repeat, assess physical seam sampling, or prove state restoration. Do not multiply modulo-one UV by a subunit span and enable `GL_REPEAT`: that wraps padded allocation. For a subunit span, retain integer fallback (and its fractional limitation) with one-time diagnostic; an exact-size **private** texture is a separately gated implementation if needed. Test selection precedence so repeat-X ordinary sprites, non-full source rects, rotated sprites, wobble, foliage, and damage-burst never enter the new UV-scroll branch. The current forest sprites have no such conflicting effect; synthetic dispatch cases suffice for CPU coverage.

## Scoped handoff after renderer ownership clears

Re-read then-current source. Add a header-only float-corner helper and a standalone `fractional_parallax_uv_test.cpp` target analogous to existing CPU targets, implementing the cases above without GL or a display. Keep `UVWrap.h` for fallback and existing waterfall coverage. Route only the full-texture, unrotated UV-scroll branch through a float quad after the real binding/capability gate; preserve the incoming signed phase, destination, tint, sampled alpha, and authored float opacity **once** (current `Sprite::opacity` is separate from `tint.a`). Save/restore the GL and SDL-owned texture state on every exit. Do not borrow or alter fog PixelWobble's private mip cache; use only filters supported by complete levels. After the fog worker releases build/GPU ownership, run this focused CPU gate and existing `waterfall_uv_wrap_test`/`parallax_speed_test`, then an exclusive physical-GPU ID37/ID40 held-camera and seam-crossing check with logged actual spans/path and matched state. Neither this report nor a future CPU pass alone establishes Windows visual parity.

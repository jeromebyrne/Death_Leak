# Waterfall-top ID13/14 unbounded-V fix: CPU-only numeric discriminator

**Scope/result.** Read-only source/evidence arithmetic for committed `a5bbd981dccb371fe4b2b266a187164fa86dffaa`; no build, game, Xorg, GPU, capture, production edit, or shared-status edit. The `floor(V)` deletion can materially change the *second* sine's sampled U after the first integer crossing. It leaves the first sine's V displacement unchanged modulo texture repeat. This is a predicted shader-coordinate effect, not measured framebuffer RGB.

## Grounded inputs

- Waterfall-top XML `Game/XmlFiles/levels/waterfall_top.xml`, IDs 13/14: same `Media/waterfall_4.png`, `pixelwobbleintensity=.005`, `pixelwobblespeedmod=10`, Y-scroll speed 11, `alpha=.3`, horizontal flip true, vertical flip false. Texture is 512×1024 RGBA (`file Game/Media/waterfall_4.png`).
- Committed `port/game/LevelScene.cpp:1444-1455`: for wobble objects, phase now accumulates without `phase -= floor(phase)`; at steady 1/60 preceding rendered delta, step = `11 × (720/1080) × .001 = .0073333333 V/draw` = .44 V/s. The phase mutates at draw, subject to scene draw/culling history; frame count alone is not a universal phase clock.
- `port/linux/WobbleUV.h` and `port/linux/SDLRenderer.cpp:646-700,1094-1110`: authored U,V plus phase enter shader before V-then-U sine; final sample converts to `1-V` and wraps. Original `Game/ShaderFiles/BasicLightTexture_PixelWobble.fx` uses the same V-then-U order before wrap.
- Historical **pre-fix** Linux frame-638 packet `port/build/private-watchdog-waterfall-player-packet-20261024t0040z/frame638.json`: two waterfall_4 wobble draws at ordinals 6/7, V=.635334134, `pixelWobbleTime=9.3666954`, quads 540.441956×1080.88391, alpha=.3. This is an anchor for realistic submitted inputs, **not** a post-fix observation or Windows match.

## Discriminator

Let `P` be accumulated Y phase, `n=floor(P)`, `u` authored/interpolated U, `b` authored/interpolated V in [0,1], `k=10×effectTime`, and `q=.005 sin(u k)`. New/unbounded second-sine U shift is `.005 sin((b+P+q)k)`; old/floored is `.005 sin((b+P-n+q)k)`. Therefore

`ΔU = .005[sin((b+P+q)k) - sin((b+P-n+q)k)]`,

with maximum magnitude across a full quad `≤ .01 |sin(nk/2)| ≤ .01 UV`, or **≤5.12 texture texels** and **≤5.404 screen pixels** on the recorded 540.442-pixel quad. This is a coordinate-displacement bound, not an RGB bound. For n=0 the two paths are identical; for n≥1 equality occurs only at particular times/locations (e.g. `nk` multiple of 2π), not generally. Integer phase remains wrap-equivalent for the first sine's V perturbation and final V sample, but is *not* removable before the second sine.

Illustration: extrapolate the historical frame-638 phase at steady one draw per 1/60 s and the recorded reversing clock decreasing 1/60 s per draw (remaining above its lower reversal at 5). At local quad center U=.5, b=.5, include the first-sine `q` in both paths. Numbers are double-precision CPU arithmetic, not observed GPU fragments:

| Draws after anchor | P | Time | floor(P) | ΔU at center | Texture X offset | Projected quad X offset | max texture X offset over full quad |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 0 | .635334 | 9.366695 | 0 | 0 | 0 px | 0 px | 0 px |
| 49 | .994667 | 8.550029 | 0 | 0 | 0 px | 0 px | 0 px |
| 50 | 1.002001 | 8.533362 | 1 | +.009329 | +4.777 px | +5.042 px | 4.954 px |
| 60 | 1.075334 | 8.366695 | 1 | +.000704 | +.361 px | +.381 px | 4.288 px |
| 100 | 1.368667 | 7.700029 | 1 | +.002934 | +1.502 px | +1.586 px | 3.677 px |
| 150 | 1.735334 | 6.866695 | 1 | +.002199 | +1.126 px | +1.188 px | 1.138 px |
| 200 | 2.102001 | 6.033362 | 2 | +.003134 | +1.605 px | +1.694 px | 3.071 px |

The signed center offset varies rapidly with local U/V and time; the spatial bound uses `2×.005|sin(nk/2)|`, so it is a maximum possible within a full UV-height quad, not a claim that every visible pixel reaches that value. Texture sampling/filtering, image gradients, .3 layer opacity, other composited draws, and visible clipping can diminish or alter the RGB effect. The first integer crossing in this extrapolation is ~50 draws/~0.83 s after the historical anchor; actual post-fix transition depends on draw cadence, culling, clock, and level timing.

## Limits and handoff

This calculation supports **nonzero, potentially multi-pixel UV effect** for realistic ID13/14 inputs and gives a concrete post-fix discriminator: log both submitted V and wobble time after first crossing, then compare same-present pixels under a controlled old/new shader-input A/B. The existing `port/POSTFIX_A5BBD98_ACTIVE_SYNC_CAPTURE_READY_20261024T0505Z.md` is only a prepared capture and has not supplied that observation. No same-present RGB delta, matched Windows D3D command/clock or framebuffer parity, or visual acceptance is established here. Physical :0/NVE7 and shared build remain with the performance worker. **CPU/read-only lane released.**

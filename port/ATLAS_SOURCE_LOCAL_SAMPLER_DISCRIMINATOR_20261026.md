# Source-local programmable atlas sampler discriminator — 2026-10-26

Base: clean detached `d7bfea637c61728a7b99596435eece2d79dab830`. Sole source/build/physical-NVE7 owner. **Decision: reject.** This report is the only retained repository change. The seven unrelated primary tracked edits and untracked artifacts were preserved.

## Standalone-equivalent sampling contract

For an authored texture of dimensions `W×H`, let requested integer source rectangle be `R=(rx,ry,rw,rh)` or `(0,0,W,H)`. Intersect it with the atlas region's retained authored rectangle `T=(offsetX,offsetY,w,h)`; the intersection `I` maps to page origin `P=(region.x+I.x-offsetX, region.y+I.y-offsetY)`. For a destination quad, first apply the existing trim fraction `(I-R)/R` to destination **before** world projection, retaining the original destination center as the rotation pivot. Apply horizontal/vertical flips to the interpolated *source-local* coordinate, not to page coordinates. Rotation acts on destination vertices around that original center; it must not rotate or translate `P`. If the source rectangle is partly trimmed, the local sampler domain is the intersection and the untouched destination portion is not drawn. This maintains source-rectangle, flip, rotation and painter geometry independent of page placement.

For an eligible ordinary linear sample, let local normalized interpolation be `v`, after source cropping and flip, and local source dimensions `S=(I.w,I.h)`. Set `q=clamp(v*S-0.5, 0, S-1)`; `i=floor(q)`, `j=min(i+1,S-1)`, `f=q-i`. Fetch the four exact page texel centers `P+(i.x,i.y)+0.5`, `P+(j.x,i.y)+0.5`, `P+(i.x,j.y)+0.5`, `P+(j.x,j.y)+0.5`, divide by page dimensions only for the *fetch*. Bilinear weights are `(1-f.x)(1-f.y), f.x(1-f.y), (1-f.x)f.y, f.x f.y`. The clamp is to the original source texture's edge for whole-texture sampling, **not** automatically to an arbitrary SDL source rectangle: SDL's ordinary GL_LINEAR can sample adjacent original texels outside a subrectangle. An exact implementation must distinguish texture-edge clamp from source-rectangle coordinates and retain adjacent source bytes where required. Likewise, authored repeat, fractional UV shifts, private shader/mip chains, targets and other wrap semantics must stay dedicated until separate wrap/mip proof; clamping them to the virtual region would be wrong. Tint, alpha, blend mode and order must match SDL's GL path, including quantization, not just real-number RGBA algebra.

This is a *mathematical target*, not a claim that GLSL 1.20 manual interpolation is bit-equivalent to the GPU fixed-function linear sampler. Shader interpolation precision, fixed-function weight quantization, page texture fetch rounding, RGBA modulation and framebuffer blend rounding are separately observable risks. The prior matched physical draw showed ~2.8e-5 source-texel horizontal interpolation difference at the two original cart failures; local coordinates remove page UV endpoint normalization but do not prove identical downstream rounding.

## Bounded runtime candidate

An isolated test-only GLSL 1.20 fragment program was added for **atlased cart tracks only**. It interpolated local `[0,1]` UV, fetched four page texel centers with `GL_NEAREST`, mixed bilinearly and drew a private GL quad at the original cart-track painter slot. All other sprite paths stayed unchanged. `DEATHLEAK_ATLAS_CART_PROPS=off` selected the standalone control; `on` plus `DEATHLEAK_TEST_SOURCE_LOCAL_CART=1` selected the test shader. The three-source cart page was regenerated `--full-rgba`: 1528×516, two-pixel extrusion, source SHA and byte-exact reconstructed full RGBA verified by `atlas_pack.py`. Original `cart_tracks.png` restored SHA-256: `9260867645ad75cf9513257d6729f7f8ebb005ceffc5adc39d1efe9b7a085ae1`. Isolated Docker production compilation with an identical temporary frame-indexed film-clock fixture passed. The fixture, shader and regenerated page were all reverted after test; no renderer/asset change accepted.

The prototype only exercised full-source cart-track flips/rotation from Forest IDs 50/51. It did **not** validate partial source rectangles, every transform, arbitrary blend mode or repeat/mip exceptions. Its quad and manual blend path are an empirical candidate, not yet the complete contract above. A private GL path also inserts flushes and state transitions.

## Strict serial physical gate and measured benefit

On physical X11/OpenGL Mesa nouveau NVE7, SDL accelerated=1/software=0/vsync=1, 1280×720, fixed-1/60 Forest-1 moving `B:180`, strict serial A(off)/B(on+shader)/A(off) used the same binary and synchronized film fixture. Both control full BMP SHA-256 values were `c1a8741121c97f648346433a23d481807cbab7f2b0d2390a81d860f00dd7dfce`. Candidate was `6aa9d302649b3a08fb971f2d75e0b2944d76ed9b0236f4184955ebfe87dc9df7`: **27,357 BMP payload bytes / 11,157 pixels differ**, x402..1226,y86..632. All arms had 15,990 SDL sprite submissions and the same gameplay endpoint (player 1368.99,3107.3; five orbs; 25 health). The shader emitted 120 additional private `glBegin` calls over steady 60 frames, proving actual branch execution. Separately, test-only transparent standalone cart-track suppression changed **5,651 visible pixels** at the same capture route; original PNG was restored.

Fresh physical apitrace steady frames 121–180, with each arm traced serially using the same isolated binary:

| GL call | A standalone | B source-local shader | Delta |
|---|---:|---:|---:|
| `glDrawArrays` | 2,363 | 2,303 | -60 |
| private `glBegin` | 360 | 480 | +120 |
| total draw submissions | 2,723 | 2,783 | **+60** |
| `glBindTexture` | 8,970 | 9,150 | **+180** |

The changed path is neither byte-exact nor a draw/bind win. Pixel deltas are too broad to attribute solely to the previous two one-LSB UV failures: private-quad rasterization, fixed-function versus manual bilinear arithmetic, tint/blend quantization and GL state each remain possible. The test does not isolate which dominates. There is no FPS, VRAM or batching benefit claim.

## Next discriminator

If a programmable sampler is revisited, first render **both** standalone and atlas through the same private geometry/tint/blend path, capture matched GL vertices/state and fragment-center local coordinates, and compare that pair before comparing either with SDL ordinary output. Then isolate manual four-fetch arithmetic from hardware `GL_LINEAR` on the same texture and test partial source rectangles, both flips, rotation and boundary clamp/repeat explicitly. Only after byte-exact physical A/B/A across those cases should consecutive-draw batching be tried; measure new bind/draw reductions after removing private per-sprite flushes. The current per-sprite shader increases submissions and binds, so it is not a viable default even if its pixels were corrected.

Evidence retained under ignored primary `port/build/atlas-source-local-20261026/` (A/B/A BMP/logs and hidden-track BMP/log); fresh apitrace dumps/traces in the isolated build folder until cleanup. No 60-FPS median, 8K/high-Hz or release acceptance is claimed.

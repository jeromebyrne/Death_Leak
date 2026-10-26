# Menu-toggle atlas sampling-width discriminator — 2026-10-26

Base: clean isolated detached `d4e090414c1f818bda05fa147f6d226e52ce472a`; sole source/build/physical-NVE7 lane. **Disposition: reject runtime mapping/page; report only.** Primary dirty checkout was not edited or used as a build source.

## Exact local sampling finding

The eight 150×150 options toggles are ordinary SDL sprites: two selected variants draw per options frame, with no repeat, shifted UV, private shader or altered painter slot. A regenerated full-RGBA 1232×154 page retained 2-pixel extrusion and byte-exact source reconstruction. Its virtual source rectangles and geometry were unchanged. A focused strict physical capture reproduced the prior **nine visible RGB pixels, one channel by one LSB** (x372..438, y326..458) against identical dedicated-texture A controls. The page rect for first focused `music_on_focus.png` is `(464,2,150,150)`.

I changed **only the page canvas dimensions**, preserving all eight region locations/bytes, source rectangles, SDL `RenderCopy` path, minification decision, tint, blend and draw order. The exact same binary produced:

- 1232×154: 9 RGB pixels differ, max channel delta 1.
- 1280×160: 22 differ, max 1.
- 1500×154: candidate differs (full-BMP SHA `70803a88...`; no acceptance).
- 1536×154: 22 differ, max 1.
- **2048×154: zero differ** from both controls in the original three-update route.

This isolates the horizontal normalized-UV page denominator as a sufficient causal discriminator for this route, not a source-byte, screen-geometry or authored-filter difference. For the selected sprite's horizontal endpoints x=464 and 614, binary32 normalized UV multiplied back by page width has approximately `-1.7643e-5/+1.7643e-5` texel endpoint error at width 1232, versus exactly zero at width 2048 (both endpoints are binary-exact fractions). These are CPU float calculations, **not observed GPU internal interpolation precision**; a claim about the final blend/sampler rounding algorithm would exceed evidence. Vertical page height and sampling behavior were unchanged in the 2048×154 successful fixture.

Five more strict serial A/B/A 1280×720 full-BMP routes with independently reset settings covered on/off and focused/unfocused selected variants for both toggles. Every aligned-page candidate matched both controls byte-for-byte at each tested endpoint. This is a genuine family-specific pixel-preserving page-layout correction under tested states, **not** a general source-local atlas sampler or proof for partial source rectangles, flips, rotations or all camera positions. The exact-pixel fixture and images are in `/tmp/deathleak-menu-sampling-evidence/` (`check_sampling.py`, BMPs/logs, candidate assets); no shader or UV bias was shipped.

## Actual GPU calls, time, and footprint

The candidate compiled as a production binary in isolated `deathleak-linux-build:bookworm`. All physical runs used X11/SDL OpenGL, accelerated=1/software=0/vsync=1, Mesa nouveau **NVE7**, fixed 1/60 simulation dt. Fresh serial apitrace for the 2048×154 page, same three-update options route, A(off)/B(on)/A(off): `glDrawArrays` **32/32/32**, `glBindTexture` **111/95/111**, `glTexImage2D` **31/24/31**. The 16 fewer binds and seven fewer uploads are startup-inclusive residency/identity effects; draw submissions did not fall, and no steady-frame bind reduction is established.

Untraced 180-update menu A/B/A present intervals (179 samples) had p50 **16.6947/16.6766/16.6963 ms**, p95 **17.0294/17.0946/17.0767 ms**, p99 **17.6153/17.5296/17.6933 ms**. This is overlapping vsync-paced timing, **not** a repeatable FPS gain. The 2048×154 page has 315,392 texels / **1,261,568 bytes raw RGBA proxy**, versus 180,000 texels / **720,000 bytes** for eight dedicated source rectangles: **+541,568 bytes** before driver overhead and only **57.07%** source-content occupancy. The original dense 1232×154 page was 94.87% occupied but failed exact parity. GL uploaded the page in the traced candidate; this is allocation arithmetic, not a live VRAM measurement.

## Remaining eligible scope and decision

The tested options family consists of eight ordinary menu images, two active draws per options frame; unlike the default gamepad/HUD pages, it is menu-only. The other menu buttons include 181×181, 240×120, 256×256 and 512×512 art; they cannot simply fill this 154-pixel-high page as a proven compatible family, and earlier button atlasing failed exact pixels. An enlarged sparse page solely to eliminate one-LSB differences **violates the requested density/trim objective, increases memory, and has no demonstrated steady-frame or FPS benefit**. It is therefore rejected despite the positive width discriminator. The opt-in mapping and generated page were removed from repository source before commit; default coverage remains **15/896** PNGs. Other rejected menu/branch/cart families remain dedicated; wrap/repeat/private/mip/target exceptions stay unchanged.

Next bounded design: find a genuinely dense, co-resident layout with binary-exact normalized endpoints (or a source-local sampling path proven equivalent to SDL ordinary raster including source rectangles, flips, rotation, linear filtering and blend quantization), then fresh strict multi-state physical parity, actual steady-frame bind/draw, live memory and repeated performance gate. No 8K/high-Hz, sustained 60-FPS or release acceptance is inferred.

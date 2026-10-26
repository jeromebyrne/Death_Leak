# Static rock atlas pixel gate — rejected (2026-10-25)

This was a distinct next-family probe after opt-in icon, HUD, branch and cart families were already present at HEAD `f30ab4c`. No production source or asset from this probe was accepted. The isolated candidate, scripts, GL traces, logs, captures and rejected generated page remains under ignored `port/build/atlas-rock-props/`.

## Inventory and contract

`Game/Media/objects/rock4.png` and `rock5.png` are each 941×645 RGBA. Across `Game/XmlFiles/levels`, 13 authored references occur in cliff, first ninja arena and grass levels. Each reference is an ordinary `Sprite`, `effectlighttexture`, `repeatX=false`, `repeatY=false`, `isanimated=false`, no authored wobble. Some draws have minification, tint/alpha and rotation. The proposed opt-in virtual source-rectangle mapping reused the existing SDL renderer mechanism and dedicated-PNG fallback for repeat, UV shift or private-shader requests. Painter order and logical sprite calls were unchanged. No repeat/wrap ground, fractional-parallax background, private-GL fog/water/foliage, render target or unexamined object path was admitted.

Physical Xorg `:0`/nouveau NVE7 SDL OpenGL capability probe reported `GL_MAX_TEXTURE_SIZE=16384`. The 4096 atlas ceiling was respected. Live VRAM telemetry was unavailable; the GL allocation byte sums below are only uncompressed RGBA proxies, not calibrated VRAM. The broader image inventory in `PORT_ATLAS_SCALING_GOAL.md` remains 896 PNGs / 451,933,320 source pixels; this probe did not classify every call site.

## Pixel outcome

The existing trimmed deterministic packer emitted 1853×644, two-pixel-extruded page and verified reconstructed visible source texels. A strict physical NVE7 serial A/B/A at fixed-dt frame 180 found 122 changed RGB pixels in `grass_exploration_1` and 194 in `first_ninja_arena` against matching controls, mainly ±1 color channels at rock edges/interiors. This fails the exact output gate; source crop equality is insufficient under minified/rotated filtering.

A second full-original-RGBA page kept hidden alpha-zero RGB and source dimensions untrimmed, with two-pixel extrusion: 1890×649, two sources, each full source crop byte-identical. It reduced but did not eliminate differences: grass candidate versus first control differed at 12 pixels, arena candidate versus both identical controls at 8 pixels, mainly ±1 RGB. Grass trailing control independently changed across much of the frame, consistent with known uptime-clock film variance; therefore grass three-way parity cannot be claimed even apart from the 12 candidate-localized differences. Arena controls were byte-identical. Gameplay endpoints matched within each route. The candidate remains rejected. The second page is ~4.91 MB RGBA, larger than a single 941×645 rock texture (~2.43 MB); it is not an obvious residency win for a one-rock scene.

## Actual GL counts and disposition

Serial apitrace runs of the 180-frame grass route, 120-frame warmup and 60 steady swaps, counted driver-level GL calls rather than renderer logical copies. Control and full-page candidate each submitted **3,576 `glDrawArrays`** (median 60/frame), **420 `glBegin`** (7/frame) and **6,082 `glBindTexture`** (median 103/frame). Texture allocation count was 373 each. The sum of `glTexImage2D` width×height×4 proxies was 708,798,396 control versus 711,277,056 candidate bytes; this is not live VRAM. No draw-submission or bind reduction occurred in this route. Apitrace overhead makes these runs unsuitable for FPS comparison; an uninstrumented performance A/B was not pursued after exact-pixel rejection.

No 60-FPS, 8K/high-refresh, broad atlas, shader or release acceptance follows. Next distinct action: isolate atlas UV/filter precision on a small minified/rotated sprite probe (including GL sampler coordinates), or select an unminified ordinary family with demonstrated on-screen use. Do not repeat trim/full-rectangle packing as though it solved this gate. Existing opt-in gamepad-icon runtime integration remains the accepted first ordinary family (`8c0411d`); this rejected rock slice does not alter it.

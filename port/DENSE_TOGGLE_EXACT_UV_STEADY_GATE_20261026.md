# Dense six-toggle exact-UV atlas discriminator — rejected (2026-10-26)

Base: clean detached pushed feature `f9f0da05ddf487c0054d870c35ffe0b6fdf5b0a6`; sole source/build/physical-NVE7 lane. Primary dirty source was preserved (checkpoint append only). This is a **report-only rejection**: generated page and opt-in runtime mapping were removed before commit.

## Candidate and technical eligibility

The options menu draws one music and one SFX toggle in consecutive ordinary SDL sprite slots. `music_{off,on,on_focus}` and `sfx_{off,on,on_focus}` are six 150×150 source PNGs. There is no repeat, UV shift or private shader on those draw sites. The two `*_off_focus` variants were intentionally left dedicated, because an eight-sprite 2048×154 exact-width page had already been rejected at only 57.07% occupancy and no recurring gain. Existing `LoadTexture` virtual regions preserve source rectangles, tint, rotation, filter choice and painter slot; wrap/shift/private-shader cases continue through dedicated fallbacks.

The deterministic full-RGBA packer placed six sprites with two-pixel extrusion at occupied width 924, height 154; source SHA-256 and reconstructed full RGBA verified. A test-only 1024×154 horizontal canvas made every integer region endpoint a binary-exact normalized fraction while keeping source coverage **135,000/157,696 = 85.61%**. This is substantially denser than the prior 2048-wide candidate, but it still extends **100 columns beyond occupied bounds**; therefore it is not a fully cropped production page under `PORT_ATLAS_SCALING_GOAL.md`. The 4096 ceiling is met. Raw RGBA page proxy is 630,784 bytes versus 540,000 bytes for six dedicated sources, **+90,784 bytes**; the remaining two dedicated off-focus variants would add another 180,000 bytes. These are allocation proxies, not calibrated live VRAM.

## Focused build and exact physical pixels

Isolated production Docker `make -C port -j2` passed (existing compiler warnings only); `atlas_pack.py` exact full-RGBA verification passed after canvas expansion. All physical runs were strict X11/SDL OpenGL Mesa nouveau NVE7, accelerated=1/software=0/vsync=1, 1280×720, fixed 1/60 dt, serial A(off)/B(on)/A(off), full BMP. The default three-update options route `N:1,S:1,.:1` matched byte-for-byte in all arms, SHA-256 `f8f2cc4a1d08c7b43f228771f060876ac6e0a853e5e69c7723a801b2b6d6a235`. Three additional four-update second-focus routes `N:1,S:1,N:1,.:1` used independent both-off, music-off, and SFX-off settings; every candidate matched both controls exactly (SHA-256 respectively `d65aeb1c...`, `43872a57...`, `a1082eef...`). This gates visible selected on/off/focus variants, not partial source rectangles, flips or rotation not exercised by these menu sprites. It does not imply general atlas sampling correctness.

## Real GL calls and disposition

Serial physical apitrace over the same 180-update options route, selecting a mapped and a second eligible toggle, counted **steady frames 121–180**:

- `glBindTexture`: **900 / 900 / 900** A/B/A (15 per frame).
- `glDrawArrays`: **540 / 540 / 540** A/B/A (9 per frame).
- `glTexImage2D`: **0 / 0 / 0** after warmup.

Startup-inclusive counts were binds **2766/2756/2766** and uploads **31/26/31**. Those differences are one-time residency/identity effects, not a measured recurring bind, draw or FPS gain. No calibrated live VRAM telemetry was available. Given zero steady-frame GL-call saving, positive raw allocation cost, two still-dedicated variants and the deliberate 100-column uncropped width, this candidate does **not** justify accepting a default mapping/page. The runtime/page prototype is retained only in `/tmp/deathleak-dense-atlas-evidence/`; default atlas coverage remains **15/896** PNGs (881 unatlased).

Remaining unatlased scope includes both off-focus toggles and the other six rejected toggles, menu buttons, Forest branch/cart and tree-blur families previously rejected on pixel parity, plus ordinary scene sprites not yet classified by co-resident bind transitions. Repeat/wrap, shifted UV, private shader, mipmapped and render-target textures require separate semantics and may remain dedicated. Next precise discriminator: use a matched active-frame GL/SDL draw trace to find a family whose adjacent ordinary draw texture switches actually persist after warmup, then pack to occupied bounds and test normalized-UV exactness without added width. Only after full BMP parity and reduced steady binds/draws should VRAM/timing gates decide acceptance. No 8K, high-Hz, 60-FPS or release acceptance claim.

Evidence: `/tmp/deathleak-dense-atlas-evidence/` (full BMPs/logs, serial traces and filtered GL calls, rejected verified page/manifest/TSV). The primary seven unrelated tracked edits and untracked artifacts were not staged or changed; source/build/physical-GPU ownership is released after this report-only push.

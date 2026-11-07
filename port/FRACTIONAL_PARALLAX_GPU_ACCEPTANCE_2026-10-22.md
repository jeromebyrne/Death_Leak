# Fractional parallax physical-GPU acceptance — 2026-10-22

## Verdict

**Live-path and fallback acceptance passed on the current dirty renderer**, with a bounded same-frame visual discriminator. This is not a matched Windows parity result or a direct GL-state snapshot. No source/test file was edited and no commit was made in this lane.

A forced Docker production build (`make -C port -B -j2 all`) succeeded; log: `port/build/fractional-parallax-gpu-build-2026-10-22.log`. Binary SHA-256: `0614d92ab179dee3ae8939ac47474dcafaad44180195c3b19ba4afadbd097b66`. Build included the pre-existing dirty renderer/Makefile and untracked `port/linux/ParallaxUV.h`; this is *not* fresh-checkout integration proof. Existing TinyXML warnings only.

## Physical GL evidence

Three serial 60-frame captures used the same binary, `--play Game/XmlFiles/levels/forest_exploration_1.xml --script '.:60' --frames 60 --fixed-dt 0.0166667`, Docker `deathleak-linux-build:bookworm`, host Xorg `:0`/LightDM authority, `/dev/dri`, and `SDL_VIDEODRIVER=x11 SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy`. Logs in `port/build/fractional-parallax-gpu-acceptance-2026-10-22/` explicitly report `video_driver=x11 name=opengl flags=14 accelerated=1 software=0 ... mode=strict-accelerated` and `OpenGL device vendor=nouveau renderer=NVE7`. The level loaded 71 original sprites with zero missing/unsupported assets and ended in gameplay at the same player position, `(-43.0146,3206.9)`, health 25. The first float and forced-legacy captures are frame-identical in simulation parameters; wall-clock elapsed differs, but the repeated unforced frame is byte-identical to the first.

At the real SDL bind, texture 224 (forest ID37 `shrubbery.png`) logged logical/allocation `2048x512`, span `1,1`, `npot_repeat=1`, `path=float`; texture 226 (ID40 `trees_top.png`) logged `2048x677`, span `1,1`, `npot_repeat=1`, `path=float`. Thus the NPOT ID40 did **not** require a padded-allocation fallback on this driver. Both draw through the eligible float arm on accelerated physical GL. A third scrolling texture (ID1, `1985x1090`) also took float; this test does not ascribe its scene role.

## Same-frame path discriminator and state continuity

The ignored evidence directory contains `force_span.c/.so`, a temporary `LD_PRELOAD` shim, not a source/test edit. It calls the real `SDL_GL_BindTexture` and changes only the **reported** Y bind span to `0.75` for logical `2048x512` and `2048x677` textures. It leaves the actual texture binding/geometry alone. On the otherwise identical accelerated Xorg run, logs report these two textures as `span=1,0.75 ... path=legacy`; ID1 remains `path=float`. This demonstrates the production eligibility gate takes the integer fallback for nonunit spans without switching to a software renderer. The injected span is an intentionally false return value; it does **not** establish how SDL would allocate a genuinely padded texture.

Lossless 1280x720 SDL readbacks:

- `forest60.bmp`: unforced float, SHA-256 `f368c3748ae49e5c01f38af2167f5f4df0fb7a279f72ba58e4c1da47952cf351`.
- `forest60-repeat.bmp`: repeated float, identical SHA-256 and zero differing pixels.
- `forest60-forced-legacy.bmp`: same-frame forced fallback, SHA-256 `ed16d4a7a37e0709e7a300f55bf24f9fb4c3193028c7690c6d45b98af2e759b6`.

Float versus fallback differs at 111,086 pixels, concentrated in the top 120 rows (45,466) and bottom 120 rows (65,617), where these parallax bands render. The interior `(300,130)-(1000,600)` differs at only **two** pixels of 329,000, while float versus float-repeat differs nowhere. `forest60-float-vs-legacy-diff.png` records the difference. This is a controlled same-frame visual discriminator of live float versus old integer sampling, and strong functional evidence that subsequent ordinary scene rendering survives the direct GL transaction. It is **not** a controlled `cameraX=1000/1000.5` subcut-motion measurement, nor does it measure every GL attribute/sampler before and after. The two interior pixels and any unexamined edge effects should not be overclaimed as perfect state restoration.

## Remaining gate / integration

The exact held-cut camera sequence, flip/seam/alpha comparison against matched original Windows state, explicit GL getter state-restore assertion, and a real SDL bind failure or physically padded texture were not run here. Do not claim exact Windows mip/filter parity from this acceptance. The separate untracked `port/linux/ParallaxUV.h` must be included with the renderer in any coordinated integration commit; a clean clone without it remains unbuildable. Existing unrelated dirty files were preserved. All named Docker capture containers exited; GPU/Xorg/Docker lane released.

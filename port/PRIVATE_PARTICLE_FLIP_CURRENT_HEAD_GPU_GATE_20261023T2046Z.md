# ParticleSpray horizontal-UV flip: physical-GPU Forest gate — 2026-10-23 20:46Z

## Verdict

**Visible in active Forest gameplay; the landed horizontal flip produces a localized, reproducible visual change.** On the fresh fixed-state Forest-1 `R:180` frame, an isolated same-source preflip control versus HEAD `e42ff0c` differs in **4,102 RGB pixels**, bounding rectangle **x=270..681, y=325..637** (half-open `(270,325,682,638)`). The changes cluster at x=256–319/y=320–383 and x=512–703/y=544–639, consistent with small drawn leaves, not a full-scene transform. The current-head repeat capture is byte-identical to the first (SHA-256 `42d10ba7b759ed9bcb15747317fde0f2863189814bdfa90f41f182f773035c9a`). Thus the flip's effect is actually rendered on accelerated hardware in this route. This is **Linux-only visual confirmation**, not Windows pixel parity or a user-delivery clip.

## Reproducible evidence

All artifacts: ignored `port/build/private-particle-flip-gpu-20261023t2046z/`. `current.bmp`, `preflip.bmp`, `repeat.bmp`, `hide47.bmp`; matching `.log` files; `build.log`, `preflip-build.log`; `current-leaf-crop.png`, `preflip-leaf-crop.png`, `delta-leaf-crop.png`. Current executable SHA-256 `6949e0aab86ba6a18ee0edd5d340cecac4ac368fd53933366a366fbcb4710850`; preflip executable `224b6c6ad3f530254fffbf0c8e48b9ca0bcb2cadced1d334da5f9515ed6c181a`. Preflip was built from the **same live source tree** with only an isolated read-only Docker bind-overlaid copy of `port/game/ParticleSpray.h` deleting `leaf.flipX = true;`; no tracked file was changed. The current binary was built first in its unique output path. The tracked code at HEAD has the flip; no later tracked change was incorporated after HEAD.

Capture command shape, run sequentially on exclusive physical Xorg `:0`: `docker run --rm --hostname jerome-Linux-MacBookPro --device /dev/dri` with repo bind `/src`, `/tmp/.X11-unix`, read-only LightDM `:0` Xauthority, `DISPLAY=:0`, `SDL_VIDEODRIVER=x11`, `SDL_RENDER_DRIVER=opengl`, dummy audio, `deathleak-linux-build:bookworm`, then `./port/build/private-particle-flip-gpu-20261023t2046z/{deathleak-linux|deathleak-preflip} --play Game/XmlFiles/levels/forest_exploration_1.xml --script R:180 --frames 180 --fixed-dt 0.0166667 --capture port/build/private-particle-flip-gpu-20261023t2046z/<mode>.bmp`. Every successful log reports strict accelerated X11/OpenGL (`software=0`), **nouveau/NVE7**, GPU Sepia, 71 original sprites and zero missing assets. The endpoints agree: player `(1368.99,3107.3)`, five orbs, health 25, grounded, no combat. `current.bmp` and `repeat.bmp` are byte-identical; `preflip.bmp` SHA-256 `cef59918d0831fdfe1853070606fbed975b2960bc2e2f1cf55f4c1384b303309`.

A render-hide-ID47 control was also captured but **not used for attribution**: it differs broadly from the current frame (581,469 pixels; x=0..1279/y=33..706), beyond a bounded leaf contribution. Its env-gated path therefore does not furnish a clean leaf-only mask here. The isolated header-overlay A/B and exact current repeat are the decisive controls.

## Next visual action

Retain `leaf.flipX = true` as source-backed: original `Game/src/Particles/ParticleSpray.cpp:8-11,185-214` fixes U=1 at world-left and U=0 at world-right, and the current CPU corner spy in `port/tests/particle_spray_flip_cpu_test.cpp` verifies those corners. If human/art fidelity remains contested, use the saved lossless A/B crops for review and acquire a same-state original-Windows Forest-1 leaf draw packet (particle XY/size/age, bound texture, UV, blend/sampler, pre/post pixels); do not infer Windows parity or change the flip from this Linux-only gate.

## Ownership

Before capture, no competing Death Leak game/ffmpeg/GPU container owned `:0`; after capture, none remained. Only this unique report and ignored evidence were written. Existing dirty `port/linux/SDLRenderer.cpp`, shared checkpoint/status files, and all unrelated dirty/untracked files were preserved; no source edit, index operation, commit, push, or user clip delivery.

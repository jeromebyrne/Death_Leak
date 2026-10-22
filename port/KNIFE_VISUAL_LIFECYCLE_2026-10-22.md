# Player knife visual lifecycle — 2026-10-22

## Scoped change

The player-thrown knife now has its own active visual age. Its render size and tint alpha are both `clamp(age / 0.3 s, 0, 1)`, beginning invisible and reaching the authored 84.5 × 25 world-unit size and full alpha at 0.3 s. The erroneous 0.25-second active lifetime was removed: the source's 0.25 s applies after impact/inactivation. This port still immediately removes a shot on terrain or enemy contact, preserving its existing simplified contact behavior rather than adding a new embedded-impact effect. A separate 10-second port-only collection bound keeps unimpeded offscreen knives finite; it is not a claimed source lifetime. NPC projectiles and deflected NPC shots retain their prior visuals and lifetime. Texture, centering, and horizontal flip are unchanged.

## Verification

- The focused `knife_visual_lifecycle_test.cpp` passes in `deathleak-linux-build:bookworm`: zero-age transparent, 0.15 s half-size/half-alpha, 0.30 s full-size/full-alpha, still alive at 0.35 s, bounded eventual collection, and immediate solid-box contact retirement. It inspects the submitted `render::Sprite`, not just internal state.
- Established Docker `make -C port -B -j2` passes. Only pre-existing TinyXML warnings appeared. `git diff --check` passes.
- Before the GPU lane, no competing Death Leak, FFmpeg, or build process was present. A strict physical Xorg `:0`/nouveau NVE7, accelerated OpenGL run captured 10 seconds of held-fire forest-1 active gameplay (`B:300,F:99700`, fixed 1/60 s). The ignored artifact is `port/build/knife-visual-2026-10-22/forest-1-knife-active-10s.mp4`, SHA-256 `981adb17d8829c6d6e607e78c87b0ad3c0418916b6ffd436ae8762887c633376`. H.264 1280×720 at 30 fps, 300 decoded frames / 10.000 s; full `ffmpeg -xerror` decode passed. `game.log` confirms `video_driver=x11`, `name=opengl`, `accelerated=1 software=0`, `vendor=nouveau renderer=NVE7`, forest-1 with 71 original sprites, and two ambience streams.
- Inspected 1/5/9-second stills show loaded active forest gameplay, moving player/HUD and visible thrown knives; no blank/window/menu capture or gross new compositing artifact. These stills do not isolate knife ages. The deterministic test is the evidence for the 0/.15/.30/.35 s visual formula. No frame-matched Windows comparison was performed.

## Limits

This is a source-formula correction in the partial Linux port, not Windows matched-pixel parity or Jerome's visual acceptance. Immediate contact removal remains a known port simplification versus the source's inactive interval. No art/XML, scene-wide blending, shared checkpoint/status file, or unrelated code was changed.

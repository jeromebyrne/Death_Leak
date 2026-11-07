# Private intro burst same-history surface GPU gate — 2026-10-23

## Verdict and boundary

**Causal Linux burst-draw pixel gate passes.** The preceding cross-process burst-on/off whole-frame comparison was confounded by wall-clock film/shader and process-start scene differences. A held-pull camera-shake A/B with matched non-camera state was not feasible in the present capture architecture without a new single-binary switch, detailed trace, and independent shake RNG handling; the same-binary option alone would still compare separate process starts. I therefore used the prior report's narrower render-only diagnostic: at each capture ordinal, the *same frame in the same process* was saved as a normal composite, then the backbuffer was cleared black and only the two intro sword-pull `ParticleSpray::Draw` calls were issued with the same updated particles, draw camera and zoom. There is no intervening update or shader-clock advance. The isolated result is direct evidence that those calls submit visible GPU pixels after the pull; it does not quantify their contribution in the sepia-composited scene, prove shake pixels, or establish matched Windows parity.

## Frozen source, build and physical-GPU route

At preflight the prior burst owner had explicitly released build/Xorg/capture, and no competing Death Leak process was visible. The existing unrelated dirty tree was preserved. A capture-only `--capture-intro-burst-surface` flag was temporarily inserted in `port/linux/main.cpp`, requiring distinct `--capture`, `--fresh-save-intro`, forest-1 `--play`, and rendered mode. It used the already public `LevelScene::IntroSwordPullSprays()` accessor; no emission, fade, RNG, update, camera, shader, production render order, test, or shared status source was changed. The isolated patch is retained at `port/build/private-burst-surface-gpu-1909/diagnostic-only.patch`; the temporary source edits were removed after capture, leaving the prior dirty main.cpp diff intact. This is deliberately not a shipping switch.

Built once via `deathleak-linux-build:bookworm make -e -C port TARGET=/out/deathleak-linux -j2` from read-only `/src`, unique writable `/out`. Image ID `sha256:73cf3ceea9cb72793e400e8d3bc5a029195626a2ff2082d12edfc4d81bfda3e7`; frozen diagnostic binary SHA-256 before and after runs `812427ffb0c3a2b96038146ed68aa3a88108a3f07faf61ea5dd5c3cf05a083b7`. Build log and binary are in the ignored evidence directory. Each N=165,166,170,180,196 was a serial fresh process using `--fresh-save-intro --play Game/XmlFiles/levels/forest_exploration_1.xml --script I:220 --fixed-dt 0.0166667 --frames N --capture /out/composite-N.bmp --capture-intro-burst-surface /out/burst-N.bmp`. Docker used host network/hostname, `/dev/dri`, physical Xorg `DISPLAY=:0`, LightDM Xauthority, X11 SDL OpenGL and dummy audio. No simulate-only, software, offscreen, or camera-script pixel fallback. All runs exited 0. Each log reports `video_driver=x11 name=opengl accelerated=1 software=0 mode=strict-accelerated`, `vendor=nouveau renderer=NVE7`, GPU Sepia initialized, 71 original sprites, 0 missing assets. Pull occurs at loop frame 165, so N=165 is pre-pull and N=166 is the first post-pull capture.

## Pixel measurement (lossless 1280×720 burst-only BMP)

| Frames N | Nonblack pixels | Nonblack bounding box, inclusive | Sum red intensity |
|---:|---:|---|---:|
| 165 | 0 | none | 0 |
| 166 | 1,172 | (602,480)–(656,522) | 7,004 |
| 170 | 2,803 | (593,462)–(657,526) | 48,550 |
| 180 | 7,920 | (538,416)–(657,546) | 240,002 |
| 196 | 15,934 | (458,352)–(660,571) | 427,616 |

Measurement is `max(R,G,B)>0` against the explicit black clear, with a Python/Pillow scan of each unmodified BMP. The N=165 negative control is completely black; N=166 onward shows nonzero red burst pixels expanding from the player area. This is a causal **draw-call visibility** result because the diagnostic surface contains no other scene, projectile, film, HUD, particle, or postprocess draw calls. Source `LevelScene.cpp::EmitIntroSwordPullBurst` creates precisely two finite sprays (20 and 50 particles), and `LevelScene::Draw` draws those sprays at the player slot. The diagnostic reissues exactly those two spray draws after normal-frame save. See `burst-only-contact.png`, individual `burst-N.bmp`, `composite-N.bmp`, and `run-N.log` in the evidence directory. The diagnostic surface is deliberately raw pre-sepia and black-backed; pixel totals must not be interpreted as final-scene burst brightness or a Windows match.

## Focused checks and release

The corrected private burst fade CPU fixture compiled with `-Wall -Wextra -Werror -UNDEBUG` and passed against the actual `LevelScene.cpp`, including `fadeIn=5`, `fadeOut=1`. The diagnostic CLI rejected a surface request without the required paired capture/mode. `git diff --check -- port/linux/main.cpp` passed both before and after restoring the diagnostic hunk. No source or test changes from this task remain; therefore no software commit/push was made. Existing unrelated dirty files, `PORT_WORK_STATUS.md`, `CHECKPOINT_STATUS.md`, and shared `port/build/deathleak-linux` were not edited. Final process check found no Death Leak build/game/capture process. **Exclusive shared Docker build, physical Xorg `:0` / NVE7 GPU, capture, and narrow source-file ownership are explicitly released.**

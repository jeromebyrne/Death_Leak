# `sea_2` final lower-bar implementation and GPU gate — 2026-10-23

## Outcome

**Implemented and verified the bounded Linux final-scene correction.** In `port/linux/main.cpp`, only the lower `finalFrame` draw in the `sea_2` branch changed: construct `Sprite bottomBar{finalFrame, {}, {0,620,1280,100}}`, assign `bottomBar.flipY = true`, and draw it. The upper `{0,0,1280,100}` draw, `finalFrame` guard, film/title order, postprocess, and separate ordinary-gameplay branch are unchanged. No renderer, XML, asset, or shared HUD file was edited.

`port/render/Renderer.h` defaults now give the lower command opaque-white tint, opacity 1, rotation 0, flipX false, flipY true, full texture source, and destination `{0,620,1280,100}`. The old aggregate's trailing `0,true` instead initialized opacity 0 and rotation 1 degree. The upper command remains unflipped with defaults; ordinary gameplay still uses its distinct approximately 33⅓-pixel bands. This matches `Game/XmlFiles/UI/final_scene_hud.xml` lower-bar `vflip=true` and unrotated 1920×150 source geometry mapped to 1280×100. `git diff --check` passed. The current one-row uniform-black `frame_top.png` means rendered pixels alone cannot verify UV reversal; the named field/source inspection is that gate.

## Build and physical-GPU evidence

A focused build of the current working tree passed with `deathleak-linux-build:bookworm` (image `sha256:73cf3ceea9cb72793e400e8d3bc5a029195626a2ff2082d12edfc4d81bfda3e7`), source mounted read-only, `make -e -C port TARGET=/out/deathleak-linux -j2`, and isolated output in ignored `port/build/sea2-final-bar-2026-10-23/`. Binary SHA-256: `b339ce56a5eefb2231756db5488a414ba6d2c8ab0acbfbff044f7164feca85ab`. The unrelated pre-existing dirty `SDLRenderer.cpp` was included in this integrated build, not staged or attributed to this correction. Build warnings were existing TinyXML fallthrough warnings.

Using the same binary on physical Xorg `:0` with `/dev/dri`, direct nouveau/NVE7 OpenGL, strict accelerated SDL (`accelerated=1 software=0`), and GPU Sepia postprocess, `capture.sh` saved lossless 1280×720 BMPs from **active loaded gameplay**, not menu/preview: `--play .../sea_2.xml --script .:8 --frames 8 --fixed-dt 0.016666667` and the same at frame 930. Sea-2 loaded 19 original sprites, zero missing/unsupported assets, and authored ambience. The frame-930 log records final-title reveal at frame 900/time 15.0166 s.

Both sea-2 captures have every pixel black across full width on rows 0–99 and 620–719, with nonblack scene on rows 100 and 619. Thus y=50/670 and the lower top edge at y=620, including x=0/1279 ends, have axis-aligned uninterrupted coverage. At frame 930 the center title-region red-pixel count rises from 0 (frame 8) to 35,200, confirming title remained visible over the scene/film; source order confirms film is before bars/title. Capture SHA-256: early `161cae78e9da85c53a39034048c9a06573aea2f7fadae8053358d84650dbb1b9`; title `ebe3a55b1e34a7294207f426331eca177479c6007dae785af278631a1b0378bb`.

A third active Forest-1 control with the same binary, frame 8, loaded 71 sprites and zero missing assets. Full-width black rows 0–32 and 687–719 remain, while row 33 is nonblack and center row 50 is nonblack; ordinary gameplay did not inherit the sea-2 100-pixel bars. BMP SHA-256: `6f69593cae6c24e778feeb7568c55b0754bdcc8c5dce65efafef1892a4d287ca`.

Reproduction script, build/game logs, three BMPs, and executable are in the ignored output directory above. The capture demonstrates Linux geometric behavior, not a matched original-Windows frame or visual-parity/release acceptance. No pre-patch same-state image was produced; the geometric cause follows from source fields and the post-patch boundary gate.

## Scope and lane release

The pre-existing dirty status, renderer, tests, and numerous unrelated untracked reports were preserved. Only `port/linux/main.cpp` and this report are in this task's scoped software change. The isolated build and all three GPU capture containers exited successfully; final process check found no Death Leak game, capture, or FFmpeg process. **Physical GPU `:0` and build lane are released.**

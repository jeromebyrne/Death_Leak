# HUD meter horizontal source gate — 2026-10-23

Starting HEAD `1377fdb` (vertical owner released `Hud.cpp` and GPU). The checkout contained unrelated dirty work; this slice owned only `port/game/Hud.cpp`, its uniquely named CPU test, this report, and a preflight-exclusive physical GPU capture.

## Source discrepancy and bounded correction

`Game/XmlFiles/UI/game_hud.xml:76-98` gives meter X=-900 and widths 300/200 in the centered 1920×1080 UI. `Game/src/UI/UIMeter.cpp:50-109` draws bar/backing at `initialX + edgeWidth*0.9`, width `(meterLength-edgeWidth*0.8)*progress`; the middle uses `initialX + edgeWidth`, width `meterLength-edgeWidth*0.95`. `Game/Media/UI/meter_edge_2.png` is 4 pixels wide. `Game/src/UI/UIManager.cpp:303-305,366` projects to 1280×720 (scale 2/3), yielding initial X=40, bar/backing X=42.4, middle X=42.6667. For the focus meter the full bar width is 131.2 and middle width 130.8. Previously `Hud.cpp` used bar X=43, focus bar full width=127, middle X=40 and focus middle width=133; the health middle was similarly misplaced/oversized. These are source-backed, visible horizontal geometry errors, not the already-corrected vertical stack.

The bounded patch applies those formulas to both bars, health backing, and both middle/overlay draws. Caps, vertical positions, progress timing, tint, draw order, UV flags, orb and text are unchanged. Initial health XML length 300 and settled length 75 are both covered; the formulas track animated health length without a hard-coded width.

## Gates

- Focused production-HUD renderer spy `port/tests/hud_meter_horizontal_private_cpu_20261023.cpp`: PASS at initial, settled, target-registration, and partial health/focus with white backing. It asserts exact source-projected X/width for both bars, backing and middle. Focus snaps to 0.5 on decrease as authored; health interpolates to 0.8.
- `git diff --check -- port/game/Hud.cpp`: PASS.
- Docker `deathleak-linux-build:bookworm` compiled current tree with read-only `/src` and unique writable output, exit 0. Binary SHA-256 `18770e4878c0f34bcba5fb478eabc78d04d0a7d568f2ca8e1539a734be14fb92`; log `port/build/private-hud-meter-horizontal-20261023t1922z/build.log`. Initial `make OUT=...` probe failed because the Makefile hard-codes `build/deathleak-linux`; corrected by compiling the same source list directly to `/out`.
- Physical GPU preflight found no competing game/x11grab/ffmpeg or GPU Docker job. This exact binary completed 180-frame active Forest-1 gameplay on physical Xorg `:0`, live LightDM authority, `/dev/dri`, SDL X11/OpenGL strict acceleration (`accelerated=1 software=0`; nouveau/NVE7), GPU Sepia, and 71 loaded original sprites. Lossless 1280×720 capture `port/build/private-hud-meter-horizontal-20261023t1922z/forest-active-hud-horizontal.bmp`, SHA-256 `8fbef106580e142985f6000b6b01e14c4c852570e63e8868d8957959f3678496`; `capture.log` alongside. Game/container exited and GPU lane is released.

Build/capture include unrelated pre-existing dirty source: current-tree runtime smoke, not clean-HEAD isolation. No matched Windows-pixel comparison or Jerome visual acceptance is claimed. Focus middle/left-cap V-flip remains a separate UV question outside this horizontal-only gate.

## Ownership / delivery

Only `port/game/Hud.cpp`, the focused test, and this report were written/staged. `SDLRenderer.cpp`, `CHECKPOINT_STATUS.md`, `PORT_WORK_STATUS.md`, and other dirty paths were not edited or staged. HUD source/test and physical GPU ownership explicitly released after this gate.

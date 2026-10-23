# Ordinary gameplay HUD bars — CPU implementation (2026-10-23)

## Scope and authored mapping

`Game/XmlFiles/UI/game_hud.xml:49-65` authors opaque top and bottom `frame_top.png` widgets at `(-960,490,1920,50)` and `(-960,-540,1920,50)`, with vertical flip on the lower widget. The ordinary gameplay branch in `port/linux/main.cpp` now draws both after the film overlay and before `hud.Draw()`. It reuses the existing `finalFrame` texture, white/255 tint, and `AuthoredUI` conversion, yielding floating logical rectangles `(0,0,1280,33⅓)` and `(0,686⅔,1280,33⅓)`. A named lower `Sprite` sets `flipY=true` and leaves `flipX=false`; this avoids the aggregate-initializer field-order trap. Missing `frame_top.png` emits a path-specific load diagnostic. The `sea_2` final-scene branch and its existing 150-unit / 100-logical-pixel bar geometry are untouched. No renderer, HUD, asset, shared status, or GPU/display files were edited.

## Verification

Before editing, `port/linux/main.cpp` was clean, prior shake commit `05cc746` was HEAD, and no competing build/main.cpp process was found. CPU/source assertions passed for postprocess/film/bar/HUD order, lower flip, opaque tint, authored rectangles, and separation from the final-scene branch. `Game/Media/UI/frame_top.png` exists as a 1080×1 RGB PNG. `git diff --check -- port/linux/main.cpp` passed. Isolated Docker build using read-only source mount and `/tmp/death-leak-hud-bars-build` output passed (`deathleak-linux-build:bookworm`, `make -e -C port TARGET=/out/deathleak-linux -j2`); only existing TinyXML warnings appeared. This compiles the current dirty shared tree, not a clean-HEAD or GPU visual acceptance run.

## Outstanding visual gate and ownership release

No GPU/display was used. A future Forest-1 active-frame capture can check opaque black interiors at `(640,10)` and `(640,710)`, surviving scene/film at `(640,34)` and `(640,686)`, and HUD orb/meters/text above bars; a `sea_2` capture can separately confirm the pre-existing 100-pixel final-scene bands. Pixel output cannot prove `flipY` on a one-row black asset, so source/CPU inspection is the flip evidence. Main.cpp and the Docker build lane are explicitly released after this scoped commit; unrelated dirty work remains untouched.

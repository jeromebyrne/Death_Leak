# HUD whole-meter vertical source gate — 2026-10-23

## Source contract and scoped change

Starting HEAD `24370dd`; preceding cap owner had released `port/game/Hud.cpp` and physical GPU. Existing unrelated dirty files were left untouched. `Game/XmlFiles/UI/UI.xml:2` sets 1920×1080, `Game/XmlFiles/UI/game_hud.xml:76-98` positions health/focus at Y-up 460/440, and `Game/src/UI/UIManager.cpp:303-305,366` scales the screen to 1280×720. `Game/src/UI/UIMeter.cpp:50-109,131-150` resets the bar, both caps, and middle to the same bottom-left meter Y, sets every component's height to the native edge height, then applies the screen scale. The edge asset is 19 pixels high. `UISprite.cpp:66-95` forms vertices from that bottom-left plus dimensions. Consequently, in the Linux Y-down viewport each complete stack has height `19*2/3 = 12.6667`; health top `(540-460-19)*2/3 = 40.6667`, focus top `(540-440-19)*2/3 = 54`. There is a 0.6667-pixel gap between projected meter rectangles.

`port/game/Hud.cpp` previously used unrelated Y/height pairs for bars (45/9, 66/9), caps (42/12.6667, 63/12.6667), and overlays (43/13, 64/13). The bounded correction gives each health or focus bar, backing where present, both caps, and overlay the source-projected common top and 12.6667 height. Existing X/width, UV flips, tint, draw order, progress timing, orb, and text were not changed. This does not resolve any unrelated horizontal geometry or texture-sampling mismatch.

## Gates

- Focused production-HUD renderer spy: `port/tests/hud_vertical_stack_private_cpu_20261023.cpp`, compiled with C++17 and `port/game/Hud.cpp`; PASS for initial, settled, partial, and health-backing states. It checks all components' Y/height against the XML/UIMeter projection. The preceding cap test hardcodes the intentionally superseded cap Y positions and is stale for that assertion.
- `git diff --check`: PASS.
- Docker image `deathleak-linux-build:bookworm` built this working tree with read-only source and unique output; exit 0. Binary SHA-256 `775e9047316e62864bbb4475e75a21de514259accb2ee0c5e3250e714d97d6c3`; build log `port/build/private-hud-vertical-stack-20261023t1918z/build.log`.
- Before capture, no competing game/x11grab/ffmpeg process or GPU Docker job appeared. That exact binary completed 180-frame active Forest-1 gameplay on physical Xorg `:0`, using live LightDM authority, `/dev/dri`, SDL X11/OpenGL strict acceleration. Log records `accelerated=1 software=0`, nouveau/NVE7, GPU Sepia, 71 loaded original sprites, and a live player state. Lossless 1280×720 frame SHA-256 `e7565e8f3f3c0077a56c5594cc8f4eb707d740d9034a766245fb550eed87cab0`: `port/build/private-hud-vertical-stack-20261023t1918z/forest-active-hud-vertical-stack.bmp`; capture log alongside. The game/container exited; physical Xorg/GPU lane is released.

The build/capture includes unrelated pre-existing dirty source, so it is a current-tree hardware smoke gate, not a clean-HEAD isolation or matched Windows-pixel comparison. Matched Windows pixels and Jerome's visual review remain open; no visual parity is claimed.

## Ownership / delivery

Only `port/game/Hud.cpp`, the uniquely named focused CPU test, and this report were written/staged. `SDLRenderer.cpp`, shared checkpoint/status files, and all other dirty paths were not edited or staged. HUD source/test and GPU ownership are released after this gate.

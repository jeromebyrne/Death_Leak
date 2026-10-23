# HUD meter cap geometry source/GPU gate — 2026-10-23

## Scope and source finding

At starting HEAD `824702e`, `port/game/Hud.cpp` and physical Xorg/GPU were released by the preceding right-cap owner. The checkout contained unrelated dirty files; none were edited or staged here. `Game/XmlFiles/UI/UI.xml:2` specifies a 1920×1080 base and `Game/XmlFiles/UI/game_hud.xml:76-98` places both meters at x=-900. `UIManager.cpp:303-305,366` and `UIWidget.cpp:147-153` scale those coordinates to the backbuffer; centered 1280-wide projection puts both left-cap origins at x=40. `UIMeter.cpp:86-109,131-150` draws both caps using the native edge texture width/height multiplied by the UI screen scale. `Game/Media/UI/meter_edge_2.png` is 4×19 RGBA, making each cap 8/3×38/3 logical pixels at 1280×720. `UISprite.cpp:66-95` establishes sprite-flip UV semantics.

Bounded correction: health/focus left-cap x 39→40, and all four cap dimensions 4×15→8/3×38/3. The previously corrected right-cap x origins, bar/middle/overlay geometry, tint, and draw order remain unchanged. Current cap y=42/63 also remains unchanged: the source-projected vertical position (health ~40.67, focus 54) conflicts with the port's existing bar/overlay vertical stack. Changing only caps' y would tear each meter apart; a whole-meter vertical geometry task is needed before claiming source-equivalent Y. Focus left-cap V-flip likewise remains outside this placement/size slice. This is a precise X/size correction, not Windows visual parity.

## Verification

`port/tests/hud_meter_cap_geometry_private_cpu_20261023.cpp` compiled with production `Hud.cpp` using C++17 and passed. Renderer spy checks four cap X/Y/width/height values, flip/opacity/tint, initial health length 300, settled length 75, and partial health/focus state. Y assertions intentionally preserve the existing vertical stack. Prior `hud_right_cap_source_gate_private_cpu_20261023.cpp` hardcodes the superseded 4×15 dimensions and is stale for that assertion. `git diff --check` passed.

Docker image `deathleak-linux-build:bookworm` built the current working tree with read-only `/src` and a unique ignored `/out`; exit 0. Binary SHA-256 `125cb2c4b20cda76637b000ad9c79c8c7d066f96`. It includes unrelated dirty source, so the capture is a current-tree test, not a clean-HEAD isolation claim. Build log: `port/build/private-hud-meter-cap-geometry-20261023t1914z/build.log`.

No competing game/x11grab/capture process or Docker GPU job appeared at GPU preflight. A 180-frame active Forest-1 capture of that exact binary on physical Xorg `:0` exited 0 using live LightDM authority, `/dev/dri`, SDL X11/OpenGL strict acceleration. Log reports `accelerated=1 software=0`, nouveau/NVE7, GPU Sepia, 71 original level sprites, 0 missing/unsupported, and a live player at `(1368.99,3107.3)` with health 25/focus 100. Lossless 1280×720 frame `port/build/private-hud-meter-cap-geometry-20261023t1914z/forest-active-hud-meter-cap.bmp`, SHA-256 `04d6997fb583ab93bc6346f9c0f01299e2256d2732a10419104d14a10fd4a04a`; capture log alongside. This is a hardware/runtime smoke gate, not a matched Windows image or Jerome acceptance. Container/game exited and physical Xorg/GPU lane is released.

Only `port/game/Hud.cpp`, the uniquely named CPU test, and this report were written. `CHECKPOINT_STATUS.md` and `PORT_WORK_STATUS.md` were not touched.

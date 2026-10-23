# Private HUD meter trim painter-order implementation — 2026-10-23

## Result

**Bounded source-contract correction implemented and verified on Linux.** In `port/game/Hud.cpp`, both health and focus `meter_overlay.png` submits now follow the left/right `meter_edge_2.png` submits. Health backing, fill, both edges, then translucent middle trim; focus fill, both edges, then middle trim. `Game/src/UI/UIMeter.cpp:48-125` establishes that original sequence. No geometry, UV, tint, opacity, transition, orb, or other rendering field was changed. The unresolved right-cap X discrepancy was not bundled.

## Focused command spy

New `port/tests/hud_meter_trim_order_private_cpu_spy_20261023.cpp` compiles with current `Hud.cpp` and passes. It checks every emitted sprite's exact texture, destination rectangle, zero source rect, tint alpha, opacity, rotation, flips, wrap/UV shifts and effect fields; ordering is asserted at full meters and after partial health/focus progress, including health-loss backing before the red fill. The test catches the existing focus right-edge aggregate values (opacity 0, rotation 1, no flip) and deliberately preserves them rather than smuggling in another correction. `c++ -std=c++17 -O2 -Wall -Wextra -pedantic port/tests/hud_meter_trim_order_private_cpu_spy_20261023.cpp port/game/Hud.cpp -o /tmp/deathleak_hud_meter_trim_order_20261023 && /tmp/deathleak_hud_meter_trim_order_20261023` reported `PASS`. `git diff --check -- port/game/Hud.cpp` passed.

## Docker build and physical-GPU active gameplay

Current-tree Docker image `deathleak-linux-build:bookworm` built `make -e -C port TARGET=/out/deathleak-linux -j2` with `/src` read-only and unique ignored output directory `port/build/private-hud-meter-trim-order-20261023t1905z/`; exit 0, binary SHA-256 `b99fcfca4b22541f35ca0eb634c4b731bb541417a06578ea6be4203399788e25`. The preflight found no competing Death Leak game/FFmpeg/x11grab process or capture container. A fresh 180-frame `--play Game/XmlFiles/levels/forest_exploration_1.xml --script R:180 --fixed-dt 0.0166667` capture on physical Xorg `:0` with `/dev/dri`, LightDM Xauthority, and strict SDL X11/OpenGL exited 0. Log reports accelerated=1/software=0, nouveau/NVE7, GPU Sepia, 71 original Forest sprites and zero missing/unsupported. Endpoint was grounded active Forest gameplay, player `(1368.99,3107.3)`, five orbs, health 25, focus 100. Lossless frame `forest-active-hud-trim.bmp` SHA-256 `206a7c56989bdfd6c97dddea1a917fca208e6271eff5bf0772bc9b41bf7db783` and PNG/logs are in that ignored directory. This is a live integration frame, not an original-Windows matched reference. Existing unrelated dirty renderer/particle code is included in the current-tree build, so a comparison with the prior orb frame is not a causal pixel A/B.

## Boundary, open issue, and release

No original-Windows matched meter frame or Jerome visual acceptance is claimed. The right-cap positional discrepancy remains for a separate source/scaling review. The focus right edge's pre-existing aggregate values are likewise outside this painter-order-only change. Next step: obtain an original same-state HUD reference or separately source-prove the right-cap geometry and focus-cap fields before considering another patch; user visual review remains open.

Only `port/game/Hud.cpp`, the uniquely named focused CPU spy, and this report were written in source control. Existing unrelated dirty tracked/untracked files and the Git index were preserved up to the final scoped commit check. The named Docker build and capture containers exited; final process preflight found no game/capture process. **Exclusive `Hud.cpp` and physical Xorg `:0`/NVE7 lane ownership released.**

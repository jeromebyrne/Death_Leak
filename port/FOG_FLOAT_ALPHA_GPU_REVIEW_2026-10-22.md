# Fog float-alpha physical-GPU review — 2026-10-22

## Result

The current built fog fix renders in active forest-1 gameplay on physical Xorg `:0`, accelerated OpenGL `nouveau / NVE7` (`software=0`). At the deterministic 300th fixed 1/60-s frame, hiding XML fog IDs 119/122 changes **177,863 RGB pixels**, bounded by `[0,218,821,720)`, with mean absolute RGB difference **3.597/255** (maximum channel difference 74). This establishes a visible current-head fog contribution; it does **not** isolate the float-alpha fix's pixel delta against an otherwise identical pre-fix binary.

## Evidence and method

Evidence: `port/build/fog-float-alpha-gpu-2026-10-22/{baseline,hide,only}-300.bmp` and matching `.log` files. Binary SHA-256: `8c6279bb99d87eba42fb3afff3a707fa141c5fbb9708d51142acd9b89cb89a8b` (`port/build/deathleak-linux`, built 15:33 after the fix source timestamps). Capture SHA-256: baseline `032d39c72b82e9dccac311aefacd2956a81247cd83c3b41d49f39da845857989`, hide `f2b4a43ce2cad777e8485f6a20d0aa214a0037551685cee7a63c55bdb8954f09`, only `e6f9348007529664fd98982a0fd7b2dcb30869cda9481ef93ee1bce2b09b5625`.

Before capture, process inspection found only the Xorg server, no competing game/FFmpeg/Docker GPU process. Three sequential fresh Docker runs used the existing built binary, physical `DISPLAY=:0` and LightDM Xauthority, `/dev/dri`, `SDL_VIDEODRIVER=x11`, `SDL_RENDER_DRIVER=opengl`, dummy audio, and `--play Game/XmlFiles/levels/forest_exploration_1.xml --script R:180 --frames 300 --fixed-dt 0.0166667 --capture ...`. The baseline has no ID filter; hide sets `DEATHLEAK_RENDER_HIDE_XML_IDS=119,122`; only sets `DEATHLEAK_RENDER_ONLY_XML_IDS=119,122`. All three 1280×720 captures have identical logged gameplay state: player `(1400.52,3107.54)`, five orbs, grounded, no enemies/projectiles. Each log says `video_driver=x11 name=opengl`, `accelerated=1 software=0 mode=strict-accelerated`, `vendor=nouveau renderer=NVE7`, GPU Sepia, 71 original sprites, zero missing assets, and simulation ended **in forest-1**, not menu. `elapsed` in logs is wall time, not simulation time. No game/Docker process remained afterward.

## Comparator limit and next step

The closest existing same-scene/frame fog capture is `port/build/fog-gpu-review-2026-10-22/only-300.bmp` (and `baseline-300.bmp`, 12:58). Current only-fog versus that old only-fog differs at 233,617 pixels (bbox `[0,44,891,720)`, mean absolute RGB 3.017); full-scene baselines differ at 878,636 pixels (mean absolute RGB 16.763). **These are not controlled before/after float-alpha deltas**: that earlier capture predates intervening PixelWobble and other renderer/gameplay changes, so no causal attribution to the fix is valid. No source or binary was changed to fabricate a pre-fix comparator.

Next visual gate: use an otherwise identical pre-fix binary built from the immediately preceding source state (or a narrow temporary binary-only float-alpha toggle), then recapture the exact frame/ID filters on the same GPU and compare composited pixels; separately seek matched Windows forest-1 state for parity/acceptance. Current capture proves the corrected build executes and fog contributes pixels, not Windows visual parity. No commit or push; shared source/build and unrelated files were preserved.

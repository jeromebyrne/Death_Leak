# Configurable display-resolution vertical slice — 2026-10-25

## Source contract

`--window-size WIDTHxHEIGHT` accepts bounded dimensions 640×360 through 7680×4320; default remains 1280×720. The SDL window is resizable. The scene target, authored world/HUD/UI coordinates, shader orthographic projections and mouse coordinates remain logical 1280×720. SDL's `SDL_RenderWindowToLogical` handles mouse translation. The private-GL scene postprocess now fits the logical 16:9 image into the actual drawable, clearing black letterbox bars. Menu/preview scenes resolve their unprocessed target to the same centered physical viewport before capture/present. Captures of the window use full physical GL framebuffer pixels; mid-scene diagnostic reads intentionally remain 1280×720 target pixels. Atomic packet dimensions/viewport are read from the actual BMP, not assumed 720p.

The source audit found hardcoded 1280/720 uses for authored draw rectangles, repeat/cull widths, private GL orthographic projection, scene target and packet metadata. Authored-coordinate uses remain intentional; launch dimensions and capture metadata were the configuration bugs. The prior menu/preview target-bound capture was logical-only and a private-GL scene pass stretched on non-16:9 outputs; both are corrected here.

## Strict physical evidence

- Xorg `:0`, SDL x11/OpenGL, accelerated=1/software=0, nouveau/NVE7. Serial runs only. SDL reports one display, current 1440×900@60, 36 advertised modes including 1280×720@60. There is no advertised 4K or 8K mode. The window manager clamps a requested 1440×900 window to an actual 1440×828 drawable; this is **not** a 900p window or 8K acceptance.
- The physical SDL mapping probe at actual 1440×828 mapped window center (720,414) to logical (640,360), and top y=0 to logical -8 (outside the scene). A bound scene target gives identity mapping instead; game mouse polling occurs with the window target active, and menu/preview resolve restores that state before the next frame.
- Exact staged-source build (clean HEAD renderer plus these display hunks, excluding the pre-existing dirty raft-mip experiment) ran menu and Forest gameplay at requested 1280×720 and 1440×900. Captures measured 1280×720 and actual 1440×828 respectively. High-window menu content occupies rows 9–818, centered 810-pixel 16:9 projection with black margins. Physical game capture is likewise centered. Staged gameplay decoded RGBA matched the isolated candidate byte-for-byte at both sizes. At 720p, full-GL capture decoded RGBA matched the previous SDL readback exactly.
- The same staged binary ran the fixed-dt Forest→Waterfall-top 425-frame route at requested 1440×900: atomic frame-415 and postfade frame-424 BMPs and JSON image/viewport metadata are all 1440×828, including pre-HUD frame 424. The 639-frame ID22 diagnostic retained 1280×720 pre/post mid-scene BMPs while its final window BMP and JSON report 1440×828. Both routes completed on the strict physical NVE7 GPU.
- Reproduction artifacts: ignored `port/build/performance-display-20261025/` (SDL mode/mapping probes, staged renderer/build, serial logs, captures and packet JSON). No frame-rate gain or sustained high-refresh claim is made.

## Limits and next step

This host has no 4K/8K monitor mode; the CLI and code bounds support requests up to 7680×4320, but actual 8K drawable allocation/presentation is not verified. A true 8K display/GPU gate remains open. The next distinct slice is fixed-step/real-time timing and refresh decoupling; this slice did not alter gameplay update cadence. Higher-mode sustained FPS, transition tails, VRAM/thermal and release acceptance remain open.

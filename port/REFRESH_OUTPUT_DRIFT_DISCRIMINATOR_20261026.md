# Physical 120-Hz output-size drift discriminator (2026-10-26)

**Disposition:** The previous 100-update 1440×810@120 candidate gate remains rejected. This slice attributed the observed 1440×778 capture to a real X11/SDL window/display transition, not to BMP stride, logical projection, or a packet-replay readback coordinate. It did **not** identify the external actor that initiated the transition, establish a reliable mitigation, or enable an independent refresh-slot scheduler. No runtime fix is accepted.

## Setup and controls

- Exact pushed feature base `d985c1e34ab9f781ddc26dcac78fd8c8a4508cf0` in clean detached `/tmp/deathleak-refresh-20261026`; production Docker build passed with existing compiler warnings. Seven unrelated primary tracked dirty paths and untracked files were left untouched.
- Temporary **test-only** `DEATHLEAK_DISPLAY_TRACE` probes in `SDLRenderer::PollQuit` printed SDL window events, window/output sizes and flags; `SaveFrame` printed current display mode. Probe edits were reverted before this report. Physical X11/OpenGL Mesa nouveau NVE7, SDL accelerated=1/software=0/vsync=1 and dummy audio. This is the actual `:0` display through LightDM authority, not Xvfb/offscreen.
- Prior exact-base 100-update 1440×810@120 controls A1/A2/A3 stayed 1440×810, while prior packet candidate 100-update captures (with and without recorder) became 1440×778. Prior 10-update candidate stayed 1440×810. All initially reported active 1440×810@120.

## Causal discriminator

An instrumented optimized packet-base 100-update `I:100` physical run reproduced the bad size. The event order after initial `Fullscreen active=1440x810 refresh=120Hz` was:

1. `SDL_WINDOWEVENT_FOCUS_LOST` (13) while window/output still 1440×810, flags 39 (fullscreen/shown/resizable).
2. Window resize/size-change to **1440×900**, then move to `(0,49)`, then resize/size-change to **1440×778**; output tracked each window size. Flags still 39 during resizing.
3. `SDL_WINDOWEVENT_HIDDEN` (2) and `MINIMIZED` (7), flags 107. At final capture, window and renderer output were both **1440×778**, while `SDL_GetCurrentDisplayMode` reported **1440×900@60**. The capture path failed only because this first temporary probe used an unmounted output directory; the window/mode evidence precedes that failure.

The 32-pixel mismatch is therefore **not** an internal image readback truncation: physical display changed from 810@120 to 900@60 and the fullscreen window was moved/resized to a 778-high work-area-like rectangle after focus loss. The exact actor/trigger of the focus loss and mode switch is unproven; the packet recorder cannot be blamed or exonerated solely from this event sequence. Do not silently recapture/relabel 778 pixels as 810.

A subsequent strict serial physical A(base at `8892db31`)/B(instrumented `d985c1e`)/A(base) 100-update sequence completed 1440×810@120 in all three arms. Full BMP header dimensions were 1440×810 each; captures exist at `/tmp/deathleak-fullframe-base-20261026/port/build/refresh-A{1,2}.bmp` and `/tmp/deathleak-refresh-20261026/port/build/refresh-B.bmp`, logs `/tmp/deathleak-refresh-gate/{A1,B,A2}.log`. Gameplay endpoints matched, but BMP hashes did not all match because the native film clock was unsynchronized (`c59bac...`, `2844f7...`, `189d75...`); this is **not** a pixel-parity proof. It shows the mode drift is intermittent in this physical session, not an inevitable effect of every 100-update packet build. No positive pixel branch or A/B performance gain is claimed for this diagnostic-only slice.

## Next bounded action

Reserve a dedicated physical display session or isolate the display manager/window focus source; log monotonic timestamps for XRandR screen-change events, SDL focus/move/resize/minimize events, active mode, actual output size, and process/window identity. Require stable 1440×810@120 **through each capture/present slot**, not only initialization. Only after that gate is stable, implement opt-in independent refresh slots from the sealed packet and compare exact-base/candidate/base on synchronized film, fixed 60-Hz simulation counts, presentation counts/interval percentiles and gameplay endpoint; repeat must not run simulation, resource loads, captures, or mutable draws. Preserve existing painter/postprocess/HUD/capture order. If display ownership cannot be stabilized, fail closed on mode/output mismatch and keep 120-Hz acceptance blocked.

No claim is made for 120 FPS, GPU-time improvement, 8K output, sustained thermal/VRAM, or release acceptance. The existing single-repeat wall-time measurements are pipeline/CPU+driver observations, not GPU-only timing.

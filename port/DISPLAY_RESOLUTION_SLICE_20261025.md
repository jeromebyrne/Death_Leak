# Explicit output resolution / fullscreen gate — 2026-10-25

## Scoped change

`--window-size WIDTHxHEIGHT` already accepted 640×360 through 7680×4320 while the scene target, projection, HUD and menu coordinates remain authored 1280×720. This slice adds `--fullscreen`: select only an exact resolution advertised by the window's SDL display, prefer a mode closest to current refresh, wait for X11's asynchronous drawable resize, and reject a drawable that does not match. It logs actual output and GL texture/renderbuffer/viewport limits and rejects sizes exceeding them. `SaveFrame` now reports actual capture dimensions, which can differ from the requested window size under the window manager. Windowed mode remains resizable. No refresh-rate selection or gameplay-clock change is claimed.

## Input, HUD and capture audit

`SDLRenderer::Mouse` uses `SDL_RenderWindowToLogical` on the live renderer. SDL's logical-size viewport maps physical mouse positions into the 1280×720 UI coordinate space, including a centered letterbox; positions in bars map outside interactive logical rectangles. World projection and HUD drawing remain at 1280×720. The scene target remains 1280×720; sepia's physical viewport and unprocessed-scene destination both preserve 16:9. Final `SaveFrame` queries live `SDL_GetRendererOutputSize` and reads the full physical OpenGL framebuffer, including black bars, rather than a logical-viewport-shifted crop. A 1440×900 fullscreen menu capture was 1440×900 with black 45-pixel top/bottom bars; at 1280×720 window/fullscreen, full BMP bytes matched. A requested 1440×900 *window* yielded 1440×828 capture because the window manager constrained it; the actual output must be used, not the request.

## Physical NVE7 evidence

Authenticated Xorg `:0` advertised 1440×900@59.90 (current), 1280×720@59.90 and smaller modes; no 7680×4320 mode. Mesa nouveau/NVE7 reports GL_MAX_TEXTURE_SIZE=16384, GL_MAX_RENDERBUFFER_SIZE=16384 and viewport 16384×16384. GL capacity permits an 8K-sized target numerically but this monitor does **not** support 8K fullscreen. `--window-size 7680x4320 --fullscreen` passed syntax validation then failed explicitly with `fullscreen mode 7680x4320 is not advertised by display 0`.

Clean-HEAD isolated build, strict SDL x11/OpenGL accelerated=1/software=0/vsync=1. Serial 1280×720 window/fullscreen/window two-update captures matched full BMP SHA-256 `ff5ca1abf6bb31e6d708ce5f1978a00503b0781ca81c4ccf1bfa3d9ed8793254`. An exact-source rebuild with final capture logging also matched that hash. Serial Forest `B:180`, fixed 1/60-dt, 120 warmup/60 measured updates: all arms ended `(1368.99,3107.3)`, 5 orbs, 25 health, 11 fired. Frame-time p50/p95/p99 ms: window A 21.607/21.9768/22.0599; fullscreen B 21.1922/21.6798/21.9358; window C 21.5414/22.0609/22.4248. This is mode parity and no observed latency regression, **not** a sustained-FPS improvement. Native-clock long-route final-frame exact pixels were not claimed because existing full-screen film is wall-clock-driven; short synchronized captures passed. Captures/logs: ignored `port/build/resolution-20261025/`. Xorg was restored to 1440×900 after tests.

## Disposition

Accept explicit fullscreen mode validation and output-size provenance. Do not claim physical 8K or high-refresh acceptance, representative all-scene performance, or release readiness. Next distinct task: supported-resolution active-gameplay input/HUD/capture exercise at 1440×900 fullscreen, including actual mouse movement/clicks and synchronized post-HUD film capture; then test larger modes only on a display that advertises them.

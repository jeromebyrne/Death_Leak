# High-refresh presentation clock seam — 2026-10-26

Base: clean detached `origin/feature/linux-renderer-foundation` at `56b869ce609941e81e6788e49232bb0914f87473`. Primary seven unrelated dirty tracked paths were preserved.

## Implemented

`FixedStepPacer::PresentationAlpha()` exposes a bounded, read-only fractional position between 60-Hz fixed simulation ticks after due updates drain. It returns zero while update debt remains. `ShaderClock::PresentationSnapshot()` reads authored total/wobble clocks without invoking mutating `Draw()`; extra display slots must not increase shader phase or game speed. Neither API changes the existing fixed-dt update or render path, so this is a timing seam for a future independent presentation loop, **not** a high-refresh renderer.

`port/tests/high_refresh_clock_test.cpp` drives ten synthetic seconds at 30/60/75/120/144/165/240 display slots. Every rate consumes exactly 601 fixed updates including the startup update, no dropped steps; display-only slots respectively 0/0/150/600/840/1050/1800. Repeated snapshot reads leave shader clocks unchanged. The test passed with C++17 `-Wall -Wextra -Werror`. Production Docker build passed from this isolated source, with only existing TinyXML warnings. `git diff --check` passed.

## Limits and next implementation

The live `main.cpp` loop currently draws only after a simulation update and update-debt drain. Thus it still presents at most one new image per 60-Hz simulation quantum; 75–240-Hz slots above are synthetic scheduler evidence, **not** emitted GPU frames. No interpolation of game objects, camera or shaders is wired and no visual, FPS or high-Hz acceptance is claimed. A full extra-frame pass needs update/render separation of the intertwined main loop and a read-only presentation snapshot for scene/animation state. Audio and gameplay updates must remain fixed-tick only. The currently visible Xorg `:0` is protected by a root-only LightDM auth file; sandbox X11 access failed, so physical NVE7 and exact synchronized 60-Hz pixels were not run for this slice. No supported >60-Hz device mode is established. Next owner should make a narrow render-only repeat/interpolation seam, validate exact 60-Hz synchronized pixels, then test on a genuinely advertised high-refresh display before claiming support.

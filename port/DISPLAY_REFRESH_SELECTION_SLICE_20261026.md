# Display refresh selection vertical slice — 2026-10-26

## Scope and source audit

Base: remote `0251d7863e79b80ff6c6270cd846aecb2ae0a5b8` in isolated clean detached worktree. The primary tree's seven unrelated dirty tracked paths and untracked files were not edited or staged. Existing `--window-size` already accepts 640x360..7680x4320, while `--fullscreen` checks advertised geometry, drawable dimensions and GL target/viewport limits. The renderer retains its 1280x720 logical canvas, `SDL_RenderWindowToLogical` mouse mapping and physical-pixel capture. Live gameplay already uses a fixed 60-Hz simulation pacer, but rendering/interpolation at every high-refresh display slot is not implemented.

This slice adds `--refresh-hz HZ` for fullscreen only (integer 24..360). SDL must advertise that exact geometry/rate pair. When absent, the prior closest-to-current-refresh mode choice remains. After the fullscreen drawable settles, the code checks the active mode rate and refuses a mismatch. The new renderer configuration hook is backward-compatible with existing render spies. No painter, authored coordinate, simulation or input code changed.

## Verification

- Isolated production Docker build: `deathleak-linux-build:bookworm` `make -e -C port TARGET=/out/deathleak-linux -j2` passed. `git diff --check` passed.
- Physical Xorg `:0`, SDL x11/OpenGL, Mesa nouveau NVE7, `accelerated=1 software=0 vsync=1`: explicit 1440x900@60 and 1280x720@60 both selected active 60-Hz modes and output matching drawable dimensions. Full physical BMP captures had 1440x900 and 1280x720 headers respectively. The default 1440x900 fullscreen path still selected 60 Hz.
- Explicit 1440x900@120 failed cleanly because no such mode is advertised. `--refresh-hz 60` without `--fullscreen` returned CLI error 2 before opening a window. The desktop display returned to its original 1440x900 mode.
- Diagnostic logs/captures retained locally under ignored `port/build/display-refresh-20261026/` in the isolated worktree; no test instrumentation is shipped.

## Limitations and next gate

The NVE7 panel advertises ~60-Hz modes, not 120/144/240 Hz or 8K. The selected 60-Hz physical tests prove mode selection and capture dimensions, not 8K/high-Hz support, mouse-click delivery (Cinnamon lock-screen overlay has intercepted earlier XTest input), exact synchronized visual parity, or sustained gameplay performance. Because this feature does not alter rendering or gameplay, no latency-gain or pixel A/B/A claim is made. Next work should isolate >60-Hz presentation/interpolation from fixed 60-Hz simulation, audit per-render animation/shader/audio clocks, and test on an actually high-refresh GPU/display; separate real mouse-input verification is still needed.

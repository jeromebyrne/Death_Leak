# Render-only extra-presentation pass: rejected at current seam — 2026-10-26

Base: clean detached remote `64a051e1663f286b51a0284493ab4caaad4138d5`. Primary source's seven unrelated dirty tracked files, especially `port/linux/SDLRenderer.cpp`, were untouched. This is a report-only gate, not a high-refresh implementation.

## Source decision

A display-only slot cannot safely re-enter the current `main.cpp` draw block. The live scheduler at lines 754–774 loops until a fixed update is due, and lines 1623–1628 skip the draw while update debt remains. After updates, its camera/draw/packet block spans lines 1629–2043. It does not have a read-only entry point:

- Lines 1630–1635 consume global `rand()` signs, call mutating `CameraShake::Update`, perturb stored camera coordinates and alter future camera/shake state. `CameraShake.h::Update` may also retire the shake.
- `ShaderClock::Draw` at line 1636 advances pixel-wobble phase; `RenderCompleted(dt)` at line 2043 commits its prior-delta state. The new `PresentationSnapshot()` avoids this clock mutation alone, not the rest of the block.
- `Hud::Draw` at line 1782 calls two `MeterProgress::Update`s, changes progress targets and increments upgrade width. In `Hud.cpp` line 10, the health backing decays by `.005` **per draw**, independent of elapsed time. A repeated draw at the same shader/film clock therefore changes future state and pixels. Focused current-source CPU spy: same-clock extra HUD draw changed backing width `47.6273 -> 47.388` (a `.005` progress decrement scaled by the authored bar width).
- The gameplay/final 11-FPS film selectors at main lines 1755/1770 mutate frame indices and last-switch timestamps using wall clock, and the render branch may lazily load lightning/rain/film textures (lines 1715, 1728, 1767). Re-entering can advance film independently of the fixed update or alter resource side effects.
- Capture/atomic packet logic and `EndFrame` share this block (lines 1826–2026). A second presentation without a new simulation ordinal would duplicate or overwrite ordinal-bound capture/packet work unless extracted separately.

Thus a simple `if (!pacer.Due()) draw()` or a copy of the scene painter is not a read-only presentation pass. Moving only `ShaderClock::Draw` or using `PresentationAlpha()` cannot repair HUD, film, shake, lazy loads, and capture ordinals. No production candidate was accepted; no purported >60-Hz frame emission, pixel parity, or FPS gain is claimed.

## Verification and physical gate

- Clean remote-HEAD Docker production `make -C port -j2`: **pass**; binary SHA-256 `fbad9b35a07349c93d28ab321d3c0c8d60e16a1b7dae60adffc1513ce36706c0`; existing main/TinyXML warnings only. No source diff.
- Existing `high_refresh_clock_test.cpp` compiled with C++17 `-Wall -Wextra -Werror -UNDEBUG`: **pass** at synthetic 30/60/75/120/144/165/240-Hz slots, exactly 601 fixed updates each. This proves scheduler/clock seam only.
- Existing `camera_shake_cpu_test.cpp` compiled likewise: **pass**; explicitly witnesses stored camera perturbation across update samples.
- A focused test-only HUD spy, adapted to the current five-argument `Renderer::Initialize` signature, compiled and passed; repeated `Hud::Draw` at identical process clock changes rendered backing width. The older full `hud_transition_cpu_spy_test.cpp` is stale against the current renderer API/expectations and did **not** pass as-is; it is not counted as a regression pass.
- Physical Xorg `:0` socket exists, but user Xauthority returned `Invalid MIT-MAGIC-COOKIE-1 key`; mounting `/run/lightdm/root/:0` into the build container also returned X11 authorization failure and SDL fell back to `offscreen` (1024×768@0). Hence physical NVE7 synchronized 60-Hz pixels and advertised >60-Hz modes could **not** be measured here. There is no candidate to compare even if access is restored. No arbitrary-Hz, 8K, sustained-60-FPS or release claim.

## Smallest next seam

First separate **one fixed-update preparation** from **one read-only draw submission** for a bounded gameplay state: cache the sampled camera-shake view/sign and shader snapshot; split HUD meter transition/update from a const HUD draw; update the 11-FPS film selector once at the existing authored cadence and pass its selected index; preload/fence weather/film textures; exclude capture/packet effects from display-only passes. Preserve exact original per-update call order at 60 Hz, then prove repeated draw submissions do not mutate state with a draw-spy and compare synchronized full post-HUD 60-Hz pixels on authorized NVE7 before wiring a display-slot loop. Only genuinely advertised hardware >60-Hz modes may then be tested.

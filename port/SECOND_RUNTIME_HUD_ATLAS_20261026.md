# Second runtime atlas family — gameplay HUD, 2026-10-26

Base: clean isolated detached worktree at `c14a0fa8231b57182b3377b555ebefecad1edece`; sole source/build/physical-NVE7 owner. Primary dirty tracked/untracked paths were preserved except append-only performance checkpoint.

## Candidate and disposition

Actual `Hud::Load`/`Hud::Draw` use six ordinary PNGs: `meter_bar_middle`, `meter_bar_middle_back`, `meter_edge_2`, `meter_overlay`, `orb_count`, `xp_meter_bar_middle`. HUD meter draws retain authored source rectangles, tint, flips and painter order through the existing virtual-ID atlas path. The existing opt-in `gameplay_hud` page was regenerated full-RGBA and enabled by default; no separate page was allocated. `DEATHLEAK_ATLAS_HUD=off` preserves dedicated-texture control. Repeat/shift/private-shader draws still fall back to dedicated source textures in `SDLRenderer::DrawSprite`.

The deterministic free-rectangle packer produced one occupied-size 234×132 page (well below the 4096 ceiling) with 2-pixel edge extrusion. All six source hashes and complete reconstructed RGBA bytes, including invisible RGB, verified exactly. Source rectangles total 26,008 pixels (104,032 raw RGBA bytes); page uses 30,888 pixels (123,552 raw RGBA bytes), 84.2% source-rectangle occupancy. This adds about 19.5 KiB raw page area versus the six dedicated rectangles; it is not a VRAM-saving claim.

**Accepted:** the six-sprite gameplay-HUD family maps through the shared page by default, conditional on scoped commit/push verification. No draw-call/FPS gain is claimed: this is an asset identity/residency integration, not a compatible-draw batching change.

**Rejected first candidate:** six main-menu button variants were packed full-RGBA into one 1922×516 page and runtime-enabled in the isolated experiment. Physical fixed-dt menu frame-2 serial A(off)/B(on)/A(off) controls matched (`ff5ca1ab...`), but B (`9b3ca9f3...`) differed by 11 one-LSB RGB pixels within the three visible buttons. Exact-pixel gate failed; menu integration and generated page were removed from candidate source and retained only in ignored evidence. The icon page was not enlarged: menu-only 512-pixel art would impose unnecessary gameplay residency. Further menu work needs a sampling-precision discriminator, not a default switch.

## Gates and measurements

- Isolated Docker native-clock production build passed after all test-only fixture removal; binary SHA-256 `9bb183d17a889d3059713dd369865e67d5e5c9108f260cf5c740a98a8c6b3407`. `git diff --check` passed. Packer re-ran full-RGBA verification after fixture restoration.
- Strict serial physical X11/OpenGL Mesa nouveau NVE7, accelerated=1 software=0 vsync=1, 1280×720 fixed-dt fresh-save Forest-1 frame-2 A(off)/B(on)/A(off) full post-HUD BMPs were byte-identical (SHA-256 `e6bd74f25112f52379068e78f0817dd0457e3fb1e692459f01ef3102be20e89e`, 0 changed pixels). A final fixture-free candidate production-binary capture matched the same hash. This is native-clock evidence for this short route, not a claim about arbitrary later film phases.
- A temporary test-only `DEATHLEAK_TEST_HIDE_HUD` branch suppressed `Hud::Draw` for a physical positive control; it changed 3,452 RGB pixels in x=40..174,y=42..116. The fixture was restored and is absent from the commit. This proves the A/B/A frame contained visible HUD content.
- Renderer `DEATHLEAK_RENDER_PROFILE` counted **114 actual SDL sprite submission calls** in each two-frame control/candidate run. The atlas does not reduce submission count. Direct GL texture-bind counts were not available in this environment (no apitrace); no bind reduction is asserted. Two-frame wall-time copy/present data are startup/noise-dominated, not an FPS benchmark.
- Rejected menu candidate, accepted HUD A/B/A, fixture, final capture, build and submission logs remain under ignored primary `port/build/atlas-second-family-20261026/`.

## Remaining scope and limits

Default atlas coverage is now 15 of 896 media PNGs (nine gamepad icons plus six HUD sprites), leaving **881 without default coverage**. Existing scene-branch/cart-prop/orange-light pages remain opt-in, not counted as accepted default coverage. Menu buttons remain rejected. Other ordinary sprites need call-site sampling, residency and pixel gates; repeat/wrap/UV-shift/private shader assets require dedicated fallback or separate proven virtual-UV/filter behavior. No representative p50/p95/p99 or thermal/VRAM measurement, 8K/high-Hz output, sustained 60-FPS or release acceptance is established by this slice.

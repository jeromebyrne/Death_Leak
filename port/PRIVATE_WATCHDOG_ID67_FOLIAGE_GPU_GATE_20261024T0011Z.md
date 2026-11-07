# Waterfall ID67 rotated foliage UV physical-GPU gate — 2026-10-24 00:11Z

## Verdict

**Visible source-contract defect proven and narrowly corrected on Linux.** On the fixed 425-frame post-snap Waterfall-top gameplay capture, the missing 48-texel U contraction changes **79,605 final-composite pixels** in a bounded mid-screen area, with maximum summed RGB difference 143. The corrected product binary renders pixel-for-pixel identical to the same-binary opt-in UV oracle. This is physical NVE7 evidence of a causal Linux correction, **not** a matched Windows D3D10 comparison or Jerome visual acceptance.

## Preflight and causal A/B

- Inspected latest `PORT_WORK_STATUS.md`, CPU oracle, HEAD `5436320bad9d`, status, and processes. Prior viewport worker had explicitly released Xorg :0; no Death Leak build/capture/game process occupied it. Read-only ID1 audit was left undisturbed. Pre-existing dirty files included `CHECKPOINT_STATUS.md`, `COMPOSITION_DIAGNOSTIC_NOTES.md`, renderer raft experiment, integration and strafe test changes; none were staged or overwritten.
- Frozen baseline executable `port/build/private-rotated-repeatx-20261024t0000z/deathleak-linux`, SHA-256 `025576656d9f560c3aa687490b9f9e3b1a3076f8f3dbdf4fce899d88d9e27c73`. An **ignored** LD_PRELOAD `SDL_RenderGeometry` interposer, source at `port/build/private-id67-gpu-gate-20261024t0011z/id67_uv_shim.c`, selected only the 2048×252 foliage texture with flipped U=1→0. The baseline run passed vertices unchanged; the opt-in `ID67_ORACLE_UV=1` run changed only the two U=1 endpoints to 2000/2048. Both used the same executable, same shim, fresh serial processes, and identical input/state recipe. The shim observed 820 matching draw calls over 425 frames, consistent with two forest foliage draws per pretransition frame and one Waterfall ID67 draw per posttransition frame; it changed all 820 in the opt-in run. Waterfall packet ordinal 8 identifies ID67 as texture 250 at destination (-130.810242,330,1333.33337,168), +4.612°, X flip/repeatX/sway; ID66 is offscreen.
- Exact A/B recipe, from repository root (two serial runs with `ID67_ORACLE_UV=1` set only for the second):

```sh
DISPLAY=:0 XAUTHORITY=/tmp/deathleak-current-xauth LD_PRELOAD="$PWD/port/build/private-id67-gpu-gate-20261024t0011z/id67_uv_shim.so" SDL_AUDIODRIVER=dummy port/build/private-rotated-repeatx-20261024t0000z/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml --script B:1800 --fixed-dt 0.016666667 --frames 425 --capture port/build/private-id67-gpu-gate-20261024t0011z/baseline.bmp
ID67_ORACLE_UV=1 DISPLAY=:0 XAUTHORITY=/tmp/deathleak-current-xauth LD_PRELOAD="$PWD/port/build/private-id67-gpu-gate-20261024t0011z/id67_uv_shim.so" SDL_AUDIODRIVER=dummy port/build/private-rotated-repeatx-20261024t0000z/deathleak-linux --play Game/XmlFiles/levels/forest_exploration_1.xml --script B:1800 --fixed-dt 0.016666667 --frames 425 --capture port/build/private-id67-gpu-gate-20261024t0011z/corrected.bmp
```

- Both runs exited 0; logs report `video_driver=x11 name=opengl accelerated=1 software=0`, Mesa nouveau `NVE7`, the Forest→Waterfall transition at frame 398, and the same final player/orb/health/focus/fired state. Final BMPs are 1280×720. Baseline SHA-256 `9ddb80bdd02462edc6defe220f60d57ef9faf65c2668deec6c48904693f19e17`; opt-in SHA-256 `fb0dbab8558353674e1e26af3ff84c55fe5895558246a66ba269a7599f5a3775`.
- PIL RGB `ImageChops.difference`: 79,605 changed pixels, bounding box x=0..1111, y=320..523; summed absolute channel delta 1,703,274. Strong witness `(259,428)`: baseline `(54,51,45)` versus source-UV `(111,101,81)`, summed delta 143. Entire y=0..319 and y=524..719 regions are pixel-identical controls, including HUD/lower postprocess. The final composite therefore retains a material mid-strip ID67 contribution after later painter draws.

## Source correction and verification

- Original `Game/src/GameObjects/Sprite.cpp:89-96,215-234` writes repeatX U=width/textureWidth independently of rotation and then flips the whole quad. ID67 XML width 2000 vs source texture width 2048 gives screen-left U=.9765625, not 1. `FoliageSway.fx` displaces vertices but preserves UVs. Linux's `DrawFoliageSway` had used full U=1→0 whenever rotation bypassed its zero-angle repeat tiler.
- Narrow edit in `port/linux/SDLRenderer.cpp` changes only the **rotated, repeatX, partial-width foliage** branch to compute `uSpan=(destination.w/(2/3))/textureWidth`, honoring both X-flip directions. Zero-angle tiling, non-repeat, full-width/greater-width and sway geometry/time/alpha/painter order remain unchanged. This handles ID67's 2000/2048 without routing foliage through the ordinary ID17 GL path.
- Docker build: `docker run --rm -v "$PWD":/src -w /src deathleak-linux-build:bookworm make -C port TARGET=build/private-id67-gpu-gate-20261024t0011z/deathleak-linux -j2` exited 0; log in ignored gate directory. Focused `port/tests/foliage_sway_test.cpp` compiled and passed (`PASS: FoliageSway.fx world-space vertex displacement`). New binary SHA-256 `8b9d4ae846b328999799a32e4b4debdd2b7acc4ffe349785bf61ec36f9675324`.
- New product binary ran the same physical NVE7 425-frame recipe without shim and exited 0. Its `product.bmp` SHA-256 is `fb0dbab8558353674e1e26af3ff84c55fe5895558246a66ba269a7599f5a3775`, **byte-for-byte identical** to the frozen-binary opt-in corrected capture. This independently verifies that the narrow source edit implements exactly the tested visual change in this state.

## Scope and limits

Only the ID67 foliage UV hunk was staged for scoped commit; the unrelated raft experiment in the same renderer file and all other existing dirty files remain unstaged. No shared `PORT_WORK_STATUS.md` or `CHECKPOINT_STATUS.md` edit was made. Ignored A/B shim, build, logs and BMPs remain under the unique gate directory. The matched source-built Windows frame and Jerome review remain open; this report does not claim them or overall visual acceptance. No game/build/capture process remained at postflight; the exclusive Xorg :0 GPU/build lane is released.

## Commit / release verification

Scoped software commit **`f7d04b4`** (`Preserve partial repeat-X UV span for rotated foliage sway`) contains only the 13-addition/2-deletion renderer hunk and was pushed successfully to `origin/feature/linux-renderer-foundation` (`5436320..f7d04b4`). The report itself remains a unique untracked handoff; unrelated dirty files and the pre-existing unstaged raft experiment were preserved.

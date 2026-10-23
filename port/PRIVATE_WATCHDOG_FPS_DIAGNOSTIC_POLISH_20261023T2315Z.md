# Port-only FPS diagnostic polish — 2026-10-23 23:15Z

## Source basis

The Windows game loop in `Game/src/program.cpp` updates, renders, and presents without an FPS text draw; `Game/src/UI/UIGameHudScreen.cpp` likewise has no FPS readout. The Linux port previously drew `FPS --` / `FPS N` in the gameplay HUD unconditionally (`port/linux/main.cpp`). This was a port-only diagnostic visible in ordinary gameplay and review captures, not original-game UI parity.

## Scoped change

`port/linux/main.cpp` now accepts `--show-fps`. With no flag, it neither samples/counts for the FPS readout nor draws it. With the flag, the existing wall-clock sampling and gameplay HUD placement remain available. The argument is explicit, so normal `--play` and `--capture` remain clean by default. No renderer, Xorg, or GPU lane was touched.

## Verification

- `git diff --check -- port/linux/main.cpp`: pass.
- Established Docker build image `deathleak-linux-build:bookworm`, `make -C port`: pass. Existing TinyXML warnings only.
- Test-only SDL dummy/software renderer, one-frame fixed-delta forest `--play` capture with and without `--show-fps`: both exit 0 at 1280×720. PIL comparison shows exactly 216 different pixels in the bottom-left region, bounded by x=10..79 and y=693..706, where the FPS glyphs are drawn. This verifies the opt-in draw while the default capture omits it. These are CPU/software captures, not GPU or full-play visual parity evidence.
- An initial dummy render attempt without the required `DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1` was rejected by the port's strict accelerated-renderer gate; the test-only rerun succeeded. No physical GPU/Xorg run was attempted.

## Release lane

Source/build/capture gate passed for this bounded FPS polish. Commit and push only `port/linux/main.cpp` and this report; preserve unrelated dirty files and untracked reports. No clip delivery claim.

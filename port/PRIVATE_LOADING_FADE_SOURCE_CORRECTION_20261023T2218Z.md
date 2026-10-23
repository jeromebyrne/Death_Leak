# Loading-fade source correction — scoped handoff

## Source-backed change

Windows `UIGameLoading::FinishedLoading()` unpauses at the end of the minimum hold (`Game/src/UI/UIGameLoading.cpp:76-103`); its exclusive `if/else if` update does not change alpha on that same frame. Game objects update before UI in `Game/src/Game.cpp:154-255`. The port previously paused gameplay throughout FadeOut and immediately lowered alpha on the hold-exit draw.

`port/linux/main.cpp` now latches loading pause at frame entry. The hold-exit frame remains paused and opaque; the next FadeOut-entry frame advances normal gameplay, weather, scene and player/combat visuals under the splash. FadeOut's clock still advances once per frame and retires only at the existing strict `>0.20f` threshold. Destination installation, scoped Forest-1→Waterfall trigger, and other routes are unchanged. A compile-time-only passive observer was added for the connected regression; release builds omit it.

## Verification

- Final production translation unit: `g++ -std=c++17 -Wall -Wextra -Wpedantic -fsyntax-only -Iport/compat port/linux/main.cpp` passed. Isolated Docker release and observer builds passed; only preexisting TinyXML warnings. Release binary SHA-256 `f346a5db9941752b29aef5bbd64c0333e31b20693a43859b97e3eae56074e9e0`.
- Connected CPU/software observer ran the authored `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:1800 --frames 426 --fixed-dt 0.016666667` path with SDL dummy/software and test-only software allowance, not `--simulate-only`. `python3 port/tests/private_loading_fade_cpu_regression_20261023.py --observer-log port/build/private-loading-fade-correction-20261023t2218z/cpu-observer.log` **passed**. Trigger was frame 398; destination installed exactly once at 411. FadeIn stayed opaque at frame 410 and installed at 411. Hold-exit at 414 remained opaque alpha 255 and all observed gameplay/weather/scene/player updates paused. FadeOut-entry 415 ran all four, with alpha 170. FadeOut continued gameplay through 423 at alpha 0; frame 424 retired the splash while gameplay still ran. The new script can rebuild/run this connected CPU test itself without `--observer-log` (Docker toolchain required).
- Strict physical X11/OpenGL nouveau/NVE7 capture used the isolated release binary, no software fallback, from fresh processes at frames 410, 412, 413, 414, 415, 423. Logs for every frame report `accelerated=1 software=0`, `renderer=NVE7`, trigger 398 and Waterfall entry by 411. Decoded 1280×720 BMPs at 410/412/413/414 are entirely black and byte-identical; 415 is partially revealed (mean RGB 25.75/23.00/18.02), 423 is visible Waterfall (43.54/40.00/33.58). A physical observer run independently passed the same event assertions, with retirement at 424. Evidence and logs are under ignored `port/build/private-loading-fade-correction-20261023t2218z/`.

This proves the Linux production path's source-contract correction and physical fade presentation, **not** matched original-Windows pixel parity or Jerome's visual acceptance. Existing unrelated dirty renderer/integration/checkpoint files were not edited or staged.

## Ownership release

Scoped source/test/report commit only; no shared checkpoint edit. No game, recorder, or build/capture container remains running after this gate. **`port/linux/main.cpp` and physical GPU/display/build lane released.**

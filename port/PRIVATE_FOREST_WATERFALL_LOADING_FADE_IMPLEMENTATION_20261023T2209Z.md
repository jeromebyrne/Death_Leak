# Forest→Waterfall loading fade — scoped implementation handoff

Source: `PRIVATE_SCENE_TRANSITION_VISUAL_SOURCE_AUDIT_20261023T2207Z.md`, `Game/src/UI/UIGameLoading.cpp:40-135`, and `Game/XmlFiles/UI/gameloading.xml:1-9`.

## Implemented

- Only the `forest_exploration_1.xml` → `waterfall_top.xml` LevelTrigger now starts deferred loading. The outgoing Forest remains until a 0.20 s fade-in reaches opaque; only then does the existing scene replacement/reset occur.
- A source-clock 0.05 s minimum loading branch precedes the fade-out *update*. As in `UIGameLoading`, fade-out time starts when the level loads, so the first fade-out update incorporates the elapsed hold, and the splash is removed after 0.20 s total from load. Gameplay and scene visual updates pause during the loading state.
- The black splash draws over scene postprocess, film, HUD, modal/death UI, and FPS, matching the original UIManager-after-world draw order. The already-loaded `blank_dark.png` is used; a black `FillRect` fallback covers a texture-load failure.

## Asset and geometry evidence

`Game/Media/UI/blank_dark.png` is an RGB 2×2 image with all four pixels `(0,0,0)`. The authored rectangle is `(-1024,-540,2048,2048)` in the centered 1920×1080 UI space. Under `AuthoredUI`'s 2/3 mapping it spans x≈−42.67..1322.67 and y≈−645.33..720, fully covering the 1280×720 logical viewport. Because the source texture is uniform black, viewport-sized drawing yields the same covered region without relying on offscreen clipping.

## Verification and limits

- `g++ -std=c++17 -Wall -Wextra -Wpedantic -fsyntax-only -Iport/compat port/linux/main.cpp` passed.
- `git diff --check -- port/linux/main.cpp` passed.
- `g++ -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iport/compat -c port/linux/main.cpp -o /tmp/deathleak_forest_waterfall_loading_fade_main_20261023.o` passed; this compiled the complete translation unit without touching the shared build directory.
- No physical GPU/display, Docker, shared build output, or integration script was touched. The sandbox lacks `sdl2.pc` and `libpng.pc`, so a complete SDL-linked binary and presented-frame test were unavailable here. The first attempted syntax command emitted pkg-config missing-package diagnostics but C++ syntax checking itself succeeded; the clean direct syntax command above then passed.
- No matched Windows capture or Jerome acceptance was performed. This is source-backed transition logic, **not a visual-parity claim**. Follow-up needs a controlled last-Forest / opaque-boundary / first-Waterfall capture.

## Ownership release

I touched only `port/linux/main.cpp` and this unique handoff report. `SDLRenderer.cpp`, `integration.sh`, `CHECKPOINT_STATUS.md`, `PORT_WORK_STATUS.md`, other dirty files, physical GPU/display, and Docker remain untouched. **The main.cpp file and GPU/display lanes are explicitly released.**

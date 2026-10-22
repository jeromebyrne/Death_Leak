# Conditional lightning-layer checkpoint — 2026-10-22

## Source basis and implementation

Windows `WeatherManager.cpp` uses a strict `mElapsedTime > 180` condition before the first rain session, updates state before adding `delta`, and creates a zero-alpha `Media\\lightning_layer.png` `ParallaxLayer` at `kWeatherForeground` (20). `ParallaxLayer::Update` replaces both XY with camera XY; the flash starts on the first subsequent rain update, then alpha becomes `1 - elapsed_since_flash / 0.4` on later updates. Sessions last 70–120 s plus a 3 s outro; later flashes are 10–30 s apart. `LevelProperties.cpp` reads `weather_properties allow_weather` and the game-object manager advances weather only while unpaused. The source comment saying “20 seconds” contradicts the 180-second constant.

The Linux port now parses the per-level weather flag, retains a gameplay-level weather clock across scene transitions, pauses it on disabled levels and paper/upgrade modal pauses, and resets on a new menu game. It loads the shipped texture lazily when a visible flash first needs drawing. The 2048 × 2048 world-unit camera-centered quad uses the existing 2/3 world-to-logical scale, source alpha, plain-sprite vertical UV orientation, and a depth-20 dynamic painter item before postprocess. No rain particles or audio were added. Random centisecond ranges match source, but use an independent fixed-seed LCG for deterministic port checks rather than the Windows process-global `rand()` sequence.

## Verification

- `docker run --rm -u "$(id -u):$(id -g)" -v "$PWD:/src" -w /src deathleak-linux-build:bookworm make -C port -j2` — passed (only pre-existing TinyXML warnings).
- In the same image, `g++ -std=c++17 -Wall -Wextra -Wpedantic port/tests/weather_lightning_test.cpp -o port/build/weather_lightning_test && port/build/weather_lightning_test` — passed. Checks weather-disabled clock, strict first-rain onset, first-flash scheduling, mid-flash alpha, camera translation invariance / 2048 world-unit quad, flash disappearance, weather pause, and rain stop / zero alpha.
- Two-frame forest gameplay smoke with Docker `SDL_VIDEODRIVER=dummy`, `SDL_RENDER_DRIVER=software`, `DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1`, `--fixed-dt 0.05 --capture port/build/lightning-preweather-smoke.bmp` — exited 0, loaded 71 source sprites / 0 missing assets. It is pre-weather software-renderer evidence only.
- `git diff --check` — passed for the working tree.

## Limits

No matched Windows weather capture, synchronized weather RNG, 180-second rendered flash capture, or physical-GPU visual judgment was performed. The 180-second delay makes the existing short forest visual captures unaffected; a long physical run was not worthwhile for this bounded timing/alpha correction without a matched Windows frame. The Linux renderer's existing source-alpha/postprocess path is reused; exact Windows `effectlighttexture` pixel parity remains unmeasured. Rain particles, rain/thunder audio, foliage multiplier, snow, and broader weather behavior remain outside this change. A rare original later-session edge case can defer a first flash one extra update if a previous session ended while its flash state remained active; the bounded model resets that state on rain stop.

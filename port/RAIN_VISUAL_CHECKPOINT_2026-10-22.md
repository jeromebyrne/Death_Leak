# Rain particle visual checkpoint — 2026-10-22

## Source and scope

`Game/src/GameObjects/WeatherManager.cpp:145-177` creates a 300-particle `Media\\rain_drop.png` directed spray at depth `kWeatherForeground` (20): direction (-0.2,-0.8), speed 26–32, lifetime .7–.85 s, size 28–40, gravity -1, brightness .9–1, lifetime size factor .9, fade-in .15 and fade-out .8. It attaches to camera offset (0,+700), with signed spawn spreads X=1400 and Y=400 (the emitter helper multiplies the given 140/40 by ten). Source `ParticleSpray.cpp` begins the non-prewarmed spray at the offscreen placeholder (99999,9999), then respawns expired particles around the current camera using their creation-time signed offsets. `StopRaining` disables looping, allowing particles to expire rather than erasing them immediately. No particle-opacity rain intro/outro exists; the intro/outro fades the audio object.

Added a weather-only spray adapter `port/game/WeatherRain.h`, using the existing particle renderer and source-authored parameters. Its camera attachment, initial offscreen delay, respawn, finite drain, level-weather enable/pause and depth-20 painter integration are active during the existing `WeatherLightning` rain sessions. Source creates lightning first, then rain; same-depth painter order is now lightning followed by rain. No rain/thunder audio or other weather behavior is claimed.

## Verification

- `port/tests/weather_rain_test.cpp` compiled with `g++ -std=c++17 -Wall -Wextra -Werror` and passed: no rain while dry, 300 source-parameter particles/offscreen startup, all respawn by 1.1 s around a moved camera, and finite drain after stop.
- Final edited tree: Docker `deathleak-linux-build:bookworm make -C port -j2` passed. Build log: ignored `port/build/weather-rain-final-build.log`; only existing TinyXML warnings.
- Exclusive physical Xorg `:0` run, after checking no competing game/FFmpeg/build process: forest-1 idle `.:3680`, 3,680 frames at fixed .05 s, captured at 184 simulation seconds (rain begins after strict >180 s, allowing startup particles to enter). Log reports `SDL video_driver=x11 name=opengl`, strict `accelerated=1 software=0`, physical nouveau/NVE7, GPU Sepia postprocess, 71 loaded level sprites/zero missing assets, and alive grounded player at the forest spawn. BMP: ignored `port/build/weather-rain-gpu-2026-10-22/rain.bmp`, SHA-256 `87251ccb48e09de531d9ea5681b959c30fc3c1c287fb9cd342ff3a8105eb029e` (1280×720); log in the same directory. That capture preceded only the final same-depth lightning/rain painter-order adjustment; a final Docker rebuild passed, but no post-adjustment GPU frame was taken. The captured frame is a valid physical-GPU weather-period rendering exercise, not isolated pixel attribution of rain.
- `git diff --check` passed.

## Limits and next slice

This is **not** matched Windows pixel parity or visual acceptance. The port uses a private reproducible RNG, not the source's process-global Windows `rand()` sequence, so streak positions are not frame-identical. The existing generic sprite-based particle path approximates source `Particle.fx` brightness/fade blending; direct Windows rain screenshots and an isolated same-frame rain-on/off GPU comparison would be needed for a visual parity verdict. The next bounded weather slice is source-authored rain/thunder audio and its 5-second intro/3-second outro volume behavior, with a real audio-device check; do not infer audio fidelity from this visual capture.

# Health Devil voice/reward regression gate — 2026-10-23 21:06Z

## Result

PASS, bounded CPU/SDL-dummy gate for the saturated ordinary-SFX -> tracked Health Devil voice -> reward predicate lifecycle. The new assertion tracks a reward-spawn state: 16 ordinary one-shots fill the pool, priority dialog admission returns a nonzero handle while holding 16 active slots, 32 more ordinary SFX attempts cannot displace the dialog, and reward remains unspawned before and after an initial mixer callback. Only after the actual dialog sample drains does the reward gate spawn. Unsaturated admission and deliberate SFX mute remain passing controls.

## Exact verification

From the repository root, using the existing `deathleak-linux-build:bookworm` image and read-only source mount:

```sh
docker run --rm -v "$PWD":/src:ro -w /src deathleak-linux-build:bookworm sh -c 'g++ -O2 -g -std=c++17 -Wall -Wextra -Wpedantic -UNDEBUG $(pkg-config --cflags sdl2) port/tests/health_devil_voice_capacity_cpu_test.cpp -o /tmp/health_devil_voice_capacity_cpu_test $(pkg-config --libs sdl2) && SDL_AUDIODRIVER=dummy /tmp/health_devil_voice_capacity_cpu_test'
```

Output: `unsaturated=pass`, `saturated_and_reward_boundary=pass`, `sfx_muted=pass` (exit 0). The fixture compiles production `SDLSoundPlayer.cpp`, pauses SDL dummy output, and drives its real callback.

Separately, a fresh `g++ -O2 -g -std=c++17 -Wall -Wextra -Wpedantic` app build was made entirely in `/tmp` inside the same Docker image, from the current source including `main.cpp`, SDL backend, renderer, game sources and TinyXML. The existing authored fixture route ran with `SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1`, `--play port/tests/fixtures/health_devil_update_zone.xml --script I:4,.:800 --fixed-dt 0.0166667 --simulate-only`. It emitted dialog at frame 2 and reward at frame 749, count 1, with max health still 25 because the pickup was not collected. This verifies the app's authored interaction/reward path separately; `--simulate-only` uses the authored-duration branch, not live SDL playback.

## Scope/limitation

The deterministic saturated gate mirrors the exact `main.cpp` live reward predicate and updateability/zone conditions in a small test state object; it does not inject SFX saturation into a live rendered gameplay process. The two gates together cover the production mixer boundary and the authored app reward path, but are not physical-speaker audibility or a single-process saturated gameplay proof. No GPU/Xorg lane used. No renderer, `integration.sh`, strafe test, or shared checkpoint touched. Unrelated dirty files were preserved.

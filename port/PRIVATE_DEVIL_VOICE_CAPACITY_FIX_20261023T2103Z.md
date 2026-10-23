# Health Devil voice capacity fix — 2026-10-23 21:03Z

## Source contract and change

Risk from `PRIVATE_POST_VOCAL_AUDIO_RISK_GATE_20261023T2102Z.md`: `SDLSoundPlayer::PlayDevilVoice` returned handle 0 at 16 active ordinary SFX voices, and `VoiceFinished(0)` immediately returned true. The live `port/linux/main.cpp` dialog update uses `VoiceFinished(dialog.voice)` to release the Health Devil reward; the original `Game/src/GameObjects/HealthDevil.cpp` only rewards after voice completion or deliberate SFX disable.

Committed code `3fa0f14` makes dialog voice admission priority: when the 16-slot pool is full, evict the oldest ordinary one-shot (`id == 0`) and insert the story voice with a nonzero tracked handle. If all slots are already story voices, admit another tracked story voice rather than reporting a false completion. Ordinary one-shots still observe the 16-slot limit, and muted/unavailable audio still returns 0. Handle 0 is skipped on integer wrap.

## Deterministic verification

Used the pre-existing `deathleak-linux-build:bookworm` image (no Docker image build), read-only source mount, and an isolated `/tmp/health_devil_voice_capacity_cpu_test` output inside the ephemeral container. Exact command:

```sh
docker run --rm -v "$PWD":/src:ro -w /src deathleak-linux-build:bookworm sh -c 'g++ -O2 -g -std=c++17 -Wall -Wextra -Wpedantic -UNDEBUG $(pkg-config --cflags sdl2) port/tests/health_devil_voice_capacity_cpu_test.cpp -o /tmp/health_devil_voice_capacity_cpu_test $(pkg-config --libs sdl2) && SDL_AUDIODRIVER=dummy /tmp/health_devil_voice_capacity_cpu_test'
```

Result: `unsaturated=pass`, `saturated_and_reward_boundary=pass`, `sfx_muted=pass`. Fixture pauses the dummy audio device, fills exactly 16 ordinary `FinalTitle` voices, then drives the production mixer callback on the test thread. It asserts a nonzero dialog handle, no early `VoiceFinished`, continued false after one callback, and true only after draining the dialog sample. The reward-boundary assertion applies the live main.cpp predicate (`VoiceFinished`) to that handle. It also checks normal admission/completion and deliberate mute immediate completion. The first compile attempt exposed a test invocation error (concrete `Play` requires the variation argument); fixed before the passing run.

Production `SDLSoundPlayer.cpp` compiled independently with the same C++17 warning flags and SDL headers: `production_object_compile=pass`. `git diff --cached --check` passed before commit.

## Scope and limitations

Only `port/linux/SDLSoundPlayer.cpp` and the new `port/tests/health_devil_voice_capacity_cpu_test.cpp` are in code commit `3fa0f14`. The test-only pause/advance methods are compiled solely under `DEATHLEAK_AUDIO_TESTING`; production behavior uses the same callback. No SDLRenderer, integration.sh, GPU/Xorg, ID37, or other dirty paths were touched. This is CPU/SDL-dummy verification of admission and the live reward predicate, not an end-to-end gameplay interaction, physical-speaker audibility, or a measurement of how often 16 concurrent SFX occurs. Existing unrelated working-tree edits were preserved.

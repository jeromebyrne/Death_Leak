# SDL disk-audio callback mix test — 2026-10-22

## Scope and result

**Pass for the bounded callback question:** the installed SDL2 audio backend enumerates `disk`; the unmodified `port/build/deathleak-linux` opened it and wrote nonzero 48 kHz, stereo, signed-16-bit PCM during two equal-frame, near-equal-wall-time authored forest runs. After aligning the same ambient PCM by its first active callback, the knife-fire run contains a short extra transient at onset, while the following 1.2 seconds are sample-identical to idle. This is stronger than cue-dispatch logging: authored sound reached SDL's actual audio callback and its disk sink. It is **not** evidence of speaker output, perceived quality, physical input, or human acceptance.

## Method

- Repository HEAD at test: `50ba9e9`; executable `port/build/deathleak-linux` (already present, no rebuild). Unrelated worktree changes were left untouched.
- SDL2 `/lib/x86_64-linux-gnu/libSDL2-2.0.so.0` enumerated `['pulseaudio', 'alsa', 'sndio', 'pipewire', 'dsp', 'disk', 'dummy']` through `SDL_GetNumAudioDrivers`/`SDL_GetAudioDriver`.
- Both runs used `SDL_VIDEODRIVER=dummy`, `SDL_RENDER_DRIVER=software`, `DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1`, `SDL_AUDIODRIVER=disk`, distinct `SDL_DISKAUDIOFILE` paths, and unset `DISPLAY`, `XAUTHORITY`, `WAYLAND_DISPLAY`, `DRI_PRIME`. No Xorg `:0` or `/dev/dri` was used. Isolated [settings](build/disk-audio-mix-2026-10-22/settings.xml) enabled SFX. Command (vary `SCRIPT` and `PCM`):

  ```sh
  env -u DISPLAY -u XAUTHORITY -u WAYLAND_DISPLAY -u DRI_PRIME \
    SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software \
    DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1 SDL_AUDIODRIVER=disk \
    SDL_DISKAUDIOFILE="$PCM" port/build/deathleak-linux \
    --play Game/XmlFiles/levels/forest_exploration_1.xml \
    --script "$SCRIPT" --frames 30 --fixed-dt 0.0166667 \
    --settings-file port/build/disk-audio-mix-2026-10-22/settings.xml
  ```

- `SCRIPT=.:30` for idle and `SCRIPT=F:30` for knife fire. `--simulate-only` was deliberately absent, because it disables audio initialization. Both runs completed normally, and logs show SDL disk driver, two streamed WAV ambience voices, authored forest level, and no `Audio unavailable`. Fire ended `fired=3 shots=3`, idle `fired=0 shots=0`. The frame count and fixed gameplay dt are equal; actual callback opportunity is measured by monotonic wall time and PCM length, not inferred from simulation time.

| Run | Wall time (s) | Raw PCM bytes | PCM duration (s) | Nonzero 16-bit samples | Log |
| --- | ---: | ---: | ---: | ---: | --- |
| Idle | 17.491 | 3,301,376 | 17.195 | 1,359,311 | [idle.log](build/disk-audio-mix-2026-10-22/idle.log) |
| Knife fire | 17.337 | 3,272,704 | 17.045 | 1,343,268 | [fire.log](build/disk-audio-mix-2026-10-22/fire.log) |
| Knife, SFX disabled (control) | 17.202 | 3,248,128 | 16.917 | 0 | [fire-muted.log](build/disk-audio-mix-2026-10-22/fire-muted.log) |

Idle and fire differ in measured wall time by 0.155 s (0.9% of their mean); PCM duration differs by 0.149 s. The files named `.wav` by SDL's disk-driver convention are **headerless raw PCM**, not RIFF/WAV: interpret as little-endian signed 16-bit interleaved stereo at 48,000 frames/s (4 bytes/frame). This matches `SDLSoundPlayer.cpp`'s exact requested audio spec and the 4,096-byte callback increments. The muted control used [settings-muted.xml](build/disk-audio-mix-2026-10-22/settings-muted.xml), otherwise the same command. Source inspection shows `SetSfxEnabled(false)` also gates ambience in the callback (`SDLSoundPlayer.cpp`, callback near line 271), explaining its all-zero PCM; it is not an ambience-preserving knife-only subtraction control.

## PCM comparison

The first nonzero samples occur at sample offsets 266,246 (idle) and 270,338 (fire); rounded down to 1,024-frame callback boundaries these are 266,240 and 270,336 interleaved samples. Aligning those boundaries shifts fire by exactly 4,096 samples (two callbacks). The first 0.1 s after onset has RMS **104.61** counts in fire versus **51.58** in idle; direct waveform subtraction has RMS **90.06**, peak absolute difference **440**, and 9,470/9,600 differing samples. At 0.1–0.2 s, subtraction RMS is **15.66**, peak **80**, 3,741/9,600 samples different. At 0.2–1.4 s, **all 115,200 samples match exactly**, with difference RMS and peak both zero. This short, decaying excess at the first shot, followed by an exact ambient-only control span, supports attribution to the knife cue rather than ordinary ambience or run timing. Later shots are not individually attributed from the pair because ambient stream timing diverges beyond 1.4 s; the game's `fired=3` log alone is not waveform proof for each one.

PCM SHA-256: idle `776d4b70fc773804402eb8ffde777e4cb0f687887cbb487868a7e28f7d4a99b2`; fire `3a0fabf6c98277fd495725fddae677e7cb307c0ab17b1005f9c27ff9224fbce9`; muted `67e1a80e12a303d1b0f5b098dcb077cafd7d31b272251dc62afe6a719c45673c`. All PCM and logs are under ignored [evidence directory](build/disk-audio-mix-2026-10-22/). [runs.json](build/disk-audio-mix-2026-10-22/runs.json) records commands, return codes, and measured times for the paired runs. A 120-frame idle pilot is also retained there but is not part of the matched pair.

## Boundary and next action

This closes only the **non-GPU callback-mix** uncertainty in `INPUT_AUDIO_GATE_AUDIT_2026-10-22.md`. The next bounded audio gate is a human listener check on a known-working physical output device, playing the authored forest idle and knife fire with SFX on, checking audibility, transient quality, and stereo placement; record the device/route and listener observation. The absent authored level MP3 score remains a separate source-asset parity gap. No speaker or human acceptance is claimed here, and no source or checkpoint file was changed.

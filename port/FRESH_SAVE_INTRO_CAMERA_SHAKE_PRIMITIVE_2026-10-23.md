# Fresh-save intro camera-shake primitive — 2026-10-23

## Bounded change

- `port/game/CameraShake.h`: added `Start(intensity, duration, totalTime)`. It captures requested intensity/duration/start only when inactive; an active shake refuses another request without changing its running parameters. `Update` and `WillJitter` now use those captured parameters, preserving view-before-stored-position jitter, follow-axis gates, injected signs, time modifier, and expiry at `remaining <= 0`.
- `StartMedium(totalTime)` remains the landing entry point and delegates to `Start(15.f, 0.30f, totalTime)`, preserving its prior parameters and refusal behavior.
- `port/tests/fresh_save_intro_camera_shake_cpu_test.cpp`: new private deterministic CPU fixture for a 0.1-second intro window, active-window refusal and retained intensity, half-window decay, exact expiry, next-window stronger intensity, axis gates, reset, and the landing wrapper.

The source audit's intro caller wiring is **not** part of this ownership slice. This change alone does not introduce visible held-pull shake; the integration owner must request `Start((introHeldTime / 2.75f) * 5.f, 0.1f, cameraShakeTime - dt)` only on sub-threshold held frames as described in `FRESH_SAVE_INTRO_CAMERA_SOURCE_AUDIT_2026-10-23.md`.

## Verification

- `c++ -std=c++17 -Wall -Wextra -pedantic port/tests/fresh_save_intro_camera_shake_cpu_test.cpp -o /tmp/fresh_save_intro_camera_shake_cpu_test && /tmp/fresh_save_intro_camera_shake_cpu_test` — pass.
- `c++ -std=c++17 -Wall -Wextra -pedantic port/tests/camera_shake_cpu_test.cpp -o /tmp/death_leak_existing_camera_shake_cpu_test && /tmp/death_leak_existing_camera_shake_cpu_test` — pass; existing landing behavior remains intact.
- `git diff --check -- port/game/CameraShake.h` — pass.

No shared Docker build, GPU/Xorg run, commit, or push was performed. Unrelated dirty files were left untouched.

**Ownership released:** `port/game/CameraShake.h`, the new private CPU fixture, and this report. Shared integration and visual validation remain with their owner.

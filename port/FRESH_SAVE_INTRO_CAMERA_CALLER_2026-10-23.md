# Fresh-save held-pull camera-shake caller — 2026-10-23

## Change and source correspondence

`port/linux/main.cpp` now requests `cameraShake.Start((introHeldTime / 2.75f) * 5.f, 0.1f, cameraShakeTime - dt)` after incrementing held time, only in the `else` of the strict `introHeldTime > 2.75f` branch while the looped intro phase and Interact hold are active. The request precedes the existing zoom update. This matches `Player::UpdateIsPullingSwordFromStomach`'s sub-threshold `DoShake(time * 5.0f, 0.1f)` request; `CameraShake::Start` preserves active-window refusal. The `cameraShakeTime - dt` offset matches the existing draw-time shake sample. Release, threshold, phase-2, and completion do not issue new requests. A shake already active at threshold is allowed to decay normally.

The pre-existing unrelated unpulled-sword physics edits in `main.cpp` were preserved. The integration owner concurrently incorporated the caller hunk into commit `33e5017`; I did not commit or push.

## Private verification

- `c++ -std=c++17 -Wall -Wextra -pedantic -fsyntax-only port/linux/main.cpp` — pass.
- `c++ -std=c++17 -Wall -Wextra -pedantic port/tests/fresh_save_intro_camera_caller_cpu_test.cpp -o /tmp/fresh_save_intro_camera_caller_cpu_test && /tmp/fresh_save_intro_camera_caller_cpu_test` — pass. The private fixture checks placement in the actual intro branch and traces sampled intensity, active-window refusal, decay, release, strict equality/threshold, prior-window survival, phase-2, and completion.
- `git diff --check -- port/linux/main.cpp` — pass. No shared Docker build or physical GPU `:0` run was performed; no screenshot parity claim is made.

**Ownership released:** `port/linux/main.cpp`. The caller fixture and this report are complete and also released to the integration owner. No edits were made to `CameraShake.h`, `Combat`, shared status/checkpoint, or other test files.

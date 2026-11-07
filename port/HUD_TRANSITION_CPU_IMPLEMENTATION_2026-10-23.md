# HUD transition CPU implementation — 2026-10-23

## Delivered

- `port/game/Hud.h/.cpp`: independent source-stateful health/focus meters. Each meter updates at supplied pre-registration process total before registering the current clamped target. Health interpolates decreases/increases over 0.25 s; focus snaps decreases relative to movement baseline; in-flight retargeting preserves baseline/start time; target equality compares displayed fill. Normal-state health previous fill decays by 0.005 per update, including the source's possible one-step undershoot.
- Loaded `Game/Media/UI/meter_bar_middle_back.png` and emitted health-only white previous-fill immediately before red health fill, preserving x/y/height, tint/alpha convention and existing cap/overlay/focus/orb geometry. A nonpositive backing width is omitted at the renderer boundary without changing the unrounded state.
- `port/linux/main.cpp`: binds `shaderFrame.totalTime - dt` because `shaderClock.Update(dt)` runs before draw; this is the original pre-commit meter sampling time.
- `port/tests/hud_transition_cpu_spy_test.cpp`: ordered command spy covering delayed registration, 1/60 and 0.125/0.25 s interpolation, backing decay with nonuniform intervals, focus snap and recovery, mid-move retarget, equality no-op, and geometry/order/tint/orb preservation. Existing health length spy now uses explicit time and distinct backing texture ID.

## Verification

- `g++ -std=c++17 -O0 port/tests/hud_transition_cpu_spy_test.cpp port/game/Hud.cpp -o /tmp/death-leak-hud-transition-cpu && /tmp/death-leak-hud-transition-cpu` — PASS.
- `g++ -std=c++17 -O0 port/tests/health_meter_length_cpu_spy_test.cpp port/game/Hud.cpp -o /tmp/death-leak-health-length-cpu && /tmp/death-leak-health-length-cpu` — PASS.
- `git diff --check` on owned tracked files — PASS.

## Remaining gates / ownership release

- Focused CPU behavior is verified. `port/tests/integration.sh` was dirty and owned elsewhere; no test wiring was added. Full game build, GPU/Xorg visual capture, and matched transient Windows comparison were not run in this lane. Pixel parity and visual-severity ranking remain unproven.
- No commit or push. Ownership of `port/game/Hud.h`, `port/game/Hud.cpp`, `port/linux/main.cpp`, `port/tests/hud_transition_cpu_spy_test.cpp`, and `port/tests/health_meter_length_cpu_spy_test.cpp` is released to the coordinator for integration/release. Other dirty files were preserved.

# Private body slow-frame production command gate — 2026-10-23

## Verdict

**PASS, bounded Linux CPU command gate.** A test-only observer around the real non-`--simulate-only` production renderer records the actual `DrawSprite` ID2 Still texture command after the production update and before presentation. A CPU-only no-device backend avoids SDL/GL/GPU and capture. The guarded observer changes no ordinary build behavior; `port/linux/main.cpp` contains only 17 `DEATHLEAK_TEST_BODY_SLOW_FRAME_COMMAND`-guarded lines.

Run: `python3 port/tests/private_body_slow_frame_command_gate_20261023.py` from the repository root. The script compiles an isolated temporary binary directly from the production loop, scene, combat, motion, HUD, and tinyxml sources with the test-only CPU backend; it leaves `port/build` untouched. It runs active forest-1 `--play`, neutral `.:14`, without `--fresh-save-intro`, at both requested `--fixed-dt 0.05` and `--fixed-dt 0.016666666666666666`. It asserts all completed ordinals have identical modified-current/prior-slot/body-path tuples, initial prior is zero, later prior is 1/60, and raw requests retain their distinct values.

The spawn-to-ground prelude is ordinals 0–10; the player is not grounded there, so those ordinals are **not** used as the body eligibility discriminator. In particular, the idle body command is absent on some airborne prelude ordinals. The gate explicitly requires grounded=true, exactly one player visual update, and exactly one ID2 Still body command at **each of three consecutive assessed active ordinals 11–13**. Both runs emit:

| Ordinal | 50 ms raw | Modified | Prior | Grounded | ID2 body command |
|---|---:|---:|---:|---:|---|
| 11 | 0.050000001 | 0.016666667 | 0.016666667 | 1 | `idle_default_000.png` |
| 12 | 0.050000001 | 0.016666667 | 0.016666667 | 1 | `idle_default_000.png` |
| 13 | 0.050000001 | 0.016666667 | 0.016666667 | 1 | `idle_default_001.png` |

The second assessed body draw remains `_000.png`; the >1/60 upstream timestep divergence does not reappear at the production body texture-command boundary. `git diff --check` passed. The normal 60 Hz run matches these command paths and clock slots; no animation or timestep production logic was modified.

## Scope and release boundary

Files owned: `port/linux/main.cpp` (test-only guarded observer hooks), `port/tests/private_body_slow_frame_command_observer_20261023.h`, `port/tests/private_body_slow_frame_cpu_backend_20261023.cpp`, `port/tests/private_body_slow_frame_command_gate_20261023.py`, and this report. No GPU/display/capture, build-system, status-file, renderer-backend, existing-test, or unrelated dirty-file edits. The CPU software integration lane is **released**. This is not synchronized original-Windows pixel parity or visual acceptance; visual parity remains a release blocker requiring registered Windows frame/state evidence.

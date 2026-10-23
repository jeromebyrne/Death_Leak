# Scripted Roll movement-direction DTO fix — 2026-10-22

## Scope and change

`port/linux/main.cpp` now copies the persistent `movementDirection` into `PlayerStrafeVisual` after motion update on every gameplay frame, before the interactive-only controller strafe resolver. This preserves a scripted `L`/`N`/`Q`/`Z` direction of `-1` through a subsequent scripted `O` Roll, including frames where instantaneous horizontal input is zero. The separate `facing` parameter and interactive controller behavior are unchanged.

The source-proven issue is documented in `ROLL_FRAME_MATERIAL_SPRAY_STATIC_QA_2026-10-22.md`: the Roll spray emitter uses `strafe.movementDirection`, while the former scripted path passed the DTO default `+1`. Source `Game/src/GameObjects/Character.cpp:1149-1151` uses `m_direction.X` for the spray origin and particle direction. The existing `port/tests/roll_frame_material_spray_test.cpp` establishes that a supplied `-1` produces the left origin and rightward particle base; it did not cover the production scripted handoff.

## Focused verification

- `python3 port/tests/scripted_roll_direction_handoff_test.py` — PASS. Static call-site guard checks left command mapping, persistent direction assignment outside the interactive-only branch, Roll command, and DTO delivery to `UpdatePlayerVisual`.
- `c++ -std=c++20 -fsyntax-only port/linux/main.cpp` — PASS.
- `git diff --check -- port/linux/main.cpp` — PASS.

No shared binary rebuild, full `integration.sh`, physical GPU, or Xorg run was performed because the CPU integration rerun is active. This is a bounded DTO handoff repair, not release or visual parity acceptance.

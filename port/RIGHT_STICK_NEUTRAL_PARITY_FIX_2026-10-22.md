# Right-stick neutral-before-first-aim parity fix — 2026-10-22

Scope: `port/game/LevelScene.cpp` and `port/tests/right_stick_aim_line_cpu_test.cpp` only. This is a source/fixture handoff, not a build or visual acceptance.

## Source basis

Windows `Game/src/GameObjects/Player.cpp:634-658` attaches the newly created aim-line sprite at `(+m_projectileOffset.X, +m_projectileOffset.Y)` without a vertical flip or rotation. `Game/src/InputManager.cpp:375-395` calls `SetAimLineDirection` only for active right-stick input. Therefore left body-facing while the stick remains neutral before its first active input does not move, flip, or rotate the authored line. The port load defaults already encode +5, unflipped, zero angle.

## Exact patch delta

- In `LevelScene::UpdatePlayerVisual`, removed only the `else if (!playerAimLineDirected_)` neutral fallback and its three assignments (`playerAimLineFacing_ = facing`, `playerAimLineFlipY_ = facing < 0`, `playerAimLineRotation_ = 0.f`). The active-stick branch is unchanged. Neutral now leaves the initialized transform intact before first use, and continues to retain the last active transform afterward. Body facing still updates independently.
- In the existing forest-1 CPU draw-spy fixture, inserted a pre-active-stick step with body `facing=-1`, neutral stick, and a fire attempt to make the line visible. It asserts one full-alpha line, authored +5 attachment (screen X `640 + (5-200)*2/3`), no horizontal flip, SDL `flipY=true` (inverse of authored vertical unflip), and zero rotation. The existing quadrant, launch-vector, hold, and neutral-after-active fade assertions remain unchanged. Updated the PASS label to name neutral-first coverage.

The tracked `LevelScene.cpp` worktree diff also contains earlier shared right-stick implementation changes; those were preserved and are **not** claimed as this patch. The CPU fixture was already untracked when this task began; only the neutral-first assertions and PASS label were added to it here. No shared checkpoint/status file was touched.

## Validation handoff

No build, CPU test, integration script, or GPU capture was run by this worker; the integration/build lane is owned elsewhere. Pending owner validation: compile/run `port/tests/right_stick_aim_line_cpu_test.cpp`, then `port/tests/strafe_body_cpu_test.cpp`, and review the relevant integration result. Static source inspection confirms the removed branch and load defaults; runtime pass is not claimed.

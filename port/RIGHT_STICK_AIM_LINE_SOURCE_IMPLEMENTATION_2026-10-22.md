# Right-stick aim-line source slice — checkpoint (2026-10-22)

Scope: forest-1 player line only, using `NEXT_CURRENT_TREE_VISUAL_GATE_2026-10-22.md`. This is **not** a visual-fidelity acceptance or GPU result.

## Implemented, not yet verified

- `port/linux/main.cpp` forwards the live, world-up-converted right-stick aim flag/vector already sent to `Combat::Update` into `LevelScene::UpdatePlayerVisual`.
- `LevelScene` uses the four `Player::SetAimLineDirection` branches: `X>0` selects +5 X attachment and authored unflipped Y; `X<=0` selects -5 and authored flipped Y. Rotation uses `acos(normalized X)` with the source quadrant sign, then the port's `OrdinarySpriteRotationDegrees` for SDL. The ordinary-texture inverse-V rule is retained. Neutral stick preserves the last controller-set transform; keyboard-only horizontal behavior remains until a controller aim occurs. The existing 400x400 art, painter depth/order, alpha, body orientation, projectile physics and camera path remain unchanged.
- Effective `PlayerStrafeVisual::active` refreshes the existing .05 s line hold independently of weapon attempts. The existing 10/s fade remains.
- Added `port/tests/right_stick_aim_line_cpu_test.cpp`: CPU forest-1 draw spy for ±X, four diagonals, ±Y, opposed movement/aim, attachment, rotation, Y flip, alpha, matching recorded knife launch vector, no-fire strafe hold, and neutral fade.

## Verification gate / ownership

`port/tests/integration.sh` was running concurrently during implementation and still owned the build/GPU lane at checkpoint. Per task boundary, no build, CPU test execution, GPU/Xorg capture, commit, or push was performed. `git diff --check` on the three source files passed; this is only whitespace hygiene. The preexisting `port/tests/strafe_body_cpu_test.cpp` contains an obsolete expectation that no aim line is emitted on early synthetic strafe frames (`spy.drawn.size()==1`); its compatibility with the corrected source hold needs coordination with that test's owner. Do not treat the already-running integration result as validation of this source revision: its compilation began before these edits.

After build lane release, compile/run the focused gate with `g++ -O2 -g -std=c++17 -Wall -Wextra -Wpedantic -UNDEBUG -Iport/compat port/tests/right_stick_aim_line_cpu_test.cpp port/game/LevelScene.cpp port/game/Combat.cpp port/game/PlayerMotion.cpp Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp -o /tmp/right-stick-aim-line-cpu-test && /tmp/right-stick-aim-line-cpu-test` from repo root; then reconcile the preexisting strafe fixture and arrange the exclusive GPU controller-aim frame. Commit/push only after verification. Scripted-B footage is a negative control and no Windows pixel parity is claimed.

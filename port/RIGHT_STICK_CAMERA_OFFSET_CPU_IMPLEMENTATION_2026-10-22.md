# Right-stick camera-offset CPU implementation handoff — 2026-10-22

## Scope and result

Implemented a reusable, source-backed right-stick camera target and follow-order seam; **not wired into the live `main.cpp` loop** because this assignment explicitly barred editing that shared file. Therefore this is a passing CPU component/scene-draw test, **not a live controller or GPU parity result**.

Exact files touched:

- `port/game/LevelScene.h` — adds `RightStickCameraTarget` (authored/effective offsets, SDL rightY conversion, strict 8689 per-axis eligibility, raw-axis `/32767` X/Y updates, neutral reset, and source-equivalent active roll/fall/melee X-hold with Y update) and `FollowCameraWithTarget` (pre/post bounds clamp, follow flags, zoom, per-axis/enemy lag). Disconnect and ineligible gameplay preserve previous target, matching the Windows skipped-input path. SDL `-32768` Y maps to XInput-representable `+32767`; negative X retains `/32767` asymmetry.
- `port/tests/right_stick_camera_cpu_test.cpp` — new standalone CPU fixture: ±X/Y, diagonal, threshold, neutral, gated states, disconnect/pause, Y sign/asymmetry; follow-before-input and next-frame neutral return; lag/zoom/flags/clamp; forest scene render spy for ID37's camera-dependent destination and float X UV phase, and aim-line destination under the same viewport-scaled camera transform. ID37's authored Y UV multiplier is zero, so the fixture asserts Y UV remains unchanged.
- This report, `port/RIGHT_STICK_CAMERA_OFFSET_CPU_IMPLEMENTATION_2026-10-22.md`.

No edits to `SDLRenderer.cpp`, `main.cpp`, `Renderer.h`, `integration.sh`, checkpoint/status files, GPU/fog sources, or shared build output.

## Verification

Standalone host CPU command passed (assertions enabled), compiling into a unique `/tmp/right-stick-camera-XXXXXX` directory and running from repository root:

```sh
g++ -O2 -std=c++17 -Wall -Wextra -Wpedantic -UNDEBUG -Iport/compat \
  port/tests/right_stick_camera_cpu_test.cpp port/game/LevelScene.cpp port/game/PlayerMotion.cpp \
  Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp \
  Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp -o "$verify_dir/test"
env -u DEATHLEAK_RENDER_HIDE_XML_IDS "$verify_dir/test"
```

Exit 0. Compiler emitted only existing TinyXML warnings. `git diff --check -- port/game/LevelScene.h` passed. No game/Docker/global rebuild or Xorg/GPU capture was run. The fixture is not registered in `integration.sh` by assignment constraint.

## Required runtime integration (reserved `main.cpp` owner)

1. Store `RightStickCameraTarget` initialized from `scene.CameraOffsetX/Y()` at initial level load and every `scene = ...` replacement (restart, transitions, menu reset). Do not mutate authored scene offsets.
2. At current `main.cpp` follow point, use `FollowCameraWithTarget` with **the stored previous target**, preserving the current before/after clamp and scripted-camera override. The current direct `scene.CameraOffsetX/Y()` reads are still fixed, so the feature is not active yet.
3. After gameplay-active, controllable controller processing, call `target.Update(controllerSticks, eligible, rolling, falling, melee)` for the **next** update. Do not gate it on `strafeVisual.active`; do not update during modal/death/pause/menu paths. Keep headless script/controller-free behavior fixed. SDL rightY conversion already occurs in `Update`; avoid a second inversion. Define the fall gate from the port's motion flags corresponding to Windows `JustFellFromLarge/ShortDistance`, not ordinary jump/landing unless source-equivalent.
4. Run a narrow live-input/integration validation once the shared build lane is released, then physical-GPU visual review by its owner. The CPU spy establishes emitted float UV/destination behavior, not renderer fractional-UV pixels or live SDL wiring.

No commit/push: runtime wiring is intentionally incomplete under the explicit no-`main.cpp` ownership rule, and concurrent uncommitted shared renderer/main changes remain under other owners. The three scoped files are left for the coordinator to integrate/commit after the reserved lane is released.

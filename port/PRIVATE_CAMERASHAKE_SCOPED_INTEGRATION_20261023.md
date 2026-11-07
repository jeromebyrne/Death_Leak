# Private CameraShake scoped API integration — 2026-10-23

## Result

**PASS; committed and pushed only `port/game/CameraShake.h`.** Commit `92562c4daf36f56d785d85fbdb81935d485cf884` (`Add parameterized camera shake API for intro pull`) is on `origin/feature/linux-renderer-foundation`, verified with `git ls-remote`. The index is empty afterward. All unrelated dirty tracked files, including `Hud.cpp`, `main.cpp`, and `SDLRenderer.cpp`, were preserved. The released private fixture remains untracked by design; this integration commit contains only the header.

## Source contract and verification

The header's new `Start(intensity, duration, totalTime)` latches parameters only when inactive; active requests are refused. `Update` and `WillJitter` use the latched duration/intensity, while `StartMedium` retains landing's 15/0.30 parameters. This matches `Game/src/Core/Camera2D.cpp:287-307` and supplies the `Start` member called by existing HEAD `port/linux/main.cpp` (the mismatch identified in `PRIVATE_CAMERASHAKE_SCOPED_DEPENDENCY_AUDIT_20261023T2028Z.md`). The exact staged diff was one file, 12 insertions and 5 deletions; `git diff --cached --check` passed.

- `c++ -std=c++17 -Wall -Wextra -pedantic port/tests/fresh_save_intro_camera_shake_cpu_test.cpp ... && fixture` — passed, covering active-window refusal, latched intensity, decay, expiry, follow gates, reset, and landing wrapper.
- Existing `port/tests/camera_shake_cpu_test.cpp` with the same flags — passed.
- Clean scoped build: `git archive` of pre-commit HEAD `515fe23`, overlaid with **only** the SHA-identical working header (`0fc74544c8f7837613486e03478403ba06015de48e622ce9dcfed206c163bdcf`), built in private ignored `port/build/private-camerashake-scoped-20261023/snapshot` using `docker run ... deathleak-linux-build:bookworm make -C port -B` — exit 0. Only pre-existing tinyxml compiler warnings appeared; no further dependency surfaced in this build. The snapshot contains no unrelated dirty changes.

The separate focus-bar worker's `PRIVATE_FOCUS_BAR_VERTICAL_UV_PHYSICAL_GPU_GATE_20261023.md` explicitly released Docker build/GPU ownership, and no active build process was present before this serial build. No GPU/Xorg action was taken here.

## Scope and release

This proves a clean **HEAD plus CameraShake-header** compile, not a clean build or visual acceptance of any current unrelated dirty diff. No shared status file or unrelated source was modified; no HUD/main/renderer staging was done. **CameraShake.h, its private fixture, index, and Docker build ownership are released.**

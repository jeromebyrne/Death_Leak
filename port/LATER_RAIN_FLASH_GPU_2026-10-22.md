# Later rain/flash visible-overlap GPU discriminator — 2026-10-22

## Capture and state

At committed `50ba9e9`, I captured fresh, sequential physical-Xorg `:0`/nouveau **NVE7** forest-1 runs at `--script .:4098 --frames 4098 --fixed-dt 0.05`. Before capture, process inspection found Xorg but no competing game, FFmpeg, compiler, or build; no competing capture/build was started. All three runs used `deathleak-linux-build:bookworm`, `/dev/dri`, X11, strict SDL accelerated OpenGL (`accelerated=1 software=0`), GPU Sepia, dummy audio, the same authored 71 sprites/zero missing assets, and identical grounded player endpoint `(-43.0146,3206.9)` with one orb. Capture logs are in ignored `port/build/later-rain-flash-gpu-2026-10-22/`; their ~86–87 s `elapsed` is **wall time**, not weather simulation time.

The game logs do not print weather state directly. A standalone probe of the **same `WeatherLightning.h` class**, with weather allowed and 4098 updates of 0.05f, logged rain start frame 3601, second-flash trigger frame 4096 (alpha still 0), frame 4097 alpha 0.874992, and frame **4098 alpha 0.749985** (`weather-frame-probe.log`). The normal-vs-no-flash and normal-vs-no-rain GPU discriminators below independently confirm that both effects are rendered at the captured frame. This is a deterministic port-state check, not a Windows global-`rand()` timing claim.

## Same-state A/B result (1280×720 RGB)

The normal executable was the existing shared `port/build/deathleak-linux` built from unchanged HEAD source. Diagnostic executables were built in a detached isolated worktree, with **only one render guard disabled per variant** in `port/linux/main.cpp`: `false && rain.VisibleParticles()` or `false && weather.Alpha() > 0.f`. The guards leave update/simulation, camera, assets, and input unchanged. Their executables, build logs, BMPs, PNGs, and x4 RGB differences are retained in the ignored evidence directory. The worktree was removed after verification; no tracked game source was changed.

| Capture | BMP SHA-256 | Against normal |
|---|---|---|
| normal (`normal.bmp`) | `4f2d2fe9acdee6dfa718b61f214da52176d695405a7847e945fa5b0f9abea800` | baseline |
| no rain (`no-rain.bmp`) | `4bfc9cb274b52ef3b3e4c10d6fd1587758ab044880b7393b7fa3a0e7ace5673d` | **7,206** changed RGB pixels; bbox `(3,0)-(1266,335)` (exclusive upper bound); mean absolute RGB difference `(0.041,0.035,0.025)` / 255 over full frame; max `(40,34,24)` |
| no flash (`no-flash.bmp`) | `c0c816ff107f3aeaa76b1ef43cd3229824ab93e73cba243a8c3e886e5f974245` | **921,600/921,600** changed RGB pixels; full-frame bbox; mean absolute RGB difference `(70.685,60.764,42.867)` / 255; max `(150,129,91)` |

The 7,206 localized rain-changed pixels all lie within the flash-changed full frame. Example rain streak pixel `(638,29)` is normal RGB `(143,123,87)`, no-rain `(103,89,63)`, and no-flash `(110,95,67)`: the later flash and established rain are **visibly present in the same captured pixels**. This resolves the first-flash A/B's zero-rain result. The broad flash/background and localized rain-streak contributions are both visible; no missing painter is observed.

## Painter order and limit

Source `WeatherManager.cpp` creates flash before rain at the same depth 20, and source `GameObjectManager.cpp` stable-sorts equal-depth objects. Port `main.cpp:1132-1149` appends the flash painter before rain; `LevelScene.cpp:820-822` stable-sorts depth peers. With source-over blending, the established rain is drawn **over** the lightning layer in both code paths. The A/B proves actual physical-GPU overlap at the later port flash; it does not isolate exact D3D-vs-SDL filtered pixels, nor prove Windows pixel parity or Jerome's visual acceptance. No source-proven ordering defect emerged, so no speculative production edit was made. `CHECKPOINT_STATUS.md` already had unrelated in-progress edits and was deliberately left untouched.

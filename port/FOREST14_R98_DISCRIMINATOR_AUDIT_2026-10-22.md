# Forest screenshot 14 — R:98 camera/landmark discriminator (2026-10-22)

## Decision

`R:98` is a **testable Linux camera-state candidate**, not a matched Steam frame. On the collision-free scripted-right route, source predicts camera X≈593.214 versus X≈689.085 at the physically captured `R:110`. Consequently every ordinary static forest-1 landmark moves **+63.91 px** on the 1280×720 backbuffer, while the player remains at nearly the **same screen X** (≈719.85 versus ≈719.93): the player travels 96 world units left of its R:110 position, almost exactly offset by the camera's 95.87-unit leftward change. Thus the useful discriminator is **pickup/branch/raft displacement relative to the player**, not merely whether the whole picture slides. No current still proves a forest-14 renderer/geometry defect; the Steam image is an unknown gameplay state with 13 orbs/reduced health, while the R:110 capture has 3 orbs/full health. Haze, raft seam, pose, effects, and JPEG presentation remain uncontrolled.

One caution on direction: `FOREST14_LINUX_MATCH_CAPTURE_2026-10-22.md` estimates Linux landmarks 50–70 px **left** of scaled screenshot 14, for which R:98 is the correct predicted direction. `FOREST14_LANDMARK_COMPARISON_2026-10-22.md` qualitatively calls the earlier port still's trunk/branch/pickup **right** of the reference. These are different captures and unregistered visual judgments; do not promote either sign to an exact Windows camera measurement. The next capture must record measured landmark coordinates before declaring R:98 better.

## Source calculation and falsifiable prediction

- Authored forest-1 camera: `(0,3400)`, `xLag=15`, `followx=true`, `followy=false`, zoom 1 (`Game/XmlFiles/levels/forest_exploration_1.xml:3-4`). The 1920×1080 logical camera is clamped before/after following; this candidate is not at a bound (`port/linux/main.cpp:641-673`). On the no-enemy route, camera follows **pre-motion** X: `C_n=C_(n-1)+(X_(n-1)-C_(n-1))/15`; player advances `min(n,8)` world units/update from X=`-43.014648` (`port/linux/main.cpp:674-680`, `port/game/PlayerMotion.cpp:7-11,49-67`). The physical R:110 log corroborates calculated player X≈808.985 with observed 808.986, but logs **no camera X**. Calculated R:98 player X≈712.985.
- `port/game/LevelScene.cpp:978-980` projects these ordinary static sprites at screen center `640+(worldX-C)*2/3`. XML centers: pickup ID117 X=1167.985; lower/upper branches IDs 30/29 X=964.685/1012.785; raft IDs 11/57/58 X=267.985/640.085/−132.215. Predicted *sprite center X*, not identifiable texture tip/seam pixel:

| Authored center | R:110 predicted px | R:98 predicted px | Shift |
| --- | ---: | ---: | ---: |
| Pickup 117 | 959.3 | 1023.2 | +63.9 |
| Lower branch 30 | 823.7 | 887.6 | +63.9 |
| Upper branch 29 | 855.8 | 919.7 | +63.9 |
| Raft 11 / 57 / 58 | 359.3 / 607.3 / 92.5 | 423.2 / 671.2 / 156.4 | +63.9 each |
| Player | 719.9 | 719.8 | −0.1 |

This gives a within-port falsification: if the next physical `R:98` still does **not** translate the pickup, branches, and raft by a common ≈64 px from the R:110 still (after accounting for exact visible texture landmarks and occlusion), investigate projection/state/provenance. If it does, compare registered **static** pickup–branch–raft geometry with the normalized Steam image. A consistent residual horizontal offset suggests camera state; nonuniform residuals/changed seam shape after registration warrant checking rotation, occlusion and draw order. Do not use parallax trees, animated or collected orbs, player pose, or fog to estimate camera X.

## Minimum next physical capture

The GPU-lane owner should capture one verified-current-binary forest-1 frame using the existing physical X11/OpenGL recipe from `FOREST14_LINUX_MATCH_CAPTURE_2026-10-22.md`, changing only to `--script R:98 --frames 98 --fixed-dt 0.0166667` and a unique output/log path. Record binary/HEAD, renderer, actual frame/log state, and screenshot dimensions. First register the R:98 and R:110 port images using the pickup shaft/root, lower and upper branch silhouettes and raft piece junctions; check the **common +64 px shift and stationary player**. Then normalize the Steam JPEG's 1920×1080 to 1280×720 (account explicitly for any letterbox bars) and measure the same landmarks. Only if the static geometry aligns should haze/raft contour be evaluated, ideally with a lossless original Windows frame carrying actual camera X/Y, viewport, player/collection state, level-relative frame and weather/fog phase. R:97/R:100 are optional bounds only if R:98 brackets a measured match.

Scope: read-only source/report audit plus this report. No build, GPU/game process, source/test/shared-status edit, or exact Windows parity claim.

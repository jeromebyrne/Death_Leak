# Private Forest right-exit actual-camera source gate — 2026-10-23 21:22Z

## Result

**The saved frame-540 packet does not contain an observed numeric draw camera or command stream.** The existing `B:300,F:240`, fixed-1/60, 540-frame Linux capture logs end at player `(2360.52,3114.74)` and the current-tree hide-ID15 pair changes 79,175 RGB pixels in `[1011,316,1280,649)`, but the logs and image packet do not serialize `drawCameraX/Y`, cull events or `DrawSprite` calls. Consequently the earlier CPU fixture at `(2360.52,3400)` is **not** an actual-frame replay. No actual-camera command pass, Windows parity, or patch is claimed here.

## Strong source-constrained camera candidate, not a measurement

Forest-1 XML starts camera `(0,3400)`, `followx=true`, `followy=false`, level bounds right `3000`; `LevelScene::Load` enables those bounds. `FollowCameraWithTarget` clamps camera X to at most `right−960=2040` after each follow, while its Y remains 3400 (the 0..10000 vertical bounds cannot move that value). The active draw uses `cameraShake.Update(...).viewX/viewY`, so the draw camera is a sampled *pre-shake-update* value, not simply the terminal player coordinate (`port/linux/main.cpp` follow block around 813, sample around 1284, `scene.Draw` around 1388; `port/game/LevelScene.h:119-142`, `port/game/CameraShake.h`). The `B:300,F:240` route moves right then holds fire/position; the terminal player is 320.52 world units beyond the clamp. If X had reached its bound before the final draw and no recent shake displaced the stored camera, **the predicted frame-540 camera is `(2040,3400)`**. That prediction also places the player's nominal X around `640+(2360.52−2040)*2/3=853.68`, consistent with the saved hide-player changed bbox `[810,509,893,621)`. A changed-pixel bbox is not a calibrated player origin and cannot prove the camera to subpixel precision. The route's per-frame camera history and shake state were not saved; the candidate must remain conditional.

At this **candidate** camera, the corrected source oracle (`port/game/LevelScene.cpp` parallax and viewport math; prior `PRIVATE_RIGHT_EXIT_ID15_CPU_COMMAND_GATE_20261023T2119Z.md`) predicts the following logical pre-SDL destinations, rounded to 0.01 px. These are calculations, **not measured frame-540 commands**:

| XML ID | expected `(x,y,w,h)` at C=2040,D=3400 | source admission expectation |
|---|---|---|
| 19 | `(-2420.14,401.43,4902.75,682.67)` | intersects; repeat X |
| 21 | `(-927.97,-269.95,1323.33,825.33)` | intersects |
| 38 | `(-6600.34,-674.74,13333.3,1828.92)` | intersects |
| 17 | `(-2721.88,464.83,4902.75,682.67)` | intersects; repeat X |
| 22 | `(-2006.88,-166.19,3364.74,825.33)` | intersects; repeat X |
| 39 | `(-7257.34,-564.46,13333.3,1828.92)` | intersects |
| 15 | `(973.99,318.00,341.33,341.33)` | rotated AABB intersects; +84.2247°, flipY |
| 37 | `(-42.67,589.33,1365.33,341.33)` | intersects; UV X shift `1.40760` |

The source-relative painter order remains `19,21,38,17,22,39,15,37`; absolute ordinals can include other objects. For ID15, the source rotated horizontal AABB at C=2040 is approximately `[958,1331]`, and vertical AABB approximately `[302,675]`. The hide-ID15 pixel delta lies inside that broad projected footprint, but texture alpha, overdraw, postprocess and cropping prevent solving exact camera X from its bbox. The earlier fixture's C=2360.52 predicted ID15 x=760.31—**213.68 logical pixels left** of the clamp-candidate x=973.99—so it is materially the wrong geometric test camera for this packet even though its command-field contract passed.

## Minimum missing observation / next discriminator

The minimum missing observation is the **final rendered ordinal's actual `drawCameraX,drawCameraY` pair**, taken immediately after `cameraShake.Update` and passed to `scene.Draw`, tied to the exact frozen binary/input route and capture hash. The same-point `cameraX/Y`, shake active/remaining and zoom fields distinguish clamp inference from a late shake or a different binary path. A read-only debugger trace on the *already frozen* executable at the final `LevelScene::Draw` entry can supply this without a production edit; require its saved frame hash to equal the target `baseline.bmp`, as was done for the separate R:180 gate. This execution is reserved for the exclusive GPU/build owner, not this read-only lane.

Once that sidecar exists, feed the measured pair and matching visual-update state into the existing CPU recording-renderer fixture, or have the owner log the selected eight XML IDs directly at the actual frame-540 draw boundary. Compare admission, relative order, destinations, rotation/flips, UV shift and alpha to the **corrected** C,D oracle (the 21:19 fixture report supersedes five incorrect Y formulas in the older 21:14 audit). Demand one admitted ID15 command, and separately retain the saved hide-ID15 image as pixel-contribution evidence. `DrawSprite` alone cannot establish SDL sampler/GL-branch behavior or original-Windows visual parity; that needs an independently matched original frame/packet.

## Scope/evidence

Read-only inspection of saved logs/reports, source XML and camera/renderer code; arithmetic only. No game/debugger run, GPU/Xorg/Docker, build, capture, production file, test, shared checkpoint/status, commit or push. This report is the sole write.

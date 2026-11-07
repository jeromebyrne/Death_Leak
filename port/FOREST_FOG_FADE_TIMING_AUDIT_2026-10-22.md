# Forest-1 fog 119/122 fade-timing source audit (2026-10-22)

Scope: read-only comparison of authored forest fog and original Windows versus current Linux source. No build, GPU/display run, source/test edits, or Steam JPEG-based parity inference.

## Source contract

- `Game/XmlFiles/levels/forest_exploration_1.xml:1631-1661`: both are `ScrollingSprite`, `updateable=true`, `always_update=false`, `Media\\fog.png`, pixel wobble. ID 119 starts at (789.985352, 2954), alpha 0.5, moves +1000 X over 45 s; ID 122 starts at (-125.014648, 2861), alpha 0.6, moves -750 X over 70 s. Both fade in for 6 s and fade out for 6 s.
- Windows `Game/src/GameObjects/ScrollingSprite.cpp:5-10,23-59,60-70` initializes elapsed to zero; each eligible update adds `delta`, resets at `elapsed >= LerpTime` and skips the XY/alpha update on that frame. Before 6 s, alpha = authored alpha × elapsed/6; at exactly 6 s it is authored alpha. `DrawableObject.cpp:214-219` saves the authored alpha as `mOriginalAlpha` on initialization. Thus the pre-first-update level frame is *full authored alpha*, not transparent.
- Linux `port/game/LevelScene.cpp:517-530,831-855` loads the same alpha/timings and duplicates that fade, position, and endpoint-skip logic. `port/game/LevelScene.h:27` initializes elapsed to zero. Draw uses `object.alpha` as opacity (`LevelScene.cpp:1084-1085`). Existing `port/tests/scrolling_sprite_test.cpp:38-65` encodes full alpha initially, 1/6 alpha at 1 s, and full alpha at 6 s; this audit did not execute it.

With uninterrupted eligible updates from level entry, the **source-predicted** values at level-relative elapsed times are:

| elapsed | ID 119 alpha / X | ID 122 alpha / X |
|---|---|---|
| 0 s, before first update | 0.50 / 789.985352 | 0.60 / -125.014648 |
| 3 s | 0.25 / 856.652019 | 0.30 / -157.157505 |
| 6 s | 0.50 / 923.318685 | 0.60 / -189.300362 |

At the first positive-delta update, both abruptly drop from the authored initial alpha to a small fade-in value. This discontinuity is **shared by Windows and Linux source**, so it is not a proven port bug. The 3/6 s figures refer to accumulated eligible update delta, not necessarily wall time. Windows `GameObjectManager.cpp:205-257` skips updates when paused or outside its update zone; Linux has a corresponding scrolling-sprite zone check (`LevelScene.cpp:831-855`). Both sprites intersect the 4000×4000 initial camera zone from the authored camera (0,3400; XML line 3), so normal initial play should advance them.

## Verdict and next isolated test

No concrete **unpaused 0/3/6 s fog fade formula** parity bug is provable from source. A separate possible pause-path difference warrants isolation: Windows manager receives `mPaused` and stops object updates (`Game.cpp:170-179`; `GameObjectManager.cpp:209-257`), while Linux calls `scene.UpdateVisuals(dt, ...)` unconditionally even when its paper/upgrade modal branches skip gameplay (`port/linux/main.cpp:573-600,1067-1078`). Whether those modal states correspond to Windows `mPaused` requires a matched interaction trace before declaring a user-visible fog parity bug.

Next isolated CPU-only test: capture ID 119/122 opacity and X immediately after load and after accumulated **eligible** deltas of 3 and 6 seconds at a fixed camera in the initial zone; separately compare a 3-second paused/modal interval between Windows and Linux with equivalent pause states. Do not use an unmatched Steam JPEG as evidence of timing parity.

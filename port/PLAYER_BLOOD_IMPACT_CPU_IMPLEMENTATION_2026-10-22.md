# Player blood first-contact splat — CPU implementation (2026-10-22)

## Implemented scope

`Combat` now independently sweeps each active player-blood center against `LevelScene::Ground()` every tick, without the knife's live-enemy line skip. The deterministic winner is the earliest center-segment intersection (segment order breaks ties); endpoint and collinear overlaps are included. This is a port approximation to the Windows 64×64 broad phase and last-center-to-current-center ray, not exact source collision-manager ordering. An impact stops active velocity and occurs once, uses the preselected `blood_impact.png` or `blood_impact_2.png` variant from the blood-only spawn RNG, applies signed 0–19.8 Y jitter plus -20 Y, and draws at ground-blood depth 4000. The inactive clock begins on contact; scale ramps in over the first 5% of 5.5 s, alpha fades over its last 20%, and the effect survives equality and is removed only after 5.5 s. Active flight still retains its documented port-only 10 s orphan bound. `ClearLevel` clears either phase.

`Combat::SetArt` reads the authored `is_pierceable` flags from `Game/XmlFiles/materials.xml` and loads both impact textures. For a pierceable line with an available material particle texture, the existing directed 10-particle line-spray helper emits at the untruncated blood ray-end/current center and source pre-impact depth 3000; zero launch direction is passed. A line with no material splats without spray. Blood does not use shots, apply damage, attach to actors, or play a line-hit sound.

## CPU verification

From repository root:

```sh
g++ -O2 -std=c++17 -UNDEBUG -Iport/compat port/tests/player_blood_impact_cpu_test.cpp port/game/Combat.cpp port/game/LevelScene.cpp Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp -o /tmp/player_blood_impact_cpu_test && /tmp/player_blood_impact_cpu_test
```

Passed: `PASS: first blood line splat, material/no-material, inactive envelope, deferred nonpierceable`. The fixture checks a line crossing with a live enemy, one transition and stopped Y velocity, authored pierceable `water` spray count/depth/ray-end, impact variant/depth/zero initial scale, jitter range, no health change, 90%-elapsed half alpha, exact inactive equality/removal, authored nonpierceable `caverock` not falsely splatting, material-less splat, and level clear.

The existing `player_hit_blood_visual_test.cpp` was compiled with the same source list and passed its active-flight regression. `git diff --check -- port/game/Combat.cpp port/game/Combat.h` passed. No GPU, renderer, LevelScene, SDL, Xorg, or build was touched.

## Deliberate limits

- Nonpierceable and unknown-material contacts are skipped by this minimal **splat** slice. Their source rebound is not implemented; it requires line-strip normal/identity not retained by `GroundSegment`. Such flights remain active until the port-only orphan bound. This avoids a false impact texture/depth. `SolidBox` is never treated as a material line.
- The port stops the blood center at center-sweep contact but places material spray at the untruncated new center, matching the source ray-end concept only approximately. The existing spray helper has port particle parameters rather than an independently proved exact Windows blood-emitter sequence.
- Inactive age uses `dt`; Windows uses `Timing::GetTotalTimeSeconds()`. This is not wall-clock ordering parity. The CPU fixture is not a visual/GPU acceptance test.
- The focused fixture does not exhaustively cover every endpoint/tangent/parallel/collinear multi-segment ordering case from the audit contract.

No commit or push: `Combat.h/.cpp` already contained uncommitted player-shot launch-state and prior active-blood changes before this slice. A scoped commit cannot stage these files wholesale without incorporating that unrelated in-flight work; the new fixture/report are left in the shared worktree for coordinated integration.

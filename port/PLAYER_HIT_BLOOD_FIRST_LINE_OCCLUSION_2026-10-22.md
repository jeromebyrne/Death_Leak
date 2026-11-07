# Player-hit blood: first-line occlusion — 2026-10-22

## Source check

Windows `Projectile::HandleSolidLineStripCollision` (`Game/src/GameObjects/Projectile.cpp:608–711`) processes a valid line before deciding whether its material is pierceable. A nonpierceable material emits a directed spray, retains the flight texture/depth, and enters `ReboundOffSolidLine` with a 0.25 s inactive lifetime (`:740–772`). `SolidLineStrip::GetProjectileCollisionData` (`Game/src/GameObjects/SolidLineStrip.cpp:418–458`) is subject to a box test and ray intersection. The Linux center sweep is therefore only a deterministic approximation, not a claim about Windows object-list collision order.

## Scoped change

- `Combat::Update` now selects the earliest **geometric** blood/ground-segment contact before checking material. Authored-order ties retain the first segment. A front nonpierceable or unknown-material line cannot be skipped in favor of a farther pierceable line in that sweep.
- Such a blocked flight stops at the front contact, remains unsplatted (`blocked`, not `impacted`), retains `blood_projectile.png` at depth 3000, and is retired only after the source rebound lifetime of 0.25 s. This intentionally avoids inventing a rebound normal/velocity from `GroundSegment`, which does not carry the strip normal. A pierceable or material-less first line retains the existing splat path.
- `Combat.h` adds the blocked state and one focused fixture friend. Existing dirty launch-state, knife, blood-impact, renderer, scene, and integration changes were preserved. No shared file was committed.

## CPU verification

From repository root, both commands passed:

```sh
g++ -O2 -std=c++17 -UNDEBUG -Iport/compat port/tests/player_blood_first_line_occlusion_cpu_test.cpp port/game/Combat.cpp port/game/LevelScene.cpp Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp -o /tmp/player_blood_first_line_occlusion_cpu_test && /tmp/player_blood_first_line_occlusion_cpu_test
g++ -O2 -std=c++17 -UNDEBUG -Iport/compat port/tests/player_blood_impact_cpu_test.cpp port/game/Combat.cpp port/game/LevelScene.cpp Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp -o /tmp/player_blood_impact_cpu_test && /tmp/player_blood_impact_cpu_test
```

New fixture result: `PASS: nearest blood line blocks rear splat; pierceable front still splats`. It authors the rear line first, crosses both in one tick, tests known nonpierceable `caverock` and unknown material ahead of pierceable `water`, checks front-contact Y, no material spray/impact texture/depth, no later rear splat, exact 0.25 s equality/removal, and the inverse (pierceable front) splat. Existing fixture result: `PASS: first blood line splat, material/no-material, inactive envelope, deferred nonpierceable`. `git diff --check -- port/game/Combat.cpp port/game/Combat.h` passed.

## Remaining fidelity gaps

- Blocked flight is stationary until retirement; Windows rebounds, moves/spins, and emits its nonpierceable material spray. Implementing that branch requires preserved strip normal/identity and a separate source-order/visual gate. This patch fixes the false rear splat/occlusion only.
- The port center sweep lacks Windows 64×64 broad phase, object-list collision ordering, and wall-clock inactive aging. No GPU, Xorg, Docker, renderer, main, LevelScene, or integration test/build was run by this slice.

# Forest-1 ID47 emitter draw admission — CPU proof and scoped implementation

Disposition: **source-equivalent ID47-only draw gate implemented.** No physical GPU/display or matched Windows packet was used. This proves a deterministic render-space camera-edge difference, not a particular captured-frame pixel mismatch.

## Source predicate

Forest-1 XML ID47 is centered at `(201.684967, 4334.996094)` with `50000.269531 × 2000.207764` bounds, zero rotation and no parallax multiplier. Original `GameObjectManager::Draw` admits a non-player only after `Alpha()>0 && Camera2D::IsObjectInView` (`Game/src/GameObjects/GameObjectManager.cpp:302-330`). `Camera2D` tests strict origin containment, then strict rectangle overlap (`Game/src/Core/Camera2D.cpp:39-80`). Original `GameObjectManager::ScaleObjects` scales position and dimensions by backbuffer/`1920×1080` (`Game/src/GameObjects/GameObjectManager.cpp:452-463`; `Game/src/GameObjects/GameObject.cpp:454-466`). With the original 1280×720 render space represented by the Linux port, `scale=2/3`, camera half extents are `(640,360)` render pixels, equivalent to `(960,540)` XML-world units. The ID47 Y lower edge is `3334.892212`; therefore its lower camera-center admission boundary is `2794.892212` world units. Touching this edge alone is rejected by strict source inequalities.

`GameObject` defaults to parallax multiplier `(1,1)` and current offset `(0,0)` (`Game/src/GameObjects/GameObject.cpp:17-34`); ID47 does not author an override. Source `ParticleSpray` has zero rotation. The authored giant dimensions are culling bounds, not an emitter quad.

## Deterministic CPU oracle

`port/tests/forest47_emitter_draw_cpu_test.cpp` injects four fixed particles, one warmed/advected downward to world Y=2703 after a feasible 6.8 seconds at four world units/tick and 60 ticks/second. At camera center `(201.684967, 2793.892212)`, the emitter is wholly above the camera but that leaf is inside the viewport: old per-particle clipping submits one draw, source predicate admits zero. At camera Y=`2795.892212`, emitter and leaf are both visible and source admission submits one draw. At exact contact Y=`2794.892212`, strict source overlap rejects. The test independently transcribes source `Camera2D` in scaled render coordinates and compares the production predicate.

## Change and verification

Only `port/game/ParticleSpray.h` adds `Forest47EmitterInView`; only `port/game/LevelScene.cpp` applies it when `spray.xmlId == "47"` at the existing painter slot. Other spray IDs, update culling, particle RNG/motion/alpha, art and renderer are untouched.

- CPU oracle: `c++ -std=c++17 -Wall -Wextra -Wpedantic -UNDEBUG port/tests/forest47_emitter_draw_cpu_test.cpp -o /tmp/forest47_emitter_draw_cpu_test && /tmp/forest47_emitter_draw_cpu_test` — pass.
- Modified translation unit: `c++ -std=c++17 -Wall -Wextra -Wpedantic -Iport/compat -c port/game/LevelScene.cpp -o /tmp/forest47_LevelScene.o` — pass.
- Full `make -C port -j2` attempted; this environment lacks `sdl2`, `libpng`, and `gl` pkg-config packages / `SDL.h`, so linkable full build is unavailable here. Stopped after the missing dependency error; no GPU run.
- No current active-gameplay camera packet is asserted to cross this boundary; no matched Windows visual parity claim.

Software commit `f56ad84630cf912b00dc4895f3617ab802c5b16c` pushed to `origin/feature/linux-renderer-foundation`. This private report remains local/untracked; unrelated dirty files were preserved.

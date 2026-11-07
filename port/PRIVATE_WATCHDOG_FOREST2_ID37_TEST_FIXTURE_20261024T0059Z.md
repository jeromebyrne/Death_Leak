# Forest-2 ID37 CPU draw-spy fixture — 2026-10-24 00:59Z

## Deliverable

Added only `port/tests/forest2_id37_emitter_boundary_cpu_test_20261024t0059z.cpp` (besides this unique report). It loads `forest_exploration_2.xml` with `DEATHLEAK_RENDER_ONLY_XML_IDS=37`, requires the authored `oak_leaf.png` texture and the sole spray's ID/depth/bounds/four staggered lifetimes, then replaces its particles with deterministic states. An independent Windows `Camera2D` emitter predicate rejects the emitter at Y=boundary−1 and at exact contact, while one warmed leaf still intersects the viewport. Direct `ParticleSpray::Draw` proves that negative control emits one leaf; `LevelScene::Draw` must emit zero off-emitter/contact and one at boundary+1, with flipped U and 80-world-unit size scaled to 53⅓ pixels. This catches both a missing loader admission and a loader-only change lacking emitter culling.

## Reproduce after exclusive GPU/build owner releases the lane

From repository root, with assertions enabled:

```sh
cpu_dir=$(mktemp -d /tmp/forest2-id37-cpu-XXXXXX)
g++ -O2 -g -std=c++17 -Wall -Wextra -Wpedantic -UNDEBUG -Iport/compat \
  port/tests/forest2_id37_emitter_boundary_cpu_test_20261024t0059z.cpp \
  port/game/LevelScene.cpp port/game/PlayerMotion.cpp \
  Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp \
  Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp \
  -o "$cpu_dir/forest2_id37_emitter_boundary_cpu_test"
env -u DEATHLEAK_RENDER_HIDE_XML_IDS "$cpu_dir/forest2_id37_emitter_boundary_cpu_test"
```

**Expected on current source:** assertion at `std::count(spy.loaded..., leafAsset) == 1` fails because `LevelScene.cpp` admits only Forest-1 ID47 and waterfall-top IDs49–51. After a Forest-2/ID37 loader admission alone, the off-emitter `scene.Draw` zero-command assertion is expected to fail because its current emitter gate covers only ID47. After both scoped changes, expected stdout is `PASS: Forest-2 ID37 authored admission and emitter-boundary draw gate`.

## Execution / ownership

Not compiled or executed: exclusive ID47 owner was building/capturing, and this lane was instructed not to run Docker/build/GPU. No source, renderer, Makefile, integration script, shared status, or other worker files were edited; no commit or push. Fixture is unverified until the command above is run after owner release. It is command/source parity only; no claim of Forest-2 visual magnitude or pixel parity.

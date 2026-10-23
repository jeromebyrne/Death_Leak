# Forest small-orb hover/trail implementation — 2026-10-22

Scope: authored non-native `CurrencyOrb`s in forest-1 and waterfall-top only. No XML, spawned/native orb, collection-radius, integration script, Docker build, physical GPU, or shared build output change.

## Source changes

- `port/game/LevelScene.cpp/.h`: admits forest-1 small orbs into the existing sine/strict-130 tracking path; preloads `Game/Media/orb_shatter.png` with explicit load failure; creates one stable-keyed 15-particle runtime spray at the hovered false→true tracking edge. Uses source depth 3250, bounds 3200×1200, signed radial directions, ±30 source spawn spread, staggered .35→.1167 s lifetimes, speed 1.51–3.40, size 41–80, gravity .5, brightness .81–1, scale-to .1, fade-in .15, fade-out .8, infinite attached loop. A per-key Linux seed is deterministic but does not claim Windows RNG parity. Runtime spray is not XML-warmed. Attachment follows post-motion orb XY only on particle recycle; collection/ApplyCollected detaches and stops looping, residual particles drain, and scene reload clears trails. Separate kOrb painter stream draws after authored kOrb sprites and before same-depth dynamic items; diagnostic-only-ID filtering suppresses untagged trails.
- `port/game/ParticleSpray.h`: records pre-step `drawSize` with pre-step XY and draws that snapshot; optional parent XY changes only attached recycle. Existing XML spray update/warm path remains default.
- `port/tests/orb_trail_test.cpp`: focused CPU scene/render spy for named forest ID70 and waterfall ID75, strict acquisition/pickup boundaries, one keyed 15-particle spray, source constants, tracking, attached recycle to exact current XY, detach/drain, saved collection, and previous-step quad size.

## Verification

- Host `g++ -O0 -std=c++17 -Wall -Wextra -Wpedantic -Iport/compat` linking the focused test with `LevelScene.cpp`, `Combat.cpp`, and TinyXML into `/tmp/orb_trail_cpu`: `forest` **PASS**, `waterfall` **PASS**. Only pre-existing TinyXML parser compiler warnings.
- Existing `waterfall_orb_hover_tracking_test.cpp` linked similarly into `/tmp/waterfall_orb_trail_cpu`: **PASS**.
- `git diff --check` on scoped source files: **PASS**.

No post-change physical-GPU capture or matched Windows pixel comparison was attempted; coordinator owns that reservation. The runtime same-depth dynamic-item ordering is a deterministic port rule, not a proven Windows insertion-order match. The CPU gate did not assert whole-scene pixels or broad integration. Existing untracked reports and the village worker's `port/tests/integration.sh` change remain untouched.

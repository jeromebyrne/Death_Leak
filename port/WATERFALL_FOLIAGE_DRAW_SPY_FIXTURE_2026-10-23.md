# Waterfall-top foliage phase-centre CPU draw spy — 2026-10-23

## Delivery

Added the persistent, standalone `port/tests/waterfall_foliage_phase_centre_draw_spy_test.cpp`. It replaces the need to rely on the earlier **ignored** diagnostic at `port/build/waterfall-foliage-phase-2026-10-23/draw_spy.cpp`: the new fixture is an assertive two-mode gate, not a second production implementation. No production source, renderer, shared script/status, GPU/Xorg, or physical display was touched.

The spy loads the actual `Game/XmlFiles/levels/waterfall_top.xml`, filters one authored XML ID per process because LevelScene's diagnostic filter is process-static, and captures real submitted `render::Sprite` commands. It checks IDs 64/65 at two visible cameras each, ID60 as a non-parallax/X-flipped control, and times 0 and 0.5. It asserts exactly one submission, independently derived destination and parallax world centre, authored corner intensities, time/global factor, flip, and independently evaluated source-bottom sine displacement. `baseline` expects the current 64/65 mismatch; `fixed` expects the eventual correction. A failed fixed run is an expected negative control today.

## Exact isolated reproduction

From repository root; this produces only a uniquely named binary in `/tmp`, not a shared game build:

```sh
g++ -O0 -std=c++17 -UNDEBUG -Iport/compat \
  port/tests/waterfall_foliage_phase_centre_draw_spy_test.cpp \
  port/game/LevelScene.cpp port/game/Combat.cpp port/game/PlayerMotion.cpp \
  Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp \
  Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp \
  -o /tmp/waterfall_foliage_phase_centre_draw_spy_20261023
for id in 64 65 60; do
  /tmp/waterfall_foliage_phase_centre_draw_spy_20261023 "$id" baseline
done
```

Verified: compile exit 0; all three baseline invocations exit 0 and print `FOLIAGE_PHASE_DRAW_SPY=PASS mode=baseline ID=64|65|60`. For both cameras and both times, IDs 64/65 print `centre=MISMATCH bottom_phase=MISMATCH`, while ID60 prints `centre=MATCH bottom_phase=MATCH`. `/tmp/waterfall_foliage_phase_centre_draw_spy_20261023 64 fixed` exits 1 and prints `FOLIAGE_PHASE_DRAW_SPY=FAIL mode=fixed ID=64`, confirming the gate rejects the currently uncorrected centre.

After the separate LevelScene ownership handoff and narrow production fix, run `fixed` for all three IDs; every invocation must exit 0 and print `FOLIAGE_PHASE_DRAW_SPY=PASS mode=fixed ID=<id>` with all four rows per ID at `centre=MATCH bottom_phase=MATCH`. The direct Draw calls prove the active port draw-command contract, not original Windows paused/out-of-update-zone cadence. This is CPU evidence only, not visual acceptance or pixel parity.

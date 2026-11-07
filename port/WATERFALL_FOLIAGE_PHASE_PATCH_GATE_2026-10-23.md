# Waterfall foliage phase patch gate — 2026-10-23

## Current gate

Inspected HEAD `ef69c5b4aaa9ee249685e9982ab8c7dad081ea31` (`Add waterfall foliage phase-centre CPU draw spy`). The committed draw spy is present, but `port/game/LevelScene.cpp` is still dirty with an unrelated right-stick aim-line patch and still submits `object.foliageSway.worldX/Y` from authored XML position. No ID64/65 phase-centre production fix is present. The most recent ownership resolution (`AIM_LINE_OWNER_RELEASE_RESOLUTION_2026-10-23.md`) explicitly withholds transfer: focused CPU passes and a plausible handoff report do not authenticate release of the neutral aim-line owner's dirty `LevelScene` work. This report authorizes no second writer to that file. Coordinator must obtain that owner's explicit completion/release (or explicitly assume/reconcile ownership after independent scoped review), identify the accepted patch, re-read status/diff, and assign one exclusive integration owner before the change below. Preserve the aim-line changes, including its adjacent Draw hunk; do not stage or commit the entire mixed dirty file opportunistically.

## Exact one-site production delta once released

In `LevelScene::Draw`, after the existing `draw.foliageSway = object.foliageSway;` and before setting its time/global multiplier, insert only:

```cpp
if (draw.foliageSway.enabled && !object.player && !object.playerShadow &&
    !object.playerAimLine && !object.parallaxLayer) {
    draw.foliageSway.worldX = worldX - (cameraX - worldX) * (layerX - 1.f);
    draw.foliageSway.worldY = worldY - (cameraY - worldY) * (layerY - 1.f);
}
```

Use the already selected `worldX/Y` and depth multipliers `layerX/Y` from the destination calculation; no new state, loader rewrite, geometry/renderer, shader-clock, weather, culling, UV, tint, flip, rotation, or painter-order edit. Windows `GameObject::Update` translates `World` by `(camera-position)*(multiplier-1)` and `FoliageSway.fx` samples the transformed vertex before its sine; Linux currently translates the destination but leaves the shader centre at XML position. Waterfall-top IDs 64/65 are ordinary depth-1500 `kFarForeground` sprites (`1.10,1.05`), so the proposed centre follows their camera-dependent destination. ID60 is ordinary depth-1750 `kGroundFront` (`1,1`) and must remain exactly at its XML centre despite its authored X flip. The active-draw correction is deliberately narrower than Windows pause/out-of-update-zone temporal parity, which needs a last-successful-update camera/state trace.

## Fixed-mode release test

The committed `port/tests/waterfall_foliage_phase_centre_draw_spy_test.cpp` loads actual `waterfall_top.xml`, observes submitted commands, uses two visible cameras and times 0/0.5 for each ID, and filters one XML ID per process. From repo root, after the single owner applies the delta, compile its isolated binary and run **fixed** mode:

```sh
g++ -O0 -std=c++17 -UNDEBUG -Iport/compat \
  port/tests/waterfall_foliage_phase_centre_draw_spy_test.cpp \
  port/game/LevelScene.cpp port/game/Combat.cpp port/game/PlayerMotion.cpp \
  Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp \
  Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp \
  -o /tmp/waterfall_foliage_phase_centre_draw_spy_20261023
for id in 64 65 60; do
  /tmp/waterfall_foliage_phase_centre_draw_spy_20261023 "$id" fixed || exit 1
done
```

Gate: all three print `FOLIAGE_PHASE_DRAW_SPY=PASS mode=fixed ID=<id>`; all four camera/time rows per ID say `centre=MATCH bottom_phase=MATCH`. The prepatch `baseline` pass and prepatch ID64 `fixed` failure are already documented in `WATERFALL_FOLIAGE_DRAW_SPY_FIXTURE_2026-10-23.md`; they are negative-control evidence, not postpatch results. Re-run focused aim-line/strafe regressions on the integrated tree, then review the scoped diff before any build/GPU lane handoff. The spy proves CPU draw-command phase only.

## Visual GPU acceptance discriminator, after CPU and ownership gates

Use the exclusive physical GPU lane and the same frozen committed build/state for a lossless **control-vs-fix** waterfall-top comparison: identical 1280×720 viewport, per-ID visible camera (`64: (0,3000)`; `65: (1000,3000)`; `60: (-1500,3000)`), shader time 0 and 0.5, global foliage multiplier 1, weather, texture/filter, player/camera, and frame state. Capture ID-isolated foliage crops plus full-scene context on prepatch and patched builds; log actual draw centre/destination and capture parameters. Expected discriminator: IDs 64/65 retain the same destination/top edge/texture placement but their **bottom-edge X sway displacement** changes to the independently predicted phase, while ID60 and top-corner displacement remain invariant. An image difference alone is insufficient if registration, time/weather, or scene state drift; no visual-severity rank follows from the CPU spy. A matched original-Windows lossless frame/sequence at equivalent object update state is additionally required to claim Windows pixel parity, and Jerome's visual acceptance remains separate.

## Risks and status

The fixed draw-centre formula samples current draw camera, whereas Windows may retain the camera from an object's last successful `Update` when paused or outside update zone. ID65 and ID60 are culled at initial camera `(0,3000)`, so an all-ID initial-camera spy is invalid. The foliage sine uses source-bottom world Y (`centreY - 338.5`) and ID60's X flip changes source-corner labels; do not infer phase from centre alone or screen-left labels. Existing dirty `LevelScene.cpp/.h`, `main.cpp`, shared status, and unrelated fixture changes were left untouched. This was a read-only source/ownership review except for this new report: no build, test execution, GPU capture, source edit, commit, or push was performed.

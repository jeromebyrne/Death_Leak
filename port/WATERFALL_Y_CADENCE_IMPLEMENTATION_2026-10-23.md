# Waterfall Y-cadence implementation (2026-10-23)

## Verdict

**CPU cadence implemented and verified for authored waterfall-top ParallaxLayer IDs 6 and 13.** The port now advances their stored V once per `LevelScene::Draw`, using the modified delta associated with the **preceding rendered scene draw**. The first draw of a fresh `LevelScene` starts at V=0. A reload of the same scene instance resets each authored layer's V to zero while retaining the preceding render delta, as Windows retains process-owned `Timing::LastUpdateDelta` across level loads. Modal object-update pause does not pause the scroll. No physical GPU/Xorg or image capture was used; RGB parity is not claimed.

## Source and implementation

- `Game/src/GameObjects/ParallaxLayer.cpp:114-150`: `ParallaxLayer::Draw` changes vertex V using `GetLastUpdateDelta()/GetTargetDelta()`; `ParallaxLayer::Scale` scales Y speed from 1080 authored units to 720 output units.
- `Game/src/program.cpp:149-177`: `Render()` occurs before `SetLastUpdateDelta(delta * timeModifier)`; `Game/src/Core/Timing.cpp:8` initializes that delta to zero.
- `port/game/LevelScene.cpp`: `UpdateVisuals` records the latest modified update delta even with `objectsPaused`; `Draw` advances auto-scrolling layers with the previously rendered modified delta, then stores the current pending delta after drawing. It mutates V at the layer's Draw path before viewport culling, matching the source's `ParallaxLayer::Draw` ordering. No draw means no V advance. Several updates without a draw do not accumulate V. Several draws without an update each advance with the preceding draw's stored delta.
- The port's diagnostic XML-ID filter still suppresses filtered objects; that is an opt-in diagnostic path, not Windows production behavior. `LevelScene` has scene-local timing because the owned edit surface excludes the process clock/main loop. A brand-new scene instance starts with zero; the production main loop reuses its scene instance on level loads, preserving render history as above.

## Verification

The focused existing `port/tests/waterfall_autoscroll_test.cpp` was updated to assert actual spy submissions for IDs 6 and 13. With modified deltas `[1/60, 1/60, 1/120 (modal), 1/60]`, captured V values are respectively `0, 1, 2, 2.5` times each ID's scaled 60-Hz step (`7*(720/1080)*.001` and `11*(720/1080)*.001`). It additionally checks multiple updates/no draw, two draws/no update, and same-instance reload resetting V but retaining render delta; destinations, alpha, flips, and wobble-enable stay fixed. The no-GPU test passed via direct g++/TinyXML CPU build. A full `deathleak-linux-build:bookworm` Docker `make -C port -j1` build passed (only pre-existing TinyXML warnings).

The older `waterfall_wobble_uv_test.cpp` has update-timed V assertions and is not part of this focused cadence verification; it will require a separately scoped expectation update before treating it as a cadence regression gate.

## Boundary

This closes the source-backed CPU submission-cadence mismatch identified in `WATERFALL_Y_CADENCE_CPU_DRAW_SPY_2026-10-23.md`. It does not establish pixel magnitude or Windows binary image parity. The reserved exclusive GPU lane can later verify a frozen-camera, variable-delta rendered sequence without sharing this implementation's CPU-only lane.

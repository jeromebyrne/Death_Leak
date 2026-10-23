# Forest preceding-rendered-delta animation integration — 2026-10-23

## Result

Integrated one process-owned, zero-initialized preceding rendered modified-delta slot in `port/linux/main.cpp`; it is committed immediately after `renderer->EndFrame()`. The port currently has no damage time modifier, so its `dt` is the current modified delta. Only LevelScene's ID68/other authored sprite animation, ID2 body animation, and finite firing-arm animation consume the previous slot. Physics, weather, shader, orb hover, spray, aim-line hold/fade, and other visual timers remain on current `dt`. The separate waterfall Y-scroll scene clock was not changed. A simulated-only iteration does not commit a prior rendered delta; menu and off-zone rendered iterations do.

`UpdateVisuals` and `UpdatePlayerVisual` accept an explicit animation delta at the end of their existing signatures, preserving existing direct-call fixture behavior via a default current-delta fallback. The production main loop always supplies the process slot. Sequence-entry reset, hold-falling, velocity/strafe rates, update-zone freeze/re-entry, at-most-one-frame advancement, firing restart/two advances, and finite arm suppression are unchanged.

## Verification

- New actual-XML CPU command spy: `port/tests/forest_preceding_delta_cpu_test.cpp`. It loads forest ID2 and ID68 from the authored files, checks exact emitted Still/Running and orb texture paths on all 20 controlled ticks, including ID2 `_000` at ticks 3/13 and `_001` at 4/14, ID68 freeze on off-zone ticks 6–7, re-entry at tick 8, and continued prior-slot progression while off-zone. Still→Running→Still resets are asserted. It commits its independent previous-delta variable only after each `Draw`/`EndFrame`. The same spy confirms a successful shot with prior `.001` emits arm `_000` despite current `.0166667`, and that aim-line alpha remains on current-delta fade. **PASS** (isolated `/tmp` host CPU executable).
- Adjacent `orb_animation_zone_test` and `player_arm_parity_test` CPU fixtures: **PASS**. Adjacent `aim_line_parity_test` fails at its pre-existing left-facing neutral-flip expectation (`:59`); this patch does not touch aim-line orientation, and the new focused spy verifies its current-delta alpha behavior. This unrelated failure was not papered over or changed.
- `git diff --check` for scoped source: **PASS**. Post-GPU-owner-handoff serial Docker production build `make -C port -B -j1 all`: **PASS**, TinyXML warnings only. No physical GPU/Xorg/capture was used by this lane.
- The GPU owner first landed `port/WATERFALL_FOLIAGE_CURRENT_HEAD_GPU_REVIEW_2026-10-23.md`, explicitly releasing the frozen HEAD capture. Its visual result applies to the **pre-animation** frozen binary, not this new build.

## Scope and remaining acceptance

The scoped software commit contains only `port/linux/main.cpp`, `port/game/LevelScene.cpp`, `port/game/LevelScene.h`, the new CPU spy, and this handoff. Unrelated dirty files, shared integration/status files, XML/assets, renderer, and foliage were untouched. This source-model command-path result does **not** establish matched-frame original-Windows pixel parity or Jerome's visual acceptance. A later exclusive GPU lane needs lossless matched Windows/Linux active-play ID2/ID68 frames and timing/state sidecars before making either claim.

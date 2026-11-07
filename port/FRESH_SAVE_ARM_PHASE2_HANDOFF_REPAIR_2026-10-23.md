# Fresh-save arm phase-2 handoff repair — 2026-10-23

## Result

The no-shot Pull→ordinary handoff now retains the intro arm's finished state. On entering either intro body phase, `SetIntroBodyPhase` selects the corresponding authored empty arm sequence (represented by its name with no loaded frames). Exiting the phase leaves it finished; the first ordinary `UpdatePlayerVisual` switches that arm to the selected body sequence and marks it finished. Ordinary idle ticks therefore cannot draw `back_arm_attack_000` without a subsequent restart. A later successful shot restarts the ordinary Still arm and preserves arm-before-body draw order. The existing authored 40-fps Pull body rate and intro arm draw suppression were not changed in this repair.

## Focused evidence

- Original `player_anim_default.xml` has empty `IntroCutscene1`/`IntroCutscene2` arm sequences (`:1532-1536`); original `Player::UpdateAnimations` switches/finishes the arm (`Game/src/GameObjects/Player.cpp:963-975`), and `Character::UpdateAnimations` switches/finishes it on ordinary body selection (`Game/src/GameObjects/Character.cpp:1099-1112`). `Character::Draw` skips finished arms (`:1888-1907`).
- Added a no-shot first ordinary tick and five subsequent idle ticks to `port/tests/fresh_save_intro_animation_test.cpp` before its later shot-positive assertion. The spy checks ID2 body frame/path and absence of all arm commands on each tick, then arm-before-body after the shot. It continues to exercise 15-fps Looped, 40-fps Pull, variable preceding-rendered deltas, and retained final Pull frame.
- Test-first red: with the new no-shot assertions and old scene code, `/tmp/fresh_save_arm_handoff_test` aborted at the arm-presence assertion (`fresh_save_intro_animation_test.cpp:59`). A subsequent intermediate run exposed an erroneous test expectation that ordinary idle remains on frame zero; the fixture now tracks the XML's 20-fps Still body rate.
- Final focused CPU command, run from repository root with output only in `/tmp`: `g++ -O2 -std=c++17 -UNDEBUG -Iport/compat port/tests/fresh_save_intro_animation_test.cpp port/game/LevelScene.cpp port/game/PlayerMotion.cpp Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp -o /tmp/fresh_save_arm_handoff_test && /tmp/fresh_save_arm_handoff_test`. Output: `fresh-save intro animation CPU test passed`; exit 0. `git diff --check -- port/game/LevelScene.cpp port/tests/fresh_save_intro_animation_test.cpp` passed.

## Boundary and ownership

This is command-path/clock CPU evidence only. It does not establish same-history Windows timing, occluded pixels, physical GPU parity, or the main-loop held-Interact/save-flag integration. The earlier QA's separate intro fall/grounded handoff remains open. No shared `port/build`, physical GPU `:0`, Combat, main, renderer, or other dirty files were touched; no commit or push. **Exclusive ownership of `port/game/LevelScene.cpp`, `port/tests/fresh_save_intro_animation_test.cpp`, and this report is explicitly released.**

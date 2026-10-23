# Range-kill integration regression — 2026-10-23

Scope: CPU/offscreen-Mesa combat integration fixture; no physical GPU/Xorg. The shared `port/build/deathleak-linux` was **not rebuilt or overwritten** while a GPU capture lane was active. No gameplay source was changed.

## Cause

The rerun-3 `encounter_roll.xml --script B:330` assertion expected one kill, but the endpoint was `spawned=1 enemies=1 hits=14 kills=0 fired=20 shots=8`, player `(2612,107.449)`, health 22. Source assigns the ninja 20 health (`port/game/Combat.cpp`, encounter spawn) and ordinary knives 1.5 damage each; `hits` also counts enemy projectiles that strike the player. Thirteen knife contacts plus one player hit produce 19.5 ninja damage, so the ninja correctly survives. The player continues right at 480 units/s, while the stunned ninja is left behind (at frame 330: ninja x=1980), and further right-facing shots cannot finish it. This is a stale scripted encounter path, not evidence for changing ninja health or knife damage.

Focused frame probes: at B:220 the player is close enough to the ninja to hold position; B:240 already leaves it behind. A repaired `B:220,F:120` sequence retains the 340-frame encounter window, stops horizontal movement and keeps firing toward the target. The focused executable endpoint was `player=(1763.54,107.449) health=22 spawned=1 enemies=0 hits=15 kills=1 fired=20 shots=2`. Thus it adds the one lethal knife contact while preserving the one player hit and the normal-health enemy.

## Change and verification

`port/tests/integration.sh` now uses `B:220,F:120` for only the `range_kill` fixture and keeps the `spawned=1 enemies=0 ... kills=1` requirement. The script lines were already present in the shared dirty worktree when this lane reached the edit; this lane independently verified them and did not overwrite or restage any other integration-file changes. The focused gate passed on the frozen shared executable using `SDL_VIDEODRIVER=offscreen`, `SDL_RENDER_DRIVER=opengl`, and `DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1`. `git diff --check -- port/tests/integration.sh` passed. The earlier full-suite rerun stopped here; a later full integration run has not been claimed as passing by this report.

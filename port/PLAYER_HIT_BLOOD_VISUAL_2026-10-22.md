# Player-hit blood projectile: bounded active-flight CPU slice (2026-10-22)

## Implemented

- In `Combat.h/.cpp`, an accepted enemy projectile/player hit that actually lowers health now spawns exactly one independent `blood_projectile.png` flight at the player's XY, depth 3000. The health/hurt-cooldown/shot-removal path is otherwise unchanged.
- A fixed Linux-only RNG chooses the original `256*(0.5+rand()%50*0.01)` size range (128–250.88 world units), without consuming global random state. The visual starts with zero alpha/scale, grows and fades in linearly to full size/alpha by 0.3 s, and uses source active blood gravity of `0.5 * min(2, dt*60)` velocity units and matching nominal-tick displacement. It does not rotate.
- Blood flight is independent of the RGB damage burst, knife shots, and ninja material sprays; `ClearLevel` clears it. A 10-second *port-only collection bound* prevents unbounded orphaned flights until collision/impact lifecycle is owned. This is not a source active lifetime.

## CPU verification

`port/tests/player_hit_blood_visual_test.cpp` uses a fixed shot at the accepted collision edge and a draw spy. It passed after compiling against current `Combat.cpp`, `LevelScene.cpp`, and TinyXML with g++ C++17 and `-UNDEBUG`. Assertions cover one initial zero-alpha draw at player origin, source size range/depth-by-painter insertion, 0.1/0.3-second alpha and scale, nominal-tick vertical motion, hurt-gate rejection, zero-damage non-spawn, a second accepted hit, and level clear. The pending `player_shot_launch_state_test.cpp` also passed against this worktree. `git diff --check` passed for the two production files.

## Boundaries and handoff

This does **not** implement solid-line/character impact detection, impact texture variant, Y jitter, ground-blood depth 4000, 5.5-second inactive envelope, or solid-line material spray. It also does not alter solid hazard/drowning/direct `ApplySolidContactDamage` routes, because those currently lack an explicit player XY at their damage call edge. The port's existing zero-damage projectile contact still consumes the projectile and starts hurt cooldown; this slice only suppresses its blood spawn. A follow-up should preserve the exact collision winner and implement the source impact/inactive lifecycle in a separately owned seam.

No GPU/display, Docker, SDLRenderer, main, shared status, or unrelated files were changed. `Combat.h/.cpp` already held another owner's uncommitted player-shot launch-state changes before this task, and the tree has many unrelated pending edits; this slice was **not committed or pushed** to avoid folding that owner's changes into a scoped commit. Stage only this slice's hunks after that owner lands/coordinates, then rerun the focused CPU fixture.

# Ninja lethal visual-effect slice — 2026-10-22

## Scope and source contract

Implemented only the lethal `kNinja` effect queue in `port/game/Combat.cpp/.h`. The ordinary health, hit, kill, and enemy-erasure paths remain unchanged; XML, assets, forest, waterfall, and shared status files were not edited. `NPC::DoNinjaExplosion` (`Game/src/GameObjects/NPC.cpp:524-609`) supplies the exact ten texture entries/duplicate counts and speed multipliers; `SpawnLimb` starts them at `(enemy.x, enemy.y+50)`. The source also creates five `explosion_lines.png` particles at enemy position at ground-front depth. The port now loads those authored files, emits on the first lethal crossing only, draws the ten debris at the combat actor depth and five lines at ground-front depth, moves them after enemy erasure, and clears them on level exit. Debris drawing uses the PNGs' native pixel dimensions (the source passes 80x80 to `Debris`, but `nativeDimensions=true`); exact source rendering scale still needs a Windows comparison.

## Verification

- Docker image `deathleak-linux-build:bookworm`: `make -C port -j2` completed. Only existing TinyXML warnings appeared.
- Focused `port/tests/ninja_death_effect_test.cpp` built and passed in that image. It checks no effect after a nonlethal hit; one lethal hit creates exactly 10 debris and five spray entries; a repeat hit cannot duplicate; all 15 draw after body erasure, debris move, and all expire.
- Authored arena simulation: `--play Game/XmlFiles/levels/first_ninja_arena.xml --start-encounter 0 --script F:300 --fixed-dt 0.0166667 --simulate-only --combat-state --frames 300`, with `DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1`, ended `spawned=2 enemies=1 hits=14 kills=1`. This confirms the route reaches a kill; it does not prove effect pixels.
- `git diff --check` passed for the scoped source. Physical-GPU capture could not run in this sandbox: `DISPLAY` is unset and the host LightDM Xauthority path `/var/run/lightdm/root/:0` is unavailable. The available `/tmp/.X11-unix/X0` and DRM nodes alone do not establish an authenticated physical accelerated renderer. No software screenshot is promoted as GPU evidence.

## Remaining parity limits and lane release

This is an event-presence slice, **not** full Windows visual parity or game acceptance. Debris uses deterministic representative source-range initial speeds and approximate gravity/resistance/rotation, with a bounded 4.2-second fade; source terrain bounce/collision-dependent fade and RNG are not reproduced. Spray uses the authored count, image, depth, size/lifetime envelope and approximate directed spread, not the original particle update/fade trajectory. Common `Character::OnDamage` death particles are outside this slice. A matched Windows kill crop and authenticated physical-GPU before/after capture remain unverified. The Docker build/test lane is released; no active GPU capture lane was acquired.

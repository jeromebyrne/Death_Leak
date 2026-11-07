# Unflipped rotation visual-contract audit — 2026-10-22

## Verdict

The in-flight ordinary-sprite correction is **source-consistent for forest-1 ID15**: authored X=false/Y=false and −1.469999 rad require SDL angle **+1.469999 rad** (about +84.225°) to reproduce the Windows UV-labeled corners. This is a CPU/source contract, not a measured Windows image match. One independent edge condition remains: **authored vertical flip also conjugates rotation**. A helper keyed only on horizontal flip fixes ID15 but predicts the wrong sign for rotated, vertically flipped ordinary sprites elsewhere. No ID15 parity claim is possible without a matched original-renderer capture.

## Contract and evidence

`Game/src/GameObjects/Sprite.cpp:19-22,215-223` maps source V=0 to world-bottom; `:89-96` implements authored X/Y flips as negative scales. `Game/src/GameObjects/GameObject.cpp:202-210` forms row-vector `R*S*T`, and `Game/ShaderFiles/BasicLightTexture.fx:37-46` applies it. In screen coordinates, with authored signs `sx,sy ∈ {+1,−1}`, Windows maps UV-labeled local points by `F·A(−θ)`, where `F=diag(sx,sy)` and `A` is clockwise screen rotation. `port/game/LevelScene.cpp:997-1012` correctly inverts the authored Y flag for an ordinary SDL copy and currently computes the angle from X flip alone. `port/linux/SDLRenderer.cpp:207-208,245-249` passes both flip flags and that angle to centered `SDL_RenderCopyExF`, which maps the same points by `A(φ)·F`. Equality requires **φ = −sx·sy·θ** (modulo full turns), i.e. reverse the sign when X and Y flip parity is even; preserve it when exactly one is flipped. This assumes the ordinary SDL copy path, not custom foliage/pixel-wobble geometry.

`Game/XmlFiles/levels/forest_exploration_1.xml:237-249` makes ID15 a 512², no-flip ordinary light-texture sprite at `(2796.985352,3207)`, θ=−1.469999. Thus `port/render/OrdinarySpriteRotation.h:8-10` currently gives the correct ID15 sign, while its line-6 statement that *only unflipped* sprites need reversal is too broad. The four-ID test `port/tests/ordinary_sprite_rotation_contract_test.cpp:8-49` fixes authored Y=false in both corner functions and has no Y-flipped case. It can pass while this edge fails.

Concrete ordinary, non-repeating counterexamples are `Game/XmlFiles/levels/underwater_cave_4.xml:247-274`: ID15 X=true/Y=true and ID16 X=false/Y=true, both `sun_shaft_2.png`, θ=−1.680533, `effectlighttexture`. Correct SDL signs are **+1.680533** for ID15 (both flips), **−1.680533** for ID16 (Y only); the current X-only helper produces the opposite signs for both. `Game/XmlFiles/levels/cliff.xml:1316-1328` has another both-flipped rotated ordinary blade, and `Game/XmlFiles/levels/grass_exploration_2.xml:1341-1353` a Y-only post. These establish that the edge is authored, not hypothetical; no visual magnitude is asserted.

## Narrow next gate

Before generalizing the ID15 change, extend the CPU UV-labeled four-corner contract to all **four X/Y flip combinations**, using the underwater-cave pair as authored Y-flip controls; feed both authored flags (or equivalent flip parity) to the ordinary-angle decision. Keep shader-specific paths separately scoped (`port/linux/SDLRenderer.cpp:209-214,353-369,383-419`); this audit does not establish their parity. Then seek a lossless original Windows forest-1 **right-exit** frame matched in camera/state/viewport/time and an isolated ID15 layer before accepting ID15 visual parity. The existing Steam screenshot 14 and Linux captures do not supply that match, as documented in `FOREST_ID15_MATCHED_REFERENCE_2026-10-22.md`.

This report alone was written. No source edit, build, GPU/display/game capture, commit, shared checkpoint edit, or visual-parity acceptance was performed.

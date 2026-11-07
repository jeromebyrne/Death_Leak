# Waterfall-top unowned visual source audit — 2026-10-23 23:37Z

## Verdict

**No concrete original-Windows-versus-Linux visible mismatch established in this bounded source pass.** The distinct active-gameplay candidate examined is the left-entry foreground branch cluster, especially IDs 53/54 (not the separately reviewed raft-tip ID52). Source data supports its intended geometry, opacity, and placement in both implementations; neither a matched original-Windows active frame nor an isolated Linux ID53/54 pixel comparison is available here. This is not a parity claim and nominates no patch.

## Source-backed contract

- `Game/XmlFiles/levels/waterfall_top.xml:99-108` puts the Forest-arrival entry ID5 at x=-2191.014648, y=3131, facing right. During ensuing rightward active travel, the branch sprites ID53 and ID54 are authored at x=-1062.115356/y=3392.101074 and x=-326.214417/y=3673.001953 (`:914-944`). ID53 uses `Media/objects/branch_3.png`, 395×167 native, unflipped; ID54 uses `branch_2.png`, 531×264 native, horizontal flip. Both are `kGroundFront`, alpha 1, `effectlighttexture`, and explicitly `fade_when_player_occluded=false`.
- Original `Game/src/GameObjects/Sprite.cpp:369-422,423-456` runs the ordinary Sprite draw/effect dispatch and reads texture, flip, and native dimensions; `Game/src/GameObjects/DrawableObject.cpp:54-117,180-182` makes overlap fading conditional on its authored flag. Thus these branches are opaque authored scenery, even when in front of the player. No special branch animation or fade is specified.
- Port `port/game/LevelScene.cpp:380-405,495-503,545-579` resolves authored sprite texture, position, dimensions, depth, flip, alpha, and fade flag; `:915-946` only performs the fade update when enabled; `:1360-1430` submits the ordinary sprite using those fields. This source comparison identifies no ID53/54 omission or contradictory flag. The branch asset/dimension inventory exists in `port/VISIBLE_ID_CONTRACT_MANIFEST_2026-10-22.md:138-139`; it is not a matched visual result.

## Precise next discriminator

At a **matched active-gameplay camera/player state** where ID53 or ID54 is wholly visible, obtain one lossless native-Windows pre-HUD frame plus draw-time camera/viewport and the corresponding Linux frame with the same state. Crop the branch's opaque silhouette and compare its transformed bounding box, flip, and alpha against the XML values above, using nearby fixed scenery for registration. A missing or differently oriented branch then becomes a candidate specific to IDs 53/54; a matching silhouette closes only this branch contract. Unmatched Steam JPEG or wall-clock-matched video frames are insufficient for a source patch. This request does not depend on, access, or wait for the separately owned frame-424 packet.

## Scope

Read-only original source, XML, port gameplay parser/draw source, and existing report inspection. Only this unique report was written. No main.cpp, renderer, build, GPU/display, test, shared status, commit, or push action.

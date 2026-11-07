# Waterfall post-fade source gate — 2026-10-23 23:07Z

## Verdict

The distinct conspicuous Waterfall-top feature in the corrected clip's last three seconds is the broad blurred central foreground trunk/curtain that hides the traversing player by 12.8 s. **No source-supported defect or new patch is established.** Its non-fading occlusion is authored, unlike the separately owned Forest ID37 issue and Waterfall ID46 shaft. No implementation action is nominated from this unmatched H.264 footage.

## Bounded evidence

- `PRIVATE_CORRECTED_FADE_ACTIVE_GAMEPLAY_VIDEO_20261023T2231Z.md` identifies the 13.000 s/390-frame corrected-fade clip and its strict accelerated capture. I inspected saved decoded stills at 10.5, 11.5, 12.0 and 12.8 s. A soft, dark near-vertical trunk occupies the middle while the player moves right behind it; the large sharp trunk remains at right. The clip has no same-state original-Windows counterpart, so appearance alone cannot prove a parity residual.
- Destination XML `Game/XmlFiles/levels/waterfall_top.xml:533-547` authors **ID29** at x=-1605.014648, y=3505, 1112×2743, `kFarForeground`, `tree2_blur.png`, alpha .95, `effectlighttexture`, and `fade_when_player_occluded=false`. That placement/asset/depth is consistent with the central soft trunk in this entry-side camera view; this is an attribution from authored geometry, not an isolated draw capture. The separate sharp right tree is consistent with ID23 at x=-760 (`:443-457`), whose fade flag is *true*. Neither is ID46.
- Original `Game/src/GameObjects/DrawableObject.cpp:54-117,180-182` changes alpha on overlap **only when** the XML fade flag is true; `Game/src/GameObjects/GameObjectManager.cpp:302-329` draws visible objects in ordered depth. `Game/ShaderFiles/BasicLightTexture.fx:53-59` multiplies sampled alpha by object alpha. The port parses the same flag/alpha (`port/game/LevelScene.cpp:572-575`), only runs its occlusion update when enabled (`:919-946`), and submits authored depth/opacity (`:1360-1407`). Thus ID29 remaining opaque as the player crosses is the source contract, not evidence that the port missed an enabled fade.

## Disposition and scope

Do not change ID29 alpha/fade or renderer layering based on this clip. A future candidate would need an identity- and camera-matched original-Windows lossless frame with ordered ID29 draw state and pre/post-ID29 pixels showing a first divergence; no such packet is in this bounded evidence. Read-only/source-only audit; only this unique report written. No GPU, Docker, build, source/test/asset/shared-status/index changes or commit. ID37, ID46 and clip delivery remain outside this lane.

# Forest-1 player draw-order source audit — 2026-10-23

**Verdict: no player-silhouette/contrast source change is justified by this draw-order audit.** The original and port put the player at `kPlayer=3000`, behind `kGroundFront=1750` and all foreground layers, and both fade the authored tree ID 6 when its rectangle contains the player. This is a source-path finding, not a matched-frame visual verdict or an attribution of individual pixels in the ~9 s clip.

## Original Windows path

- Forest-1 XML (`Game/XmlFiles/levels/forest_exploration_1.xml`) authors player ID 2 at `kPlayer`, alpha 1, `effectlighttexture`; tree ID 6 at `kGroundFront`, alpha 1, `fade_when_player_occluded=true`, with occluding alpha 0.4; lower shrubbery ID 37 at `kMiddleForeground`, alpha 0.95, without player fade. The foreground tree-top IDs 41–45 are `kGroundFront` and do not fade. ID 46 shadow is `kGround` (behind the player), alpha 0.5. Fog IDs 119/122 are `kGroundFront` at alpha 0.5/0.6; they too are in front of the player.
- `Game/src/GameObjects/GameObject.h:33–57` assigns depth values. `Game/src/GameObjects/GameObjectManager.cpp:58–65,302–330,1022–1025` sorts descending once and draws in that order, so high-numbered background draws first, then player, then low-numbered foreground. Drawables with alpha <= 0 are skipped. `Game/src/GameObjects/Character.cpp:1888–1908` draws the arm first, then the body; `Game/src/GameObjects/Player.cpp:1177–1180` delegates to it. The original `Sprite.cpp:636–637` sends its single `m_alpha` to the light-texture shader.
- `Game/src/GameObjects/DrawableObject.cpp:15,58–112` applies the authored occlusion fade when the player center lies within the sprite's world rectangle. It steps alpha by `1.5 * delta` toward 0.4 while occluded and back toward 1 otherwise. The commented-out depth guard does not constrain this mechanism to foreground objects, but ID 6 is foreground. `GameObjectManager.cpp:220–247` updates always-update objects or those intersecting the update zone.

## Linux port path

- `port/game/LevelScene.cpp:34–46,480–487,567–570,620` maps the same authored depth values, retains XML order, and stable-sorts descending. `LevelScene.cpp:1185–1235` interleaves dynamic/spray painters before depth crossings; the player remains in its authored `kPlayer` slot, before foreground sprites. `LevelScene.cpp:1300–1320` draws the player arm before its body, matching `Character::Draw`.
- `LevelScene.cpp:902–924` tests the same player-center/rectangle overlap for the authored fade, applies the same `1.5 * dt` step and next-update clamp, and updates rendered alpha. `LevelScene.cpp:1255–1280` passes the sprite alpha into the renderer. Ordinary SDL scene copies use `SDL_SetTextureAlphaMod(texture, sprite.tint.a)` at `port/linux/SDLRenderer.cpp:197–213`; the extra `opacity` field is not multiplied into that ordinary path. Thus ID 6's fade is not visibly squared by the ordinary renderer. Player ID 2 is also on that ordinary light-texture path in normal gameplay.

## 9-second implication and limits

- If the player center is within ID 6's authored rectangle (`x≈784–1896`, `y≈2523–5267` before any runtime scale), **both** paths should draw a fading tree over the player; outside it, both should restore alpha 1. Other foreground objects can still occlude/soften the silhouette even though their fade flag is false. This is intentional authored layering, not evidence of a Linux-only dark player.
- The current clip's exact player/camera coordinates and per-pixel writers were not established in this read-only lane; the separate GPU player-contrast discriminator owns those captures and ID attribution. The available original stills are not registered to this ~9 s forest-1 state. Therefore this audit cannot claim Windows-pixel parity, but it identifies **no concrete draw/depth/foreground-fade divergence affecting the player silhouette at that moment**. Do not retune player alpha, foreground alpha, depth, or fade from this source audit alone.

Scope: read original and port source/XML only; no build, game/GPU run, source/test/checkpoint edit, or shared file change. This report is the sole deliverable.

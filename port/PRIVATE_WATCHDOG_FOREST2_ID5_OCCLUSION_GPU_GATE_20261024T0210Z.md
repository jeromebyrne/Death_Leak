# Forest-2 ID5 foreground occlusion — source/physical-GPU discriminator, 2026-10-24 02:10Z

## Verdict

**Large reachable visual contribution, no newly justified correction.** Forest-2 XML ID5 is an authored `tree2.png` foreground sprite centered `(1339.985352,3895)`, size `1112.317383×2743.384033`, rotated `-0.079602` rad, alpha 1, horizontal-flipped, and flagged `fade_when_player_occluded=true` with target alpha .4 (`Game/XmlFiles/levels/forest_exploration_2.xml:101-115`). The original `DrawableObject::Update` checks the player's position against the sprite's unrotated bounds and adjusts opacity by `1.5*delta` (`Game/src/GameObjects/DrawableObject.cpp:54-118`). Port parsing and `UpdateVisuals` implement those same flags, bounds and step/clamp rule (`port/game/LevelScene.cpp:573-577,920-950`). This source audit finds no first divergent field to patch; changing ID5's art/geometry/alpha/occlusion policy would be speculative. This gate is distinct from ID37 leaves, ParallaxLayer prior-X, ScrollingSprite prior-world and orb hover/tracking.

## Serial physical active-play discriminator

From current HEAD `15fb8df`, current-tree binary SHA-256 `b709f6fb0ac5b45be063349b05fc7bca9326b7ebd6a7e5058352dd7c905401e7`, serial visible/`DEATHLEAK_RENDER_HIDE_XML_IDS=5` runs used the same `forest_exploration_2.xml`, `L:150`, 150 frames, fixed dt `.0166667`, Xorg `:0`, strict accelerated SDL OpenGL (`software=0`), direct `nouveau/NVE7`, and GPU `Sepia.fx`. Both ended in active Forest-2 with player `(1285.99,3110.82)`, two orbs, health 25, zero enemies/hits, grounded; only wall elapsed differs. Both loaded 62 scene sprites, zero missing/unsupported assets. Evidence and exact script/logs/lossless BMPs: ignored `port/build/private-forest2-id5-occlusion-20261024t0210z/`.

Visible vs hidden ID5 differs at **170,822/921,600** final RGB pixels, bbox `[477,33,816,687)` (Pillow any-channel nonzero). Center `(640,400)` is `(88,81,68)` vs `(98,90,74)`; outside-control `(400,450)` is identical `(87,80,68)`. This establishes a broad reachable screen contribution of the authored tree, *not* a Windows/Linux mismatch. At frame 150 the player lies inside ID5's authored unrotated bounds, and the port has had time to approach target fade. No original D3D10 same-state draw/alpha/pixel packet or human acceptance is available.

## Disposition

No code/assets/tests/runtime edit, Docker rebuild, scoped software commit or push warranted. `git diff --check` passed. Existing dirty renderer/tests/reports and `PORT_WORK_STATUS.md` were preserved. A genuine ID5 correction needs an original active Forest-2 same-update/draw packet with player/camera/ID5 matrix, alpha, texture/sampler, ordered pre/post-ID5 lossless crop and corresponding Linux packet; compare first command divergence before changing code. The physical GPU lane is released.

# Forest ID37/player legibility, current-head physical GPU — 2026-10-24 00:19Z

## Verdict

**The near-black lower band is a material, authored ID37 foreground contribution; at this moving Forest-1 pose it overlaps the player's lowest feet but does not account for loss of the upper body. No source-proven renderer mismatch or art/alpha/depth change was found.** This is a same-simulation-state Linux subtraction, not a matched original-Windows pixel-parity claim. A registered original D3D10 frame remains unavailable; the Steam Forest screenshot is only a qualitative scene reference.

## Frozen physical-NVE7 discriminator

Preflight: no live Death Leak, FFmpeg, capture or build container/process; only the standing Xorg `:0` server. Current HEAD `f7d04b4`; reused the freshly built post-ID67 active-gameplay executable at `port/build/private-post-id67-active-gameplay-20261024t0014z/deathleak-linux`, SHA-256 `8b9d4ae846b328999799a32e4b4debdd2b7acc4ffe349785bf61ec36f9675324`. The pre-existing unstaged renderer and test edits were neither built over nor changed. The executable was built at this HEAD with the default raft-mip experiment disabled, as documented in `PRIVATE_WATCHDOG_POST_ID67_ACTIVE_GAMEPLAY_GPU_REVIEW_20261024T0014Z.md`.

Ignored packet: `port/build/private-watchdog-forest-id37-player-legibility-20261024t0019z/`. Its `capture.sh` uses Forest-1, `--script B:360 --frames 360 --fixed-dt 0.0166667`, a lossless final BMP, and the render-only `DEATHLEAK_RENDER_HIDE_XML_IDS` filter. Three **serial** Docker runs (`baseline`, `hide37`, `hide2`) used `deathleak-capture:post-foliage`, `/dev/dri`, physical X11 `DISPLAY=:0`, LightDM authority and strict accelerated SDL/OpenGL. Each log confirms `video_driver=x11 name=opengl accelerated=1 software=0`, `nouveau`/`NVE7`, GPU `Sepia.fx` scene-before-HUD, 71 loaded sprites and zero missing/unsupported assets. All three ended at the same gameplay state: player `(2808.99,3118.11)`, `vx=480`, grounded, 7 orbs, health 25, 22 fired, two shots. Wall-clock elapsed differs and is **not** a render-phase lock.

Final 1280×720 BMP SHA-256: baseline `6ac35fc34026fc7bdae6dc24fbd1c0f179263e31dbb3a7477e4dd9f18e715d67`; hide37 `695bd0915d3865218ff19cf81e2a931ec32ebf99d6e31571c893f8a743666cca`; hide2 `1febe294b12ae468743a2945abebca33889d757448467260477567ea449d170f`. The packet also contains lossless player-neighborhood crops. `hide2` is only a player-location control; its >20-channel RGB footprint above y596 is x1023–1181, y495–595 (1,202 pixels), not an exact body mask.

Maximum absolute per-channel RGB difference, baseline versus hide37, in half-open regions:

| Region | Any change | >5 | >20 | Max | >20 bbox |
| --- | ---: | ---: | ---: | ---: | --- |
| Upper scene `[0,33,1280,596)` | 507,448 | 27,677 | **6** | 23 | x1171–1172, y573–575 |
| Lower foreground `[0,596,1280,687)` | 93,926 | 37,938 | **15,359** | 91 | x0–1267, y608–686 |
| Player neighborhood `[1023,495,1182,621)` | 16,789 | 2,029 | **89** | 37 | x1115–1172, y573–620 |
| Player neighborhood above y609 | 15,267 | 1,653 | **8** | 23 | x1125–1172, y573–608 |
| Player neighborhood y609–620 | 1,522 | 376 | **81** | 37 | x1115–1126, y609–620 |

The six/ eight small >20 upper outliers and extensive low-level whole-frame changes prevent treating every changed pixel as ID37: the film shader uses a wall-clock phase. The decisive bounded observation is **15,359 strong lower-band pixels versus only 6 strong upper-scene pixels**, plus the player's lower-edge concentration. At x1100,y620 the baseline RGB is `(43,38,27)`, hide37 `(48,41,31)`; at x600,y640 baseline `(26,24,21)`, hide37 `(30,28,24)`. ID37 darkens these tested pixels; it is not an unexplained clear-color black rectangle.

## Authored contract and reference limit

`Game/XmlFiles/levels/forest_exploration_1.xml:612–635` defines ID37 as `ParallaxLayer` with `shrubbery.png`, 2048×512, depth `kMiddleForeground=1000`, alpha 0.95, repeat X, camera-X follow/UV multiplier 0.000690, and **`fade_when_player_occluded=false`**. Original `GameObjectManager.cpp:58–65,302–330` draws in descending depth; player is `kPlayer=3000` (`GameObject.h`), so ID37 is intentionally painted over the player. Original `ParallaxLayer.cpp:37+` uses one repeat span and camera-X shift; port `LevelScene.cpp:625,1328–1405` preserves descending painter order, places the dynamic player before depth 1000, follows camera X, passes authored alpha, and maps the one-span UV shift. The original `DrawableObject` fade applies only when opted in; ID37 does not opt in. This source comparison identifies no omitted ID37 fade or reversed painter order.

The best available original screenshot, `store_media/screenshots/14.jpg`, is a matchable **Forest-1 scene** near the ID117 pickup, not this x≈2809/7-orb pose and not a lossless synchronized Windows backbuffer. The runtime availability audit (`PRIVATE_WATCHDOG_MATCHED_ORIGINAL_RUNTIME_DISCRIMINATOR_20261024T0012Z.md`) found no reachable Windows/D3D10 node or identified game EXE. Thus authored depth/fade behavior is supported and the Linux causal footprint is measured, but exact original-versus-port color/UV/film parity remains open.

## Disposition

Do **not** brighten, remove, move, or add fade to ID37 based on this result. If a source-backed correction is sought, acquire one synchronized original-Windows lossless Forest-1 player-obscured frame and draw-state packet (ID37 alpha, UV, sampler, depth, film phase and camera/player state); compare its upper player silhouette separately from the y608–686 feet/foreground edge. No source/test/asset/shared status edit, staging, commit or push was made. Only this unique report and ignored capture packet were written. Postflight found no Death Leak game/FFmpeg/capture container; **exclusive physical-Xorg :0 GPU lane released**.

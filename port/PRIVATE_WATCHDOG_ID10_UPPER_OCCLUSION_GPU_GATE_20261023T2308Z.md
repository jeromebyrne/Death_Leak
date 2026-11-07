# Forest-1 moving-pose ID10 upper-occlusion physical GPU gate (2026-10-23 23:08Z)

## Decision

**The moving x≈2809 / 7-orb upper player is not strongly occluded by XML ID10 in this Linux frame. No ID10 renderer/art/XML change is justified.** At the registered 360-frame pose, hiding only ID10 changes 184,572 upper-scene pixels by >20 RGB levels, confined to the tree at x564–979; it changes **zero** pixels by >20 in the independently located upper-player rectangle x1023–1181, y495–595. ID10's strong rendered footprint ends at least 43 pixels left of that rectangle. The player's own hide-ID2 control produces 1,230 >20-level upper-player changes, so the test is sensitive to the upper silhouette. Weak player-neighborhood differences (maximum 15 levels) also occur in a nominally unaffected control (maximum 18), consistent with unsynchronized film overlay. This is a bounded Linux render-only attribution, **not original-Windows parity or a claim of perfectly phase-matched A/B pixels**.

## Source and capture contract

`Game/XmlFiles/levels/forest_exploration_1.xml:176-190` authors ID10 as `tree2_blur.png`, `kMiddleForeground`, alpha 0.7, horizontal flip, and `fade_when_player_occluded=false`. The original depth ordering draws player ID2 before ID10 (`GameObjectManager.cpp:302-330,1021-1025`); the original shader multiplies texture alpha by object alpha (`BasicLightTexture.fx:53-59`). The port's existing `DEATHLEAK_RENDER_HIDE_XML_IDS` is a render-only filter (`port/game/LevelScene.cpp:70-85`), not a simulation or XML edit. A prior held x≈2361 / 5-orb gate showed strong ID10 player overlap; that distinct state cannot be transferred to the present moving pose.

Preflight found standing Xorg `:0` only, no Death Leak/FFmpeg/capture process or capture Docker container. The ignored evidence packet is `port/build/private-watchdog-id10-upper-occlusion-20261023t2308z/`: exact `capture.sh`, `analyze.py`, four lossless 1280×720 BMPs, logs, `analysis.json`, and a crop comparison. Serial runs used `deathleak-capture:post-foliage`, physical X socket/LightDM authority and `/dev/dri`, read-only source mount, SDL X11/OpenGL strict accelerated mode and dummy audio. They reused the corrected-clip executable SHA-256 `f346a5db9941752b29aef5bbd64c0333e31b20693a43859b97e3eae56074e9e0` (HEAD `64b2b09a2e5b34ab325df928a077a31e5893b97f`, plus previously documented dirty renderer). Each ran `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:360 --frames 360 --fixed-dt 0.0166667 --capture /out/<mode>.bmp`; modes were baseline, hide-10, hide-2, baseline-repeat. The mode switch alone differs. Logs confirm `accelerated=1 software=0`, nouveau/NVE7 OpenGL, GPU Sepia scene-before-HUD, 71 authored sprites with zero missing/unsupported assets, and identical endpoint player `(2808.99,3118.11)`, 7 orbs, health 25, grounded, 22 fired, two shots, `vx=480`.

## Registration and measured pixels

Baseline BMP SHA-256 `750609b11c0a80492ca557c00be1fcbfa47bd2f6bf1066d60624846b0e171e32` exactly equals baseline-repeat and the prior ID37 gate's moving baseline; hide-10 is `50749f9022636f2adc400bf65942d9d83dacce646ce99e392a224f220afe0de1`; hide-2 is `91a6184bc69acc2870f962f71962fc94d206f882cdd0bb0091d9ff1fc8223347`. Thus independent runs reproduce the same baseline frame; identical scripted endpoint and static scene provide camera/pose registration. Camera coordinates and zoom are **not logged**, and the corrected MP4 has no simulation ordinal, so this is not registered to a specific MP4 frame or to original Windows.

Using maximum absolute per-channel RGB difference and half-open rectangles:

| Region | hide-10 >5 / >20 / max | hide-2 >5 / >20 / max |
| --- | ---: | ---: |
| Upper player `[1023,495,1182,596)` | 1,134 / **0** / 14 | 3,486 / **1,230** / 80 |
| Full player neighborhood `[1023,495,1182,621)` | 1,498 / **0** / 15 | 5,126 / **1,699** / 80 |
| Upper scene `[0,33,1280,596)` | 243,661 / **184,572** / 100 | 27,228 / 1,230 / 80 |
| Unaffected control `[50,100,450,500)` | 7,340 / 0 / 18 | 6,540 / 0 / 19 |

The hide-10 upper-scene >20 bbox is `[564,33,980,596)`, spatially separate from the player. The hide-2 >20 bbox in the upper scene is `[1023,495,1182,596)`. The above player rectangle is a conservative registered neighborhood from the hide-ID2 control, not a precise full-body segmentation. ID37 is a separate lower-band effect and was not toggled here.

Film-frame index advances from `steady_clock` (`port/linux/main.cpp:1498-1505`). Baseline-repeat is pixel-identical, but baseline versus hide-10 has nonzero changes in the unaffected control; consequently the weak differences and raw changed-pixel total are not an exact ID10 subtraction mask. The spatially separated >20-level tree effect versus zero >20-level upper-player effect is the supported causal conclusion. It also leaves open sub-20 ID10 effects in the player neighborhood; none can be isolated from film drift with this packet.

## Next discriminating test and release

If upper-silhouette parity remains a concern, acquire a **lossless original-Windows D3D10 moving x≈2809 / 7-orb packet** with common simulation/render ordinal, camera/projection, player animation, ID2/ID10 draw order and pre/post-ID10 target, sampled tree alpha/UV, blend/sampler, and film index; pair it with Linux at the same render boundary. Compare the x1023–1181 upper silhouette separately from ID37's y609–687 foot/foreground band. Alternatively, for a Linux-only sub-20 attribution, add a diagnostic film-index lock at the same postprocess boundary in an isolated test build, then repeat the ID10 subtraction. Neither test is a reason to alter authored ID10 now.

No speculative source, XML, art, test, build, shared checkpoint/status, Git index, commit or push was changed. Only this unique report and the ignored packet were written. Postflight `ps`/`docker ps` found no Death Leak, FFmpeg or capture process/container; physical Xorg `:0`/NVE7 GPU lane is **released**.

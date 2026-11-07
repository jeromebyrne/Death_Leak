# Forest ID37 moving-player legibility — physical NVE7 discriminator (2026-10-23 23:04Z)

## Decision

**ID37 does intersect the moving player's very lowest foot/foreground band, but it does not explain loss of the upper player silhouette. Do not remove or brighten authored ID37.** This is a distinct 7-orb, right-moving Forest-1 pose (world x=2808.99, `vx=480`), not the prior stationary 5-orb x=2360.52 / frame-540 test. A same-binary render-only ID37 subtraction produces 82 large (>20 RGB levels) changed pixels in the registered player-neighborhood `[1023,495,1182,621)`, all at y609–620; none above y609. The whole lower foreground has 15,400 >20-level changes, so the ID37 causal footprint is overwhelmingly the ragged lower strip. The player-region weak differences above it are consistent with film-phase drift and do **not** support an ID37-caused broad legibility defect. This does not prove original-Windows visual parity or user acceptance.

## Clip and source relationship

Reviewed the current corrected 13-second active-gameplay packet `port/build/private-corrected-fade-active-gameplay-20261023t2231z/` and its decoded 7–9.4 s Forest stills: those frames show the player traversing the dark trunk/lower-foreground neighborhood before the validated fade. The MP4 supplies no simulation ordinal, so this run is a distinct, controlled nearby gameplay pose, not an exact video-frame match. The same executable, SHA-256 `f346a5db9941752b29aef5bbd64c0333e31b20693a43859b97e3eae56074e9e0`, produced the clip and this gate. HEAD was `64b2b09a2e5b34ab325df928a077a31e5893b97f`; that executable carries the previously documented dirty-renderer state, not a pristine-tree build.

`Game/XmlFiles/levels/forest_exploration_1.xml:612-635` authors ID37 as 2048×512 `shrubbery.png`, `kMiddleForeground`, alpha .95, repeat X/follow-camera X, `fade_when_player_occluded=false`. `port/game/LevelScene.cpp:79,1316,1396-1405` provides the existing render-only `DEATHLEAK_RENDER_HIDE_XML_IDS` subtraction and ID37 draw path. The diagnostic hides the sprite only; it is **not** a proposed XML/art change.

## Physical capture and measured pixels

Preflight found only standing Xorg `:0` (PID 1360), no Death Leak/FFmpeg/capture container. The ignored evidence packet is `port/build/private-watchdog-id37-legibility-20261023t2304z/`, containing exact scripts, BMPs and logs. Serial Docker runs used `deathleak-capture:post-foliage`, `/dev/dri`, the physical X11 socket and LightDM authority, SDL X11/OpenGL strict accelerated mode, dummy audio, read-only source mount, and the above exact binary. The distinct main pair uses `capture-moving.sh`: `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:360 --frames 360 --fixed-dt 0.0166667`; baseline has empty hide IDs, hide-37 has only `37`. A third render-only hide-2 run locates the player's upper-frame footprint. Each log proves `video_driver=x11 name=opengl accelerated=1 software=0`, `OpenGL device vendor=nouveau renderer=NVE7`, GPU Sepia scene-before-HUD, 71 original Forest sprites and zero missing/unsupported assets. Both A/B endpoints match player `(2808.99,3118.11)`, 7 orbs, 25 health, grounded, 22 fired, two shots, `vx=480`; wall elapsed differs, as expected.

Main 1280×720 lossless BMP SHA-256: baseline `750609b11c0a80492ca557c00be1fcbfa47bd2f6bf1066d60624846b0e171e32`; hide-37 `96d77bc7855251148a39c704176ac7f9d53b788c18171f151c7c5752cd59115a`. The hide-ID2 control's >20-level difference above y596 bounds the upper player/sprite activity at x1023–1181, y495–595 (1,201 pixels); this is a registration aid, not a perfect per-pixel whole-body mask. For ID37 A/B, use maximum per-channel absolute RGB change:

| Region (half-open) | Changed pixels | >5 | >20 | Maximum |
| --- | ---: | ---: | ---: | ---: |
| Upper scene `[0,33,1280,596)` | 512,443 | 27,863 | **0** | 20 |
| ID37 lower band `[0,596,1280,687)` | 93,241 | 37,875 | **15,400** | 91 |
| Registered player neighborhood `[1023,495,1182,621)` | 17,464 | 2,028 | **82** | 37 |
| Same neighborhood above y609 | 15,974 | 1,680 | **0** | 19 |
| Same neighborhood y609–620 | 1,490 | 348 | **82** | 37 |

The lower-band >20 footprint spans `[0,609,1268,687)`. The unaffected `[50,100,450,500)` crop differs between A/B, and the film frame is advanced by `steady_clock` in `port/linux/main.cpp:1498-1505`. Therefore the raw changed-pixel totals and <20-level variation are **not** an exact ID37-only mask. The strong lower-band versus upper-body spatial/magnitude contrast is the bounded causal observation, not a claim of exact subtraction. Earlier held 5-orb frame-480 control in the same packet (`capture.sh`, `B:300,F:180`, frame 480) reproduced a 15,858-pixel >20 lower-band effect, zero >20 in its prior registered `[810,509,893,621)` player box. Three baseline and three hide-37 repeats at that held pose each reproduced identical within-mode BMP hashes; cross-mode film crops stayed unequal, so no false phase-match is claimed.

## Source-backed next action and release

The next fidelity action is a **registered, lossless original-Windows Forest-1 frame at the moving x≈2809 / 7-orb pose**, with draw-state for ID37 and the upper overlapping foreground ID10 (camera/UV, alpha, blend, sampler/effect, painter order and film index), paired with a Linux frame at the same postprocess/film boundary. Compare the upper player silhouette separately from the y609–687 shrubbery/feet edge. Only a demonstrated original-vs-port delta would justify a narrow renderer fix. The Linux subtraction alone does not justify modifying authored ID37.

No speculative art/XML/source/build/checkpoint edit, Git staging or commit. Only this unique report and ignored capture packet were written. Postflight `ps`/`docker ps` showed no Death Leak game, FFmpeg or capture container; the exclusive physical X11/NVE7 GPU lane is **released**.

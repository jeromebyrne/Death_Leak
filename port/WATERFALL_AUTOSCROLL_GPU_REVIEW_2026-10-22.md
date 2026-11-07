# Waterfall autoscroll physical-GPU review — 2026-10-22

**Result:** The newly committed waterfall V autoscroll is visibly active and moves texture features **downward** on physical nouveau/NVE7. Representative 7/9/11-speed layers move at the expected, geometry-adjusted rates in fixed-camera active play. No obvious tile gap/tear or changed quad edge is visible in the sampled frames. This is a bounded Linux physical-GPU review, **not** a Windows parity or human acceptance claim. Discrete stills cannot rule out intervening flicker.

## Provenance and capture

- Checkout HEAD: `68a038575b649ded1095df135b18ad4ecc4c6f08` (`Restore waterfall V autoscroll and flipped UV wrap`); executable SHA-256 `294ccea1ba8dab0df62066405ec4ee4ad04f2843421ab8257f259a5952465979` (binary mtime 2026-10-22 15:52:59 PDT). Hash/HEAD copied into the ignored artifact directory.
- Exclusive physical Xorg `:0` lane. Each run used the existing `deathleak-linux-build:bookworm` image, `/dev/dri`, LightDM Xauthority, `SDL_VIDEODRIVER=x11`, `SDL_RENDER_DRIVER=opengl`, dummy audio, and **rendered** `--play Game/XmlFiles/levels/waterfall_top.xml --script '.:120' --fixed-dt 0.016666667 --frames N --capture ...` (not `--simulate-only`/camera-script). Separate identical-prefix processes captured the last drawn frame at N=60, 70, or 120. All relevant logs report `video_driver=x11 name=opengl`, `accelerated=1 software=0`, `mode=strict-accelerated`, `OpenGL device vendor=nouveau renderer=NVE7`.
- `DEATHLEAK_RENDER_ONLY_XML_IDS=6,7,10,13,14,15,16,31` captured the eight-sheet isolate at frames 60/120; the variable was empty for corresponding full composition. IDs 6, 10, 7 were additionally captured individually at frames 60/70/120. All captures are readable 1280×720 BMPs under `port/build/waterfall-autoscroll-gpu-review-2026-10-22/`, with same-named logs. Full-size PNGs and the native-size ID 6 crop are lossless derivatives of the BMPs; the labeled contact sheets are scaled review aids.
- Player state is identical at frames 60 and 120: `(818.985, 2933.64)`, grounded, no input/velocity. The visible composed bridge, orb and terrain positions stay fixed; the isolated curtain outer edges stay fixed. This controls camera/quad motion while the texture flows.

## Visual and rate findings

The eight-sheet isolate visibly changes internal streaks between frames 60 and 120, while its extent and opacity falloff remain stable. The composed pair retains the broad pale shaft behind bridge/orbs; the flow is subtler there due to overlapping authored sheets and scene elements. Individual captures make the moving pattern clear. Comparing the same 10-frame interval (60→70), a sampled five-to-twenty-column, multirow RGB squared-difference search finds the best *positive* later-frame screen-Y displacement (features move down):

| ID | Authored V speed | Quad height | Measured shift | Expected shift at 720p |
|---|---:|---:|---:|---:|
| 6 | 7 | 3303.137 | ~102 px | 102.76 px |
| 10 | 9 | 2228.792 | ~88–90 px | 89.15 px |
| 7 | 11 | 2240.274 | ~108–111 px | 109.52 px |

Expected pixels = `speed × .001 × 10 frames × (720/1080 V-time scale) × quad height × (720/1080 world-to-screen scale)`. Raw pixel shifts are **not** ordered 7<9<11 because authored quad heights differ; normalized phase rates are. The representative physical measurements agree within a few pixels despite low-contrast/alpha texture. The 60→120 difference spans the water texture, not only HUD or camera displacement.

At native-size ID 6 crops (x=100–500, y=200–600), no exposed black slit or abrupt split-tile tear is apparent. A broad darker horizontal feature in frame 120 is visible, but does not register as a sharp scanline discontinuity in row-difference checks; it is not sufficient evidence of a renderer seam. The 60/70/120 stills do **not** prove flicker-free continuous motion, all eight IDs' independent exact speed, or Windows parity. A continuous physical-GPU video/human review remains the visual acceptance gate.

## Opportunistic damage-burst physical capture

Used the route in `DAMAGE_BURST_GPU_CAPTURE_ROUTE_2026-10-22.md` without source edits. Eight 1280×720 BMPs/logs are in ignored `port/build/damage-burst-gpu-review-2026-10-22/` (plus contact sheets and fixed, unadjusted actor crops). Every run passed the same strict X11/OpenGL nouveau/NVE7 provenance. Player contact occurs at zero-based frame 24: frame counts 24→25 change health 25→23 with `solid_damage=2` trace, remaining 23 at 35/45. Ninja projectile contact occurs at zero-based frame 27: counts 27→28 change `hits=0→1`, with enemy alive and `combo=1`, persisting at 38/48.

The **ninja** fixed crop (`enemy-crop-contact.png`) visibly brightens at age 0 (count 28), attenuates at .125 s (38), and returns near pre-hit appearance at .25 s (48), despite animation/kickback. Adjacent black scene and other player silhouette do not flash; no obvious halo or disappearing sprite. The **player** fixed crop (`player-crop-contact.png`) remains a nearly black silhouette (peak crop channel 9 in all four frames); the expected RGB-gain sequence is **not visually resolvable** there. Therefore the bounded player visual criterion is inconclusive, not accepted. Full-frame RGB cannot prove source-alpha preservation. These sparse fixtures do not establish authored-forest composition, Windows parity, or user acceptance.

No software/source edits were made, and no commit was created. Existing unrelated working-tree files were preserved; `PORT_WORK_STATUS.md` was not touched.

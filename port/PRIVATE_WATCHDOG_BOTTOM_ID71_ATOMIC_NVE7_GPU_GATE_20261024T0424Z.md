# Waterfall-bottom ID71 mirrored lower-right platform — atomic physical-NVE7 GPU gate

## Verdict

**ID71 materially contributes pixels to active Linux gameplay; no original/port parity verdict or source-proven correction.** A same-render ordinal-826 pre/post draw readback changes 8,992 RGB pixels. A same-binary exact-level hide-ID71 arm retains 8,688 changed post-Sepia/pre-HUD pixels. The pre-draw and hidden post-draw readbacks are byte-identical, and left/upper controls are unchanged. No provenance-qualified original D3D10 active-bottom packet exists; this is a Linux reachability/composition result, not original parity.

## Identity, intervention and reproduction

The target is `Game/XmlFiles/levels/waterfall_bottom.xml:1291-1305` Sprite ID71, world center `(626.985352,2090.258789)`, `kGround`, dimensions `230.036148×193.769196`, non-repeating `Game/Media/objects/floating_platform_1.png`, alpha 0.6, authored horizontal reflection, no vertical reflection or shader wobble. The art SHA-256 is `755afb1f361e47c5b6f79b497eef860d3c89d3c61d250455a896ca4776ad5b6a`. Prior source audit: `port/PRIVATE_WATCHDOG_BOTTOM_ID71_LOWER_RIGHT_PLATFORM_READONLY_20261024T0412Z.md`.

An isolated overlay of the already successful ID33 atomic packet diagnostic selects only renderer scene ordinal 8 with this exact texture path, `flipX=true`, and no pixel wobble. `DEATHLEAK_PRIVATE_BOTTOM_ID71_PREFIX` requests pre/post `SaveFrame`; `DEATHLEAK_PRIVATE_BOTTOM_ID71_HIDE=1` suppresses only that draw in the hidden arm. The overlay is in ignored `port/build/private-bottom-id71-atomic-gpu-20261024t0430z/main.cpp` (SHA-256 `77d294ecc05b0e90924b08a0e02c934c396dc80f63e70f33d6c05d5a0ec1b327`); isolated binary SHA-256 `0babc03967bf8abef704eb9bf9cf9443431bc047884125dc0d7023cd55e035f0`. Docker C++ build succeeded; only pre-existing TinyXML warnings. No shared source or renderer/test/art file was edited.

Visible and then hidden runs used the **same binary**, `deathleak-capture:post-foliage`, physical Xorg `:0`, `/dev/dri`, Xauthority, `SDL_VIDEODRIVER=x11`, `SDL_RENDER_DRIVER=opengl`, `DRI_PRIME=pci-0000_01_00_0`, `MESA_LOADER_DRIVER_OVERRIDE=nouveau`, dummy audio, `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:1800 --frames 827 --fixed-dt 0.016666667 --capture .../final.bmp --atomic-postfade-waterfall-packet .../packet.json`. Logs confirm strict accelerated OpenGL (`software=0`), nouveau NVE7, GPU Sepia, top/bottom transitions, and exit 0. Capture/log/packet files for each arm and build log are in the unique ignored directory above.

Both packet identities report active `waterfall_bottom.xml`, render/update ordinal 826, player `(833.383118,1370.51953)`, pinned stored/draw camera `(438.31134,1445.68396)`, zoom 1. This camera pin is a disclosed Linux diagnostic intervention, not a naturally matched original frame; `matchedStateEligible=false`. Both ordered draw 8 commands agree: the platform art, whole texture source, destination `[704.882568,404.548248,164.466949,138.537476]`, white tint alpha 153/opacity 0.6, rotation 0, `flipX=true`, `flipY=true` (the port's D3D-to-SDL V conversion), no repeat/UV shift/sway/wobble. Original XML/Sprite/BasicLightTexture source and current LevelScene/ordinary SDL draw agree at this command level; no first differing field is established.

## Lossless RGB readback

Pillow RGB compare on 1280×720 BGRA32 BMPs. Changed pixel means any unequal RGB channel. Bboxes are readback image coordinates. `x<600` left and `y<350` upper controls have zero changed pixels in every comparison. The final readback is corroborative only because wall-clock HUD film phase is not pinned; pre-HUD is the clean surviving-scene estimator.

| Boundary | Changed RGB pixels | Difference bbox | Mean absolute RGB |
|---|---:|---|---:|
| Visible immediate pre→post ID71 | 8,992 | `[707,411,866,540)` | 0.54511 |
| Visible→hidden immediate post-ID71 | 8,992 | `[707,411,866,540)` | 0.54511 |
| Visible→hidden post-Sepia/pre-HUD | 8,688 | `[708,411,866,540)` | 0.06100 |
| Visible→hidden final post-overlay | 8,637 | `[708,411,866,540)` | 0.04883 |

Both pre-ID71 and hidden post-ID71 SHA-256: `9bfedcce720e34e44945a20f8d10d164464d9e5084830fde0c4f0783662b0a73`. Visible post-ID71: `bb31bc70e9432ce62f7e4224ff2d5fd01b7f49e68b8876892c80602613ee84d6`. Visible/hidden pre-HUD: `7ac8644e4e9d788967e72871fdd2dbe06c4d9004e7bcaf9073818dadea361ae0` / `30a25ed5106fe512e79654fbb3657cae0340c25abe98ba58d70ab3ba604cd3c6`. Visible/hidden final: `691df19b199ce54127532a5df643dbb6678d68cb50a1adee76b1cf4dc0fd14ab` / `6c709c7ca208120a23f32fb5593b4621130007d4d1d709ea29b3278503d20701`.

## Next discriminator and release

Acquire a provenance-qualified original D3D10 **active Waterfall-bottom ID71** packet at registered camera/player/viewport: bound art/hash, actual matrix/UV corner order, sampler/mip/blend, object alpha, painter slot and lossless target crops immediately before/after ID71, plus unaffected control. Compare first command fields, then isolated residual and final composition; Waterfall-top Steam screenshot 16 is not this reference. Without that packet, no production code/art fix, commit/push, or parity claim is justified.

Existing dirty renderer/Sea-2 work and `PORT_WORK_STATUS.md` were preserved. Both capture containers exited and no Death Leak capture/build container remained at release check. **Exclusive physical Xorg `:0`/NVE7 GPU and build lane released.**

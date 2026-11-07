# Raft-only renderer mip experiment — 2026-10-23

## Finding and scope

The locally available original Windows **source and art do not prove the mip levels actually bound at a raft draw**. `Game/src/Core/TextureManager.cpp:66` passes null D3DX10 load info, and `Game/ShaderFiles/BasicLightTexture.fx:10-16` requests `MIN_LINEAR_MAG_POINT_MIP_LINEAR`; these predict a mip-capable path under the documented defaults, but there is no local bound SRV/resource descriptor, executable/PDB, or matched lossless Windows frame. The 1024×280 raft PNG and IDs 11/57/58 are detailed in the prior raft reports. This is a source-contract candidate, not a demonstrated Windows pixel defect.

Only `port/linux/SDLRenderer.cpp` was changed. With `DEATHLEAK_RAFT_MIP_EXPERIMENT=on`, an ordinary scene draw using exactly `Media/objects/raft.png` (also allowing a path prefix) uses an existing private GL PNG mip uploader and trilinear-minified, nearest-magnified sampling. The default/unset setting remains the original SDL copy. The path is disabled for HUD, source rectangles, repeats, UV shifts and special effects. It occupies the original painter slot, preserves quad destination/rotation/flips and tint/alpha, and remains before scene sepia. It clamps the raft's mip edges. Failed private upload falls back to the SDL copy. This is an **A/B experiment**, not a production fidelity change; GL mip generation is not proven identical to D3DX10 generation.

## Verification and limitations

- `git diff --check -- port/linux/SDLRenderer.cpp`: passed.
- Focused renderer syntax compilation: passed in the existing `deathleak-linux-build:bookworm` image, with source mounted read-only and no network: `g++ -std=c++17 -Wall -Wextra -Wpedantic -fsyntax-only $(pkg-config --cflags sdl2 libpng gl) -Iport/compat port/linux/SDLRenderer.cpp`.
- Native host compilation was unavailable because SDL2/libpng/GL development pkg-config files and headers are absent. No GPU, Xorg, game run, capture or off/on pixel comparison was performed. No original Windows descriptor or matched crop was obtained.

The exact next gate is a Windows bound raft SRV **and** resource mip count/view range at an ID11/57/58 draw. If more than one level is sampleable, an exclusive GPU lane may capture frozen-state, lossless Linux off/on raft-local crops and a registered same-state Windows crop. The off/on difference alone only measures local sensitivity. Check alpha edges and collateral sprites before considering a production patch; remove this switch if it does not improve the matched comparison.

This slice did not edit shared statuses, LevelScene, main, HUD, XML or art. The renderer experiment is left uncommitted because it has only syntax-level verification and no runtime/Windows acceptance evidence. Renderer-file ownership is released.

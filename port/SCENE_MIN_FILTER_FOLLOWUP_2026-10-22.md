# Ordinary scene minification follow-up — 2026-10-22

## Scope and source evidence

Code-only correction in `port/linux/SDLRenderer.cpp`. `port/WATERFALL_LIGHT_LAYER_AUDIT_2026-10-22.md` identifies ordinary `sun_shaft_2.png` ID46/47 copies as 512×512 sources rendered at about 341⅓×341⅓ logical pixels, with authored alpha, flip and rotation. `Game/ShaderFiles/BasicLightTexture.fx` specifies `MIN_LINEAR_MAG_POINT_MIP_LINEAR`, while the SDL texture-copy path had no explicit scale mode. The correction selects `SDL_ScaleModeLinear` for minified pre-sepia copies and `SDL_ScaleModeNearest` for non-minified or post-sepia (HUD) copies. Source-rectangle dimensions are considered; repeat-X scene tiles are minified at the renderer's fixed 2/3 tile scale. Filtering is set per draw so reuse of a texture by later HUD copies does not inherit linear mode. Blend, alpha, UV, flip, rotation, layer order and XML are unchanged.

SDL's single scale-mode API cannot express separate minification and magnification filters for a mixed-axis scale, and it does not reproduce D3D mip-level selection. The chosen mode follows the minified case whenever either axis minifies. Pre-sepia menu/preview sprites also receive this rule because the renderer API has no explicit scene-begin marker; the change is not claimed to isolate those phases. Dedicated GL effects still set their own min/mag filters and are not otherwise changed.

## Verification and handoff

`git diff --check -- port/linux/SDLRenderer.cpp` passed. No build, game run, Xorg/GPU capture or visual acceptance test was performed. At handoff, `port/CURRENT_WATERFALL_GAMEPLAY_VIDEO_2026-10-22.md` did not exist, and the other worker exclusively owned the physical GPU/capture and Docker-build lane. Before any build or GPU validation, confirm that report exists and capture has ended. Then compile and compare a lossless matched-camera ID46/47 shaft edge/composite against the Windows reference before accepting visual fidelity. This code-only change is intentionally uncommitted and unpushed pending that verification.

# Pale right shaft follow-up — 2026-10-22

## Decision

No additional source-backed renderer mismatch is demonstrated at pushed HEAD `50ba9e9`; no software, XML, or art correction was made. The latest physical-GPU active forest clip (`port/build/latest-gpu-gameplay-review-2026-10-22/forest-1-head-50ba9e9-active-10s.mp4`, especially its 5/9-second stills) shows the conspicuous flat-topped field, but the available Steam forest screenshot 14 is an unmatched camera/state/crop reference. Apparent edge softness in that screenshot cannot establish an ID15 filtering or alpha defect.

## Evidence beyond the earlier attribution

- The controlled physical-GPU hide/only triplet in `port/PALE_PATCH_GPU_AUDIT.md` identified the field as forest-1 XML ID15 `Media/sun_shaft_2.png`. That triplet was captured at `0f5484a`, **before** the `8a6dc85` radians-to-degrees sprite-rotation correction. It establishes identity, not a pixel-locked measurement of the current rotated outline.
- At current HEAD, XML ID15 still specifies a native 512×512 ordinary `Sprite` at `(2796.985,3207)`, rotation `-1.469999` radians, `kGroundFront`, alpha 1, no flips, and `effectlighttexture` (`Game/XmlFiles/levels/forest_exploration_1.xml:237-251`). `LevelScene::Draw` converts its radians to SDL degrees and gives it the ordinary centered sprite path; `SDLRenderer::DrawSprite` uses `SDL_RenderCopyExF` with source alpha. The recent weather/combat/aim changes did not replace its authored shader path or texture. The old transform-order speculation in `RAFT_EXIT_PARALLAX_SOURCE_AUDIT_2026-10-22.md` was corrected in `RIGHT_SHAFT_PARITY_2026-10-22.md`: source multiplication is `R*S*T` for the row-vector shader, leaving the local quad center at its translation.
- Original `BasicLightTexture.fx` samples the RGBA asset directly (`MIN_LINEAR_MAG_POINT_MIP_LINEAR`, wrapped UV), multiplies by object alpha, and uses source-alpha-over composition before full-scene sepia. There is no source shaft-edge blur. The asset's nearly rectangular yellow-alpha area with short feather, documented by direct pixel samples in `PALE_PATCH_GPU_AUDIT.md`, can produce a straight-looking boundary. A possible subpixel D3D-versus-SDL sampler difference has not been measured and cannot explain or justify removing the broad authored field.

## Next discriminating capture

Obtain a lossless **original Windows forest-1** frame at 1920×1080 source viewport (or record its exact viewport transform), with camera XY/zoom, ID15 world matrix, draw order, postprocess time and game state recorded. Capture the isolated post-`BasicLightTexture.fx` ID15 layer and final composite. Recreate that camera/state on current Linux HEAD, capture its isolated ID15 and composite at the same output resolution, then compare projected quad corners, UV orientation, alpha-edge profile and RGB after sepia. The earlier Linux hide/only frame should not be used as the final comparator because it predates the rotation fix. Only a measured divergence should gate a narrow sampler, transform, or blend correction. The current active clip remains unaccepted visual evidence, not Windows pixel parity.

This diagnostic did not use the GPU/build lane, run a build or test, alter shared `PORT_WORK_STATUS.md`, or touch unrelated dirty files. No commit/push is warranted without a software correction.

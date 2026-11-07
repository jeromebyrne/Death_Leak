# Post-fade next visual discriminator — 2026-10-23 23:02Z

## Verdict

**No distinct, unclaimed high-impact Windows-source-backed visual correction is established.** The corrected 13-second clip shows a sound Forest→Waterfall presentation at the available qualitative level; it is not a matched original-Windows comparison. Do not nominate a speculative source patch from it.

## Evidence and ownership

- Inspected `port/build/private-corrected-fade-active-gameplay-20261023t2231z/corrected-active-13s.mp4` via its saved 1/5/7/8/9/10.1/10.5/11.5/12/12.8-second stills and contact sheets. Saved `ffprobe.txt`: 1280×720 H.264, 30 fps, 390 frames, 13.000 s. `game.log` confirms strict accelerated X11/OpenGL on nouveau NVE7, 71 Forest sprites with none missing, Forest→Waterfall-top trigger, then Waterfall-bottom trigger. Frames around 9.7–9.9 s are opaque; 10.1 s reveals Waterfall gameplay. The separate physical-GPU exact-frame gate `PRIVATE_CURRENT_FADE_GPU_PRESENTATION_GATE_20261023T2258Z.md` already owns and passes the fade boundary.
- The conspicuous late dark player/tree overlap is not a new proven mismatch. Forest XML `Game/XmlFiles/levels/forest_exploration_1.xml:176-190` authors ID10 as 0.7-alpha middle foreground with player-occlusion fading disabled; ID37 is separately authored at `:612-635`. Original draw ordering is in `Game/src/GameObjectManager.cpp:302-330,1021-1025`; port interleaving/submission is `port/game/LevelScene.cpp:1329-1357,1403-1429`. `PRIVATE_POST_FADE_ID10_SOURCE_GATE_20261023T2239Z.md` and `PRIVATE_POST_FADE_ID37_OCCLUSION_GATE_20261023T2234Z.md` own those questions.
- The blocky orb count is source-proven but already claimed: original Impact/D3DX10 font at `Game/src/UI/UIGameHudScreen.cpp:238-260` versus 5×7 bitmap at `port/game/Hud.cpp:85-92`; `PRIVATE_HUD_IMPACT_NUMERAL_FIDELITY_20261023T1942Z.md` records the authentic-face blocker. The left Waterfall ID46 shaft is a conditional mip-sampling candidate, not a measured mismatch, and `PRIVATE_POST_FADE_NEXT_DISTINCT_VISUAL_GATE_20261023T2241Z.md` plus the ID46 owner already cover `Game/XmlFiles/levels/waterfall_top.xml:796-810`, `Game/ShaderFiles/BasicLightTexture.fx:10-16,53-59`, and `port/linux/SDLRenderer.cpp:220-245,270-280`. The visible FPS diagnostic is intentional/requested, not a parity nomination.

## Bounded next test (existing reference/ID46 owner only)

Acquire one atomic, lossless original Windows D3D10 render-ordinal packet at Waterfall-top ID46 contribution, including executable/assets identity, camera/player state, bound SRV mip count/range, sampler/effective LOD, ordered draw command, and pre/post-ID46 pixels. Pair with same-state Linux capture and a registered shaft-feather ROI/control. Only a localized first divergent field/pixel residual outside the already owned gates warrants a bounded patch. If original SRV is single-level or pixels match, close ID46. No new Linux-only capture or unmatched still can establish this.

## Non-overlap

Read-only clip/source/report inspection; only this unique report was written. No GPU, Docker/build, production source, tests, shared status/checkpoint, prior reports, index, commit, or message. Exclusive GPU worker `8f2b5cd2-948d-4c6d-8d12-524e4c8bb481` owns presentation captures; this lane makes no claim on them. Pre-existing dirty files preserved.

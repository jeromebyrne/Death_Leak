# Waterfall-bottom present-946 immediately post-player water sheets: bounded source gate — 2026-10-24

## Verdict

**No first source-proven Windows/Linux parity divergence in this bounded slice; no production correction licensed.** The three separate immediately post-player scene submissions at active bottom present 946 are original XML `ParallaxLayer` IDs **13, 14, 15**, respectively Linux renderer ordinals **20, 21, 22**. Each materially changes the projected-player ROI in the saved same-render Linux readbacks. Their authored foreground depth, asset, opacity, reflection, effect, and Y-scroll intent agree with the Linux command and source path. The available original Windows XML/source specifies behavior, but there is **no matched original D3D10 present-946 command or scene-target pair**, so pixel parity, exact phase parity, and the first discrepant field cannot be asserted. In particular, the later ROI flattening at ordinal 25 and 28 is a separate attribution, not proof these three sheets are erroneous.

## Identity, ordering, and authored contract

`Game/XmlFiles/levels/waterfall_bottom.xml` (SHA-256 `da4d2f97f81e9e838abd0d62cdf167d1af9db13edce1d33a34dfd11393788f04`) byte-matches the bundled Windows-content copy. Its IDs 13/14/15 begin at lines 216/240/264. All are `ParallaxLayer`, `depth_layer=kGroundFront` (original enum 1750), `updateable=true`, `always_update=true`, width 1120.136963, height 2240.273926, `native=false`, `repeatX=false`, `repeatY=false`, material `grassground`, `alpha=.2`, `effectlighttexture` with all wobble/noise intensities zero, `fade_when_player_occluded=false`, `Media\waterfall_4.png`, horizontal flip true, vertical flip false, zero camera-X/Y parallax, no follow-camera axes, no X scroll, and `autoscrolly=true`. The asset SHA-256 is `33c06ae5c326865d39ee1f7f928991dfbdbece8c5bc249611a2365842d42ca3e`, identical to the bundled Windows-content copy.

| XML ID → present-946 renderer ordinal | XML position (x,y), authored Y-scroll speed | Saved Linux destination `[x,y,w,h]` | Saved Linux V phase |
|---|---|---|---:|
| 13 → 20 | (398.451904, 1475), 11 | `[141.031738,-880.439941,746.757996,1493.51599]` | .349333972 |
| 14 → 21 | (321.451904, 1475), 9 | `[89.6983643,-880.439941,746.757996,1493.51599]` | .103999011 |
| 15 → 22 | (382.451904, 1474), 4 | `[130.365051,-879.773193,746.757996,1493.51599]` | .490665376 |

All three Linux packet commands bind `Game/Media/waterfall_4.png` (texture ID 396), source `[0,0,0,0]` meaning full texture, tint `[255,255,255,51]`, opacity `.200000003`, no rotation, `flipX=true`, `flipY=true`, `repeatX=false`, `uvShiftX=0`, no foliage sway and no pixel wobble. Linux `flipY=true` is the source-port D3D-bottom-origin to SDL-top-origin representation of authored `verticalflip=false`, **not** an observed Windows flip mismatch. The destinations overlap the player-sized rectangle `[744,334,864,478)`; ID14 ends near x=836.46, so its smaller changed-pixel count is expected spatially. Ordinal 19 is the submitted player body; these are the *next three* scene commands, before ordinal 23's additional waterfall repeat and ordinal 25's cave blur. This distinguishes this gate from the earlier bottom ID21/ID27 audit.

## Original source versus saved Linux packet

Original `Game/src/GameObjects/ParallaxLayer.cpp:26–160` updates the sprite then, in `Draw`, advances its stored V by `autoscrollyspeed * .001 * GetLastUpdateDelta()/GetTargetDelta()` and writes one V span to the four vertices; with no X parallax or X scroll, it does not invoke the X-repeat block. `ParallaxLayer::Scale` (lines 242–253) scales positive Y speed by viewport Y scale. Original `DrawableObject.cpp:125–153` selects `EFFECT_LIGHT_TEXTURE` from `effectlighttexture`; `Sprite.cpp:626–674` submits authored alpha; `Game/ShaderFiles/BasicLightTexture.fx:11–16,53–59` uses wrapped U/V sampling and multiplies texture alpha by object alpha. Original D3D blend is source-alpha/one-minus-source-alpha (`Game/src/Core/Graphics.cpp:397–401`). Original source therefore supports three independent, translucent, scrolling foreground sheets—not a player-occlusion fade or wobble shader.

Current `port/game/LevelScene.cpp:1375–1454` maps authored parallax position/size/alpha/texture, converts the V-origin flip, and advances each sheet's own fractional phase at draw by `speed*(720/1080)*.001*(previousRenderedModifiedDelta*60)` before submission. The saved packet's 120-present progression from active present 826 is self-consistent: ID13 `.469333529 + 120*(11*.001*2/3) mod 1 ≈ .3493335`; ID14 `.384000212 + 120*(9*.001*2/3) mod 1 ≈ .1040002`; ID15 `.170666680 + 120*(4*.001*2/3) mod 1 ≈ .4906667`. Observed present-946 phases differ only at float-rounding scale. This proves a consistent **Linux** phase trajectory, not an original-Windows phase match: no original count of eligible draws, preceding update delta, or matched D3D UV corner packet is available. The established current-update/preceding-delta first-frame cadence distinction in the prior autoscroll work is not a demonstrated divergence at steady present 946.

## Same-render player-ROI contribution

Read-only Pillow analysis of retained lossless BMPs in `port/build/private-bottom-player-946-20261024/` used the half-open `[744,334,864,478)` ROI (17,280 pixels). A changed pixel has any changed RGB channel relative to the immediately previous saved stage; mean absolute RGB change is over all 51,840 ROI channels. These are actual Linux scene-target effects, **before** later ordinals, sepia, and HUD.

| Stage | ROI gray mean / SD | RGB pixels changed by this draw | Mean absolute RGB change |
|---|---:|---:|---:|
| after player ordinal 19 | 14.97 / 33.75 | — | — |
| after ID13 / ordinal 20 | 32.46 / 33.17 | 14,637 | 17.601 |
| after ID14 / ordinal 21 | 36.20 / 35.42 | 7,655 | 3.796 |
| after ID15 / ordinal 22 | 47.81 / 39.44 | 13,228 | 11.729 |

The respective full-BMP SHA-256s are post-player `fdf1102743f16b5642ce9f67fc1d2abd80388f9b6a5d32d60de619d5aec375d9`, after-20 `cc03a1e57eefcdf60d51ef66ca35f7aa6b272b4f824a276e3fe5bb6a2f4469d5`, after-21 `fe98acb383c098c8cd427fc77c6c67aae150f4808a5a30d5c30a235cbfb1b8cb`, after-22 `caff4bd16711789b156b69260c46d4b7f3992e2cfc75b51da359e5eb5cb8370d`. The packet is `port/build/private-bottom-player-946-20261024/packet.json`, SHA-256 `23c91c1adcf8c4f2c8af45d41e12bb1725986724a32306f03903964fbd5c8f67`; its `matchedStateEligible=false` must be respected. Its original physical NVE7 capture route, camera `(586.836,734.477)`, player `(833.383,664.931)`, and provenance are documented in `PRIVATE_WATCHDOG_BOTTOM_PLAYER_PRESENT946_ADMISSION_AND_LAYER_PACKET_20261024.md`. This audit starts from those existing artifacts; it did not execute a renderer.

## Next parity discriminator and lane release

Acquire a matched original D3D10 active-bottom command packet at equivalent camera/player, draw/update ordinal, viewport, and prior UV state, with pre/post ID13, ID14, ID15 scene-target readbacks. Compare the **first** mismatching command field (draw order, world transform, V corners/phase and wrap/filter, alpha/blend/effect), then registered per-draw ROI residuals; only that would justify a narrow fix. No available Steam still or unmatched Windows screenshot is a substitute. This gate found no source-proven divergent field and proposes no production edit.

Only this new report was written. No GPU/Xorg/Docker/build, source/XML/art/test edit, commit, push, or shared `CHECKPOINT_STATUS.md` append. Dirty files, Sea-2, and `PORT_WORK_STATUS.md` were left untouched. **Read-only source/packet lane released; no exclusive physical capture/build lane was occupied.**

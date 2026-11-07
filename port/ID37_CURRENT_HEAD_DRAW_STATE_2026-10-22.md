# ID37 current-HEAD CPU draw-state discriminator — 2026-10-22

## Result

At inspected HEAD `e2c5adf1de43bf694185367b4702acb32c3e32bc`, the source and authored forest-1 XML still encode the ID37 follow-X timing difference described in `FOREST_BRIDGE_NEXT_DISCRIMINATOR_2026-10-22.md`. A bounded CPU source-and-arithmetic check confirms the predicted draw-state values for the earlier GPU experiment's camera inputs. It is **not** a fresh invocation of `LevelScene::Draw`, a current-HEAD binary test, or a new GPU/pixel measurement. The held-camera negative control gives zero destination difference. No change to renderer, art, or alpha is indicated by this check.

| Endpoint | Previous camera X | Current camera X | Linux current-X destination left | Prior-X destination left | Prior minus current | Quad width | Current-camera U, both |
|---|---:|---:|---:|---:|---:|---:|---:|
| Moving | 761.040 | 769.036 | −42.666667 | −47.997333 | −5.330667 | 1365.333333 | 0.530635 |
| Held | 769.036 | 769.036 | −42.666667 | −42.666667 | 0 | 1365.333333 | 0.530635 |

These numbers use the ID37-only draw geometry at 1280×720: `scale=2/3`, width 2048, `uvShiftX=Cnow×0.000690`. For the current Linux branch `worldX=Cnow`; for the proposed diagnostic prior-X destination only, `worldX=Cprev`. Thus `destination.left=640+((worldX−Cnow)−1024)×2/3`, and prior-minus-current is `(Cprev−Cnow)×2/3`. Its center has the same displacement. The U phase must remain keyed to `Cnow` in both variants.

## Evidence and method

- `Game/XmlFiles/levels/forest_exploration_1.xml:612-635` has ID37 `followcamx=true`, `followcamy=false`, `sine_wave_props active=false`, width 2048, camera-X multiplier 0.000690, alpha 0.95. The check parsed and asserted those fields using Python's XML parser.
- `port/game/LevelScene.cpp:1076-1078,1125-1127,1150-1159` has `scale=2/3`; followed ParallaxLayer X takes the `Draw` call's current `cameraX`; destination uses `(worldX-cameraX)`; and U uses current `cameraX × cameraParallaxX`. The check asserted the corresponding current-HEAD source expressions and calculated the table with Python float arithmetic. It did not edit source or build a binary.
- `Game/src/GameObjects/ParallaxLayer.cpp:26-35` calls `Sprite::Update` before assigning followed `m_position.X=camX`; `GameObject.cpp:202-210` builds `m_world` from the position during that update; `Sprite.cpp:626-637` submits cached `m_world` to the light-texture effect. The source-order assertion passed. This supports the prior-camera destination interpretation for a normally updated Windows object; it is not an executable Windows trace.
- Prior controlled physical-GPU evidence at earlier HEAD `814fbd3` in `FOREST_FOLLOW_LAG_GPU_DISCRIMINATOR_2026-10-22.md` measured the same −5.331-pixel moving offset with identical U, 49,532 changed pixels in the isolated lower crop, low mean RGB delta, and byte-identical held images. Those pixel findings are historical, **not** measured at this HEAD.

## Exact bounded executable seam, if a future regression gate is needed

An existing CPU draw-spy pattern is `port/tests/parallax_speed_test.cpp`: its `CaptureRenderer` implements `render::Renderer::DrawSprite` by collecting `render::Sprite` values, loads forest-1 XML through `LevelScene::Load`, filters to selected XML IDs with `DEATHLEAK_RENDER_ONLY_XML_IDS`, and calls `LevelScene::Draw`. A narrowly scoped future test can filter only ID37, capture one sprite per call, use the same fixed camera Y and player state, then call `Draw` at `Cprev=761.040`, `Cnow=769.036`, and held `Cnow=769.036` again. Assert Linux `destination.x=−42.666667`, `destination.width=1365.333333`, and `uvShiftX≈0.530635` at the moving and held endpoints. To test the *counterfactual* prior-X destination directly requires a temporary render-only override at `LevelScene.cpp:1125-1127` for ID37's `worldX`, preserving current-camera `uvShiftX` at lines 1157-1159; assert `destination.x=−47.997333` moving and `−42.666667` held. No such override was installed here. Reusing the draw spy without that seam validates only current Linux behavior; it cannot independently produce the Windows prior-X state.

## Limitations and handoff

This CPU check uses the prior experiment's rounded camera coordinates, not newly traced current-HEAD gameplay history. Textual source assertions plus arithmetic are weaker than an instrumented current-HEAD draw-spy execution, and the older GPU crop cannot quantify appearance after HEAD changes. It isolates ID37's transient horizontal registration, not the broad lower-bridge darkness/softness gap or Steam JPEG parity. A matched original-Windows lossless frame and same-frame camera/state history remain necessary for a material parity judgment. This report is the sole write; no source, test, Docker/build, GPU/display, capture, shared checkpoint, or other report was changed.

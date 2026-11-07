# ID37 parity follow-up — 2026-10-22

## Verdict

No further ID37 source-contract defect is supported by the available evidence. The remaining near-black lower occlusion is the authored `shrubbery.png` foreground at alpha 0.95, placed at `kMiddleForeground`. The earlier UV/Y correction remains in place; reducing its alpha, changing depth, moving the layer, or editing the art/XML would contradict the authored Windows contract. No renderer/source changes or tests were added in this diagnostic-only slice.

## Contract audit

- Forest-1 XML ID37 (`Game/XmlFiles/levels/forest_exploration_1.xml:612-635`): `ParallaxLayer`, center `(1316.985,2800)`, 2048×512, `kMiddleForeground`, `alpha=0.95`, `shrubbery.png`, `followcamx=true`, `followcamy=false`, X UV multiplier `0.000690`, Y UV multiplier 0, no flips. The level camera starts at `y=3400` and does not follow Y.
- Windows `ParallaxLayer::Update/Draw` follows camera X, leaves its authored Y, overrides repeat-X to **one** texture span with `-cameraX*0.000690` phase, and keeps bottom-vertex V=0/top-vertex V=1 when Y parallax is disabled. `Sprite::Draw_effectLightTexture` sends the XML alpha; `BasicLightTexture.fx` multiplies sampled texture alpha by that value. `Graphics.cpp` uses source-alpha/one-minus-source-alpha RGB blending and an always-pass depth function, so painter's order is decisive.
- Linux `LevelScene::Draw` follows camera X, keeps Y=2800, applies the same X UV phase, disables ordinary repeat-X on this parallax layer, inverts its SDL Y sample orientation, passes the 0.95 tint alpha, and sorts descending depth like `GameObjectManager.cpp`. No fade-when-occluded applies (`false` in XML). The SDL copy uses alpha blending. These relevant parallax/alpha/depth/occlusion rules match.
- The original RGBA texture is black wherever it has coverage. All 2048 pixels in row 64 and row 128 are `(0,0,0,255)`; all of row 511 is alpha 0; row 448 is variably transparent. With V=1 on the world-top edge, the transparent image bottom appears at the *screen top* of the quad and the opaque black image top descends toward the screen bottom. At camera Y=3400 and port scale 2/3, the quad top projects near screen y=589.3. A soft onset below that and near-black at y≈700 are predicted by the authored geometry and alpha, not evidence for an extra Linux occluder.

## Fresh physical-GPU same-frame check

Forced Docker rebuild: `port/build/id37-parity-followup-2026-10-22/forced-build.log` (exit 0; only existing TinyXML fallthrough warnings). Sequential fresh-process 1280×720 BMPs in the same ignored directory: `baseline.bmp`, `hide.bmp` (`DEATHLEAK_RENDER_HIDE_XML_IDS=37`), and `only.bmp` (`DEATHLEAK_RENDER_ONLY_XML_IDS=37`), with matching logs. All use forest-1, `--script R:180 --frames 180 --fixed-dt 0.0166667`, Xorg `:0`, OpenGL strict accelerated renderer, physical nouveau/NVE7 GPU sepia, 71 loaded sprites/zero missing assets, and identical player `(1368.99,3107.3)`/five orbs. The display cookie was mounted read-only; no credential is included in evidence.

Baseline vs hide-ID37 changes 90,958 pixels within `[0,597,1280,720)` (RGB luminance difference mask). At x=640, y=650 baseline/hide are `(58,50,36)/(74,64,46)`; at y=700 they are `(5,4,3)/(94,82,59)`. The only-ID capture darkens gradually toward the bottom. The fresh baseline is not bit-identical to the earlier ID37 review because subsequent scene/fog changes altered the composite; the within-current triplet is the controlled attribution.

## Actionable parity gap

There is no matched original Windows frame with the same camera XY/zoom, player/state, viewport, effect clocks and postprocess. Steam screenshot 14 is qualitatively compatible with an irregular shrub edge above a black matte but cannot settle pixel parity. The next discriminating step is to capture that matched original frame or instrument the original Windows draw to export ID37's UV transform and post-shader isolated layer; compare it to the present same-frame `only.bmp`. Until then, there is no justified ID37 code fix. No commit or push was made; no unrelated dirty status/notes were touched. GPU capture containers have exited.

# Post-ID37 Forest-1 ID10 player-legibility visual gate — 2026-10-23

## Verdict

**No source-backed ID10 correction; no new GPU hide run warranted.** The current-tree ID37 gate already establishes that, at the late Forest-1 fixed state, hiding ID37 changes zero pixels in the player box whereas hiding ID10 changes 9,285. The earlier film-phase-registered 2×2 GPU gate establishes that authored ID10 strongly attenuates the rendered player, but neither gate supplies a matched original-Windows pixel/draw event. A distinct source-texture/sampler discriminator below reinforces the authored explanation: ID10's mostly binary, opaque tree pixels are drawn at 70% object alpha over the player, and the normal active-gameplay quad is magnified, selecting point magnification in both original and Linux paths. There is no demonstrated blend, order, fade, or filter defect to patch. Do not remove/retune the authored foreground.

## Bounded current-tree evidence and commands

Working tree inspected at `git rev-parse HEAD` = `6dfc65ba086b3009809f9b303d3eccf391818a34`; shared dirty renderer/game/test/status files were not changed. Read-only probes:

```sh
sed -n '176,199p' Game/XmlFiles/levels/forest_exploration_1.xml
sed -n '53,62p' Game/ShaderFiles/BasicLightTexture.fx
sed -n '390,407p' Game/src/Core/Graphics.cpp
sed -n '1313,1317p;1370,1407p' port/game/LevelScene.cpp
sed -n '198,233p' port/linux/SDLRenderer.cpp
sed -n '1320,1327p' port/linux/main.cpp
sha256sum Game/Media/objects/tree2_blur.png port/build/private-current-clip-id37-gate-20261023t2015z/deathleak-linux
python3 - <<'PY'
from PIL import Image
from statistics import median
im=Image.open('Game/Media/objects/tree2_blur.png').convert('RGBA')
a=list(im.getchannel('A').getdata()); nz=[v for v in a if v]
print(im.size,len(nz),len(nz)/len(a),median(nz),max(a))
print(1686.031372*2/3,4158.374023*2/3)
PY
```

Measured source texture `tree2_blur.png`: **669×1650 RGBA**, SHA-256 `daad38d61d3306b6e7df36397113467540463fcd462828936458cc425c7841cb`; 442,185/1,103,850 pixels (40.0584%) have nonzero alpha, and median nonzero alpha is **255**. This is a whole-texture statistic, not a claim that every pixel over the player is opaque. XML ID10 is 1686.031372×4158.374023 world units, yielding a **1124.021×2772.249** nominal 1280×720 destination at the port's `2/3` world-to-screen scale, **1.68015× magnification** versus the source image in both axes. The currently reviewed capture binary SHA-256 is `95b67d671ab4a1aa62e8181c5c084080922b648b52cf8825a5ecb9a0c0739e7b`; its `baseline`/`hide-10` phase-matched packet reports 281,947 total changed RGB pixels and 9,285 in `[810,509,893,621)`. The `baseline-repeat1`/`hide-37` phase-matched packet reports zero changed player-box pixels. Both results are in `port/PRIVATE_CURRENT_CLIP_ID37_READABILITY_GPU_GATE_20261023T2015Z.md` and its ignored `analysis.json`; no capture was added or relabeled here.

The nominal magnification calculation applies to ordinary gameplay zoom=1. `port/linux/main.cpp:1323` uses zoom=1 after the sword pull; the player at this late capture has 5 orbs and progressed through the scripted intro, but the packet does not log zoom numerically. Even if zoom remained above 1, a filter-regression claim would require actual draw-state dimensions. No such claim is made.

## Original draw/occlusion contract and discriminating mechanism

Original XML `:176-199` authors ID10 `tree2_blur.png` at `kMiddleForeground`, horizontal flip, alpha **0.7**, and `fade_when_player_occluded=false`. `Game/src/GameObjects/GameObjectManager.cpp:302-330,1022-1025` orders player depth 3000 before ID10 depth 1000. `DrawableObject.cpp:58-112` enters its player fade only when that flag is true; ID10 therefore does **not** inherit ID6's fade. `Sprite.cpp:626-672` sends a single alpha to `BasicLightTexture.fx:53-59`, which multiplies sampled texture alpha once. `Game/src/Core/Graphics.cpp:392-405` sets source-alpha/inverse-source-alpha RGB overdraw. Where the source texture alpha is 255, ID10's authored RGB-over transmission of an underlying player contribution is approximately **1−0.7 = 0.3**, before subsequent color processing; a hypothetical removal would yield a roughly **3.33×** underlying contribution. This is a source prediction, not a pixel equality: texture sample alpha varies, multiple layers/postprocess intervene, and the older registered Linux gate's measured 3.36× is from a different binary.

Original `BasicLightTexture.fx:11-16` specifies `MIN_LINEAR_MAG_POINT_MIP_LINEAR`. Linux ordinary draw `SDLRenderer.cpp:202-231` chooses nearest unless its projected destination is minified; at the nominal 1.68× ID10 magnification it selects nearest/point, consistent with the original **magnification** branch. Port `LevelScene.cpp:1370-1405` submits the normal sprite with authored alpha as byte tint `179/255≈0.70196` and the mapped layer depth. This tiny quantization is not a plausible explanation for the large legibility observation. The current source and prior CPU command gate `PRIVATE_FOREST1_ID2_ID10_CPU_COMMAND_GATE_C22B60D4_20261023.md` agree on order, alpha, flip and no fade. Neither a new same-state ID10 hide nor a speculative opacity change would distinguish original-Windows fidelity without a synchronized D3D target.

## Remaining parity gate and ownership release

The next actionable fidelity step, **only if original-Windows parity is pursued**, is one lossless registered D3D10 draw/frame packet for this player/trunk pose: ID2 and ID10 painter order, actual ID10 sampled texel/alpha and sampler, blend, pre/post-ID10 target, projection/zoom, animation/film phase, and final render ordinal. Compare against a same-state Linux packet at the same render boundary. A measured discrepancy then supports a narrow renderer correction; absent that, **no change** is the honest verdict. Steam JPEG and asynchronously sampled MP4 remain qualitative references, not the missing matched packet.

No Docker container, Xorg client, capture process, source/XML/art/test, shared status file, Git index, commit or push was created/changed by this lane. `docker ps --format '{{.Names}}'` showed no Death Leak container before this gate; no GPU process was launched. **Exclusive physical Xorg `:0`/NVE7 GPU lane released.** This unique report is the sole write.

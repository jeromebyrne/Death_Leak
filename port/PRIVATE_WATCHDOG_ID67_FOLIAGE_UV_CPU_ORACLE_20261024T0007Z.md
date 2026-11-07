# Waterfall ID67 foliage UV CPU oracle — 2026-10-24 00:07Z

## Result / decision

**Material source-contract mismatch, independently of rotation geometry:** original D3D10 ID67 traverses U=0.9765625→0 after horizontal whole-quad flip, whereas current rotated foliage-sway Linux path traverses U=1→0. On the actual 2048×252 leaf-shadow PNG, a representative opaque/interior row gives effective authored-alpha differences of about **+52/255 at the 10–25% screen-left-to-right positions** (Linux minus source) and ~0 near the right edge. This is not just a transparent seam. It predicts a visible dark-shadow shape/phase difference wherever ID67 survives subsequent overdraw. It is **not** a measured composed-frame or Windows parity claim.

## Source and physical-packet evidence

- `Game/XmlFiles/levels/waterfall_top.xml:1163-1177`: ID67 has width 2000, height 252, rotation +0.080501 radians, `repeatX=true`, `native=false`, `horizontalflip=true`, alpha .65, foliage-sway effect, with top-left/top-right sway intensities (7,6)/(8,9), bottom intensities zero. PNG: `Game/Media/levels/forest_exploration_1/leaves_shadow.png`, 2048×252 RGBA, SHA-256 `fc80390fcb4fb2753af43acaa6c218c687358fa71adb497bde3f6b51511e4010`.
- `Game/src/GameObjects/Sprite.cpp:89-96,215-234`: X flip is applied to the quad transform, while repeatX independently writes U=`2000/2048` to both right source vertices. `Game/ShaderFiles/FoliageSway.fx:14-20,40-65` wraps UV, displaces vertices but passes their UV unchanged, and multiplies texture alpha by authored alpha. Thus the transformed screen-left edge comes from source-right U=.9765625; no whole-image U=1 endpoint is authored.
- `port/linux/SDLRenderer.cpp:245-255,696-753`: foliage-sway dispatch precedes ordinary rotated-repeatX; this draw path tiles only when `rotationDegrees==0`, otherwise draws one quad with X-flipped U=1→0. `port/linux/EffectQuadGeometry.h:25-47` preserves flip/rotation/sway geometry separately; the UV error is not a missing vertex-sway error.
- Frozen physical NVE7 frame-424 packet `port/build/private-watchdog-same-run-415-424-20261023t2352z/gpu424.json`, as inventoried in `port/PRIVATE_WATCHDOG_POST_ID17_NEXT_VISUAL_GATE_20261024T0005Z.md`: ordinal 8 texture 250, destination (-130.810242,330,1333.33337,168), rotation 4.612°, flipX/repeatX/foliageSway true, opacity .65. This is the first clean postfade Linux submission, not a matched original frame; later draws may occlude it.

## Isolated CPU discriminator

At fixed destination/rotation/sway/time/tint/painter slot, let `f` run left→right along any interior interpolated quad scanline. Whole-quad X flip makes original `U=(1-f)*(2000/2048)`; current rotated-foliage branch makes `U=1-f`. The phase offset is `48*(1-f)` source texels, or `31.25*(1-f)` output px at width 1333.33337. This holds for any rotation/sway deformation because vertex UVs are unchanged by either geometry operation. Row 126 is a representative interior PNG row. The following CPU oracle sampled horizontal bilinear RGBA at texel-center coordinate `x=U*2048-0.5`, with wrap; multiplied sampled alpha by .65; and reported premultiplied RGB against transparent black. Rows are illustrative texture samples, not a GPU sampler/mip claim.

| screen fraction f | original U / source x | Linux U / source x | sampled alpha original→Linux | authored alpha original→Linux | premultiplied gray RGB original→Linux |
|---:|---:|---:|---:|---:|---:|
| 0.00 | .9765625 / 2000 | 1 / 2048→0 | 18.5→0 | 12.0→0 | 1.27→0 |
| .10 | .87890625 / 1800 | .9 / 1843.2 | 96→175.4 | 62.4→114.0 | 6.61→12.07 |
| .25 | .732421875 / 1500 | .75 / 1536 | 156.5→236 | 101.7→153.4 | 10.77→16.24 |
| .50 | .48828125 / 1000 | .5 / 1024 | 252→255 | 163.8→165.8 | 17.34→17.55 |
| .75 | .244140625 / 500 | .25 / 512 | 255→249.5 | 165.8→162.2 | 17.55→17.17 |
| .90 | .09765625 / 200 | .1 / 204.8 | 243→243 | 158.0→158.0 | 16.72→16.72 |
| 1.00 | 0 / 0 | 0 / 0 | 0→0 | 0→0 | 0→0 |

Nearest source texels corroborate materiality without relying on filtering: at `f=.10`, x1800 alpha99 vs x1843 alpha176; at `.25`, x1500 alpha159 vs x1536 alpha234. At a hypothetical flat gray-128 backdrop and source RGB 27, `.25` implies about -19/255 output gray for Linux versus original (alpha/255 blend); the actual scene backdrop and later overdraw are unknown. Pixel sample RGB is constant ~27 through these nontransparent locations; the **alpha silhouette**, not RGB texture hue, carries the difference.

**Controls:** If only rotation is set to zero while keeping `repeatX=true`, `port/linux/RepeatX.h:19-46` produces one tile for this width: tile width `2048*2/3=1365.333`, destination width 1333.333, sourceWidth `round(2048*1333.333/1365.333)=2000`, and `DrawFoliageSway` uses flipped U=`2000/2048→0`. Thus original and Linux phase coincide (up to rounding/sampler details). If `repeatX=false` at the actual nonzero rotation, original Sprite.cpp retains default U=0→1 before flip and current foliage branch also uses U=1→0: again no span discrepancy. Either control isolates the conjunction **rotated + repeatX + foliage sway**; changing flipX to false reverses the direction but does not remove the 48-texel span mismatch.

## Reproduction and limits

Read the XML ID67 and PNG dimensions with Python `xml.etree.ElementTree` and Pillow, then for each `f` above compute both U formulas, sample two wrapped texels around `U*2048-.5` on row 126, linearly interpolate all RGBA channels, and multiply alpha by .65. This oracle was run against the current files, without build/display/GPU. It intentionally excludes vertical bilinear filtering, mip LOD, exact SDL/D3D sampler differences, scene alpha accumulation, perspective/raster coverage, overdraw, and a matched original Windows packet. Endpoints at U=1 depend on wrap/filter edge behavior; the interior `.10` and `.25` findings do not.

## Narrow next GPU implementation gate

After the exclusive renderer/GPU owner releases the lane, instrument **ID67 only** at the frame-424 first-active state: capture an ID67-only or immediately-pre/post-ID67 lossless layer, then a final-scene matched A/B and unaffected control crop. Choose an uncovered mid-strip ROI away from HUD/player and report changed-pixel count plus alpha/luma effect; do not infer significance from the CPU oracle alone. If material after overdraw, change only `DrawFoliageSway`'s **nonzero-rotation, repeatX** UV endpoint from 1 to `authoredWidth/textureWidth` (respect flipX and preserve vertex geometry, time/weather, opacity, painter order). Keep zero-rotation tiling and non-repeat branches unchanged. A source-built matched original D3D10 packet remains required for any Windows-pixel-parity claim. If the effect is fully occluded or negligible in the composed frame, close this candidate without a renderer patch.

Scope: only this unique report was written; no source/tests/status edits, build, Docker, GPU/Xorg/capture, commit, or push.

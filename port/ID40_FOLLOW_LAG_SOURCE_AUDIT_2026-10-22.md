# Forest-1 ID40 upper-foreground follow-lag source audit — 2026-10-22

## Verdict

At inspected HEAD `e2c5adf1de43bf694185367b4702acb32c3e32bc`, ID40 has the same **source-backed, transient X-destination timing difference** as a followed ParallaxLayer: original Windows draws its cached matrix at the X entering the latest object update, then follows the current camera X; Linux computes destination from the current draw camera X. Both draw UVs from the current camera X. This is a code-path prediction, not a matched Windows pixel measurement or proof of a material upper-strip mismatch. It cannot change the strip's Y extent, opacity, or broad darkness; after a held-camera update, the X contrast disappears. The authored `trees_top.png` ID40 itself accounts for the *topmost* strip in an older controlled Linux hide-ID capture, while the former broad y112–272 artifact was traced separately to ID47 (see `port/COMPOSITION_DIAGNOSTIC_NOTES.md`). No ID40 art/alpha or renderer correction is justified by this audit alone.

## ID40-specific source contract

- `Game/XmlFiles/levels/forest_exploration_1.xml:666-689` defines ID40 at Y=4100, 2048×677, `kMiddleForeground`, `trees_top.png`, alpha .95, `always_update=true`, `followcamx=true`, `followcamy=false`, X UV multiplier .000690, no X auto-scroll and no horizontal flip. Its initial authored X=1316.985352 matters before the first settled follow update, not in the steady-state calculation below.
- Original `Game/src/GameObjects/ParallaxLayer.cpp:26-35` calls `Sprite::Update` before assigning camera X to `m_position.X`; `GameObject.cpp:160-164,202-212` builds `m_world` from the entering position, and `Sprite.cpp:626-637` submits that matrix. `ParallaxLayer.cpp:37-83` separately reads the current camera X for UV: with no flip, left U=`1−.000690*Cnow`, right U=`−.000690*Cnow`. This assumes the ordinary camera-update → object-update → draw path and successful culling; paused/update-skipped and first-load draws need separate history.
- Current `port/game/LevelScene.cpp:1076-1078,1122-1159` uses `worldX=Cnow` for followed ID40, `destination.x=640+((worldX−Cnow)−1024)×2/3`, and SDL-equivalent `uvShiftX=+.000690*Cnow`. It retains no previous followed X for destination. ID40's height is `677×2/3=451.333333` screen pixels, and X follow cannot alter destination Y.

## Representative moving/held CPU calculation

Use the documented forest camera increment `Cprev=761.040 → Cnow=769.036` from `port/FOREST_FOLLOW_LAG_GPU_DISCRIMINATOR_2026-10-22.md`; this is a reused input, **not** a new ID40 capture. Assume a 1280×720 logical viewport, 2/3 scale, one normal update before each draw, and prior settled follow X=Cprev. ID40 width is 1365.333333 screen pixels. The Windows source's U coordinates and the port's UV phase have opposite signs by convention; both correspond to the same current-camera phase.

| Endpoint | Original cached-matrix X | Original destination left | Linux destination left | Original minus Linux | UV at both destinations |
| --- | ---: | ---: | ---: | ---: | --- |
| Moving, first update at Cnow | 761.040 | −47.997333 | −42.666667 | −5.330667 px | Windows left/right `0.46936516 / −0.53063484`; Linux `uvShiftX=+0.53063484` |
| Held, next update at Cnow | 769.036 | −42.666667 | −42.666667 | 0 px | Same current-camera UV |

The moving source quad's right edge remains x≈1317.336, beyond the 1280-wide view; the Linux edge is x≈1322.667. Thus the 5.331-pixel difference is horizontal image registration, not removal of a full-width top strip. At illustrative camera Y=3400, ID40's port/source Y span from the unchanged formula is approximately −332.333 to 119.000; this is consistent in scale with the earlier ID40 hide contribution at y0–111, but not a same-state pixel comparison. The older hide measurement (`port/COMPOSITION_DIAGNOSTIC_NOTES.md:55-70`) attributes that top-edge contribution to ID40, not to follow-lag, and does not establish Windows parity.

## Next falsifiable CPU-only step

If exact current-HEAD draw-state regression evidence is needed, adapt the existing `CaptureRenderer`/forest XML load pattern in `port/tests/parallax_speed_test.cpp` to filter **only ID40**, recording `DrawSprite` destination, width, height and `uvShiftX` at the moving and next held endpoint above. Current Linux should return x=−42.666667, width=1365.333333, height=451.333333 and U≈+0.53063484 both times. A temporary, render-only previous-X destination seam (without altering UV, Y, alpha, texture or other IDs) would predict x=−47.997333 moving and −42.666667 held. A spy of the unmodified Linux path alone cannot prove the Windows prior-X matrix; original-runtime instrumentation or a same-state Windows frame would be required for pixel/material parity. Do not infer a material upper-band fix from this CPU discriminator.

Scope: source/XML inspection and arithmetic only. No build, Docker, GPU, capture, source/test edits, shared checkpoint change, commit or push; this report is the sole write.

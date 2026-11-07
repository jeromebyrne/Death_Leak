# Forest-1 ID37 follow-lag physical-GPU discriminator — 2026-10-22

## Verdict

The source-predicted one-update destination lag is **real but low-impact in this controlled ID37 frame**. On physical Xorg/NVE7 at a right-moving forest-1 endpoint, moving only ID37's draw destination to the previous update's camera X shifts its quad center **5.331 backbuffer pixels left** while keeping the current-camera U phase. The isolated ID37 crop changes 49,532 pixels, but its mean per-channel RGB difference is only 0.394/255 and its maximum channel difference is 6/255. A five-pixel horizontal registration gives the lowest measured crop error. After a camera hold, baseline and prior-X images are byte-identical. This supports a small, transient visual-contract difference, **not** a darkness fix or a matched Windows parity verdict. Keep authored XML/art and Linux behavior unchanged pending a frame-matched original-Windows trace if exact legacy behavior matters.

## Controlled experiment

- Source HEAD at capture: `814fbd311f84076a31eae46ee0f043a5dfa64489`. Its `c1725ee..HEAD` diff touches none of `port/game/LevelScene.cpp`, `port/linux/main.cpp`, or forest-1 XML. A forced Docker build used a temporary diagnostic-only modification: ID37 destination `worldX` optionally takes the immediately previous draw/update's camera X; its U calculation, alpha, painter order, art, and all other IDs remain untouched. An independent temporary control pins camera X after update/frame 119. Both modifications were restored after capture; `git diff --exit-code` for those source files passed. No software or XML/art change is proposed.
- Four separate deterministic runs: `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:120,F:99700 --fixed-dt 0.0166667`; moving captures at 120 frames (last frame 119), held captures at 122 frames (last frame 121). Each run sets `DEATHLEAK_RENDER_ONLY_XML_IDS=37`, yielding an isolated ID37 render; only prior-X variant sets `DEATHLEAK_DIAG_ID37_PRIOR_X=1`, and both held variants set the same frame-119 camera-hold control. Thus each endpoint pair has identical player/frame input and current-camera UV phase. The camera-hold is a render experiment control, not ordinary gameplay.
- All four logs show SDL X11/OpenGL strict accelerated mode (`accelerated=1 software=0`), physical `nouveau / NVE7`, 71 original sprites, zero missing/unsupported assets. Moving pair ends with the same player `(888.986,3116.18)`, four orbs, 25 health; held pair with `(900.506,3103.79)`, four orbs, 25 health. Wall-clock elapsed differences are not used as simulation time.
- Evidence is in ignored `port/build/forest-follow-lag-gpu-2026-10-22/`: exact capture script, four BMPs/logs, lower ID37-only crops and diff crops, plus the exact original source backup used for restoration. The moving BMP SHA-256 hashes are `ae8bbd8d96cffc3a6f7646f96e32ff0433043297e3d482e6804a4909414269e5` baseline and `e2a87a15176e5cdf5b178d6c12042fb50e39a321647fcf5ba38d5ec8fcaeec6e` prior-X. Both held BMPs hash `42f4990705b2c22033e474ca1d2c38d4da2167d634f0d01e8f94cc033cc5ce63`.

## Geometry and pixel result

| Endpoint | Previous X | Current X | Delta X | ID37 center baseline → prior-X | U phase both | Changed pixels in crop `x=0..1279,y=580..719` |
|---|---:|---:|---:|---:|---:|---:|
| Moving, frame 119 | 761.040 | 769.036 | +7.996 | 640.000 → 634.669 (−5.331) | 0.530635 | 49,532 |
| Held, frame 121 | 769.036 | 769.036 | 0 | 640.000 → 640.000 | 0.530635 | 0 |

The measured center offset agrees with `−ΔcameraX × 2/3 = −5.3307` pixels at 1280×720. Moving crop difference occupies `x=0..1279,y=600..719`; mean absolute RGB delta over the full 1280×140 crop is `(0.462,0.439,0.279)` and maximum channel delta is 6. When aligning the prior-X crop to baseline with integer horizontal displacements 0–8, RGB MAE falls from 0.394 at zero to a minimum **0.118 at five pixels**, then rises to 0.152 at six. The residual is expected from fractional-pixel sampling and changed edge coverage. The held-frame crop has zero differing pixels and the full BMPs are byte-identical.

These are within-port, ID37-isolated physical-GPU measurements. They establish geometry and practical contrast at one deterministic forest-1 endpoint only. They do not prove the old Windows executable's final matrix at that frame, an effect of ID40, or exact screenshot parity. The tiny per-channel difference in this dark lower strip does not justify emulating a likely legacy transform lag or changing brightness, alpha, XML, or art.

## Cleanup

The temporary source edits were restored exactly. The diagnostic binary was rebuilt from restored source (`make -C port -B -j2`, exit 0), yielding SHA-256 `a10f8b81dd60c2897dbd3a06a6eb2ce5405072d1d296dd99f9e0ca8fbbd1ce29`, matching the prior current-HEAD build. No commit/push is warranted for a report-only discriminator. No named capture/build container remains; the physical GPU/display and Docker build lane are released. `CHECKPOINT_STATUS.md` and `PORT_WORK_STATUS.md` were not edited.

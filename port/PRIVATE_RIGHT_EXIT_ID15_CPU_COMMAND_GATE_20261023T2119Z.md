# Private Forest right-exit ID15 CPU command gate — 2026-10-23

## Result

**PASS at the illustrative camera** `C=2360.52,D=3400`, player `(2360.52,3114.74)`, one `UpdateVisuals(1/60)` then `LevelScene::Draw`. A CPU-only `Renderer` spy recorded production `DrawSprite` submissions from the shipped `Game/XmlFiles/levels/forest_exploration_1.xml`. Exactly eight selected objects were admitted in painter order **19, 21, 38, 17, 22, 39, 15, 37**. ID15 was submitted once, ordinal 6 zero-based, with `sun_shaft_2.png`, logical destination `(760.31,318,341.333,341.333)`, rotation `+84.2247°`, flipX=false, flipY=true, repeatX=false, full-texture source rectangle zero, no UV shift, white tint alpha 255, opacity 1, and no foliage/pixel effect. This closes the bounded **Linux production command-submission question at this stated test camera only**; it is not a frame-540 replay or original-Windows parity result.

The independently compiled fixture is `port/tests/private_right_exit_id15_command_spy_20261023.cpp`. It asserts unique shipped XML IDs, their depth/effect tags, and then actual command count, order/path, destination, source rectangle, rotation, flips, repeat, tint, float opacity, UV shift, and effect flags/RGB gain for all eight commands. Texture handles are stable nonzero synthetic IDs, and paths are retained by the spy; duplicate `blank_white.png` IDs 38/39 are separated by insertion order, destination and alpha. Filter `DEATHLEAK_RENDER_ONLY_XML_IDS=15,17,19,21,22,37,38,39` is set before first Draw; the hide filter is unset. No production code is instrumented.

## Reproduction and numeric trace

From repository `source/`:

```sh
g++ -std=c++17 -O0 -Iport/compat port/tests/private_right_exit_id15_command_spy_20261023.cpp port/game/LevelScene.cpp Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp -o /tmp/private_right_exit_id15_command_spy_20261023
/tmp/private_right_exit_id15_command_spy_20261023
```

The isolated compile and run passed on this host; no shared build artifact or game binary was used. Observed command destinations, rounded here from the fixture output:

| ID | Path suffix | `(x,y,w,h)` | rotation | alpha | uvShiftX |
|---|---|---|---:|---:|---:|
| 19 | ground_blur.png | `(-2526.98,401.429,4902.75,682.667)` | 0 | 255 | 0 |
| 21 | tree_strip_dark.png | `(-1034.81,-269.947,1323.33,825.333)` | 0 | 255 | 0 |
| 38 | blank_white.png | `(-6707.18,-674.741,13333.3,1828.92)` | 0 | 38 | 0 |
| 17 | ground.png | `(-2882.14,464.827,4902.75,682.667)` | 0 | 255 | 0 |
| 22 | tree_strip.png | `(-2167.14,-166.187,3364.74,825.333)` | 0 | 255 | 0 |
| 39 | blank_white.png | `(-7471.02,-564.461,13333.3,1828.92)` | 0 | 51 | 0 |
| 15 | sun_shaft_2.png | `(760.31,318,341.333,341.333)` | 84.2247 | 255 | 0 |
| 37 | shrubbery.png | `(-42.6667,589.333,1365.33,341.333)` | 0 | 242 | 1.62876 |

## Oracle correction discovered

The referenced `PRIVATE_FOREST_RIGHT_EXIT_COMMAND_AUDIT_20261023T2114Z.md` had **incorrect Y formulas for IDs 19, 21, 38, 17 and 22**, not a runtime mismatch. Its table used `D/3` or `D/2` for those backgrounds. Production `LevelScene.cpp` applies vertical parallax **0.97 at depth 5500** and **0.99 at depth 5000**, then the `2/3` screen scale. The correct source-backed form is `360 + ((D - authoredY) * layerY - authoredHeight/2) * 2/3`; the fixture uses shipped XML values and that formula. Example ID19: audit predicted `y≈215.967` at D=3400, but production submits `y=401.429`. ID15's previously stated `y=318` remains correct (ground-front layerY=1), as do IDs 39 and 37 at this camera. All eight corrected expected fields match production within explicit float tolerances; this report supersedes only those audit Y fields.

## Bounds and evidence identity

`DrawSprite` is before SDL/GL processing: this gate cannot observe actual texture dimensions/sampling, alpha blend, tile sub-copies for repeatX, projected GPU pixels, backend branch for ID37, or rejected pre-cull objects. It does not include a measured active-game frame-540 draw camera, its visual history/shake, or a matched Windows command/frame. ID15 admission is inferred from its one recorded command after `SpriteMayIntersectViewport`, not a direct pre-cull hook. The camera is explicitly illustrative, not the historic frame-540 camera.

SHA-256: fixture `42c95bd10604a9c779da946f851c365ba0b97b4d089f34628c4f0a42149f452a`; `port/game/LevelScene.cpp` `37c336ddd0b8b8da319b73d7a82fedff8fbab0dc5058f21eceec0be52a11cee6`; shipped Forest XML `6d36a837708a732ac7d2a085010a3d22229e29618b8594b5c2576d491c4161c5`.

Scope: only this uniquely named report and fixture were written. No production source, existing file, shared checkpoint/status, Docker, GPU/Xorg, Git index, or shared build was touched.

# Private Forest-1 ID6 occlusion CPU draw-command gate — 2026-10-23

## Verdict

**PASS for the bounded original-source-to-current-port command contract at HEAD `e9e6be4`; no ID6 mismatch proven.** A new standalone CPU fixture loads the shipped Forest-1 XML via production `LevelScene::Load`, steps `UpdateVisuals(1/60)`, and spies actual `LevelScene::Draw` sprite commands. It identifies ID6 by the XML render-only ID filter **plus** its authored destination width/height, not by `tree2.png` alone (ID5 shares that texture). ID2 player body draws before ID6 `kGroundFront`, which draws before ID10 `kMiddleForeground`. ID10 is a nonfading control: its command stays opacity `0.7`, tint alpha `179` across all checked states.

## Exact observed ID6 sequence

| State after update | Command opacity | Tint alpha byte |
| --- | ---: | ---: |
| Player center exactly on left edge (outside strict rectangle) | 1 | 255 |
| First center-inside tick | 0.975 | 249 |
| 24 center-inside ticks | approximately 0.4 | 102 |
| 25 center-inside ticks | approximately 0.375 | 96 |
| Next inside tick (clamp) | 0.4 | 102 |
| First left-edge exit tick | 0.425 | 108 |
| 23 exit ticks | 0.975 | 249 |
| 24 exit ticks | approximately 1 | 255 |
| 25 exit ticks | approximately 1.025 | 255 (byte saturated) |
| Next exit tick (clamp) | 1 | 255 |
| Center-inside player, camera `(10000,10000)` outside ID6 update zone | 1 (frozen) | 255 |
| Camera returned to ID6 center, player inside | 0.975 | 249 |

The apparent one-frame 0.375 and 1.025 overshoots are **not port-only defects**: `DrawableObject.cpp:15,41-112` uses `1.5f*delta` and compares current alpha to target *before* stepping, clamping only on the following update. At `dt=1/60`, float accumulation around 0.4 and 1.0 permits that extra step. The port `LevelScene.cpp:919-942` uses the same compare-then-step ordering. It submits `draw.opacity=object.alpha` and a rounded, clamped 8-bit tint in `:1403-1405`; thus the 1.025 float-of-record appears as tint 255 in ordinary sprite copies. No source-contract divergence to hand to the scene owner.

## Reproduction and scope

From `/srv/openclaw/workspace/death_leak_analysis/source`:

```sh
c++ -std=c++17 -O0 -Iport/compat port/tests/private_forest1_id6_occlusion_cpu_gate_20261023t1936z.cpp port/game/LevelScene.cpp Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp -o /tmp/private_forest1_id6_occlusion_cpu_gate_20261023t1936z
/tmp/private_forest1_id6_occlusion_cpu_gate_20261023t1936z
```

Compiler exit 0; fixture exit 0 with `PASS: production Forest-1 ID6 CPU draw-command fade/strict-boundary/order/freeze contract`. Fixture: `port/tests/private_forest1_id6_occlusion_cpu_gate_20261023t1936z.cpp`. Authored ID6 XML `Game/XmlFiles/levels/forest_exploration_1.xml:116-129`: center `(1339.985352,3895)`, size `1112.317383×2743.384033`, alpha 1, `kGroundFront`, fade true to 0.4 and back to 1. Source drawing sends alpha to `BasicLightTexture.fx` via `Game/src/GameObjects/Sprite.cpp:626-637`. This CPU gate tests neither GPU pixels nor matched original-Windows rendering.

Only this new uniquely named fixture and report were written in the repository; executable is isolated under `/tmp`. No production files, existing tests, unrelated dirty files, shared status/checkpoints, shared build binary, GPU/Docker, Git index/commit/push were touched. **CPU lane explicitly released.**

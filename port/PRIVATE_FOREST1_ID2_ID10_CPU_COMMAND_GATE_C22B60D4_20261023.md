# Private Forest-1 ID2/ID10 CPU draw-command gate — 2026-10-23

## Verdict

**PASS at HEAD `e10084a` for the bounded Linux CPU command contract.** The production `LevelScene::Load/UpdateVisuals/Draw` path, with the original Forest-1 XML and a spy renderer, submits the player ID2 body before ID10's `tree2_blur.png`. At fixed player/camera `(2171.985352,3375)`, within ID10's authored rectangle, a `1/60 s` visual update retains ID10 opacity `0.7` and byte tint alpha `179`. The ID10 command is horizontally flipped, and its SDL-facing vertical flip is true (the authored false inverted for SDL's V convention). This rules out the proposed load/update/order regression in this state. It is not a pixel, GPU, or matched-Windows parity result; no production patch is justified.

## Exact run

From `/srv/openclaw/workspace/death_leak_analysis/source`:

```sh
git rev-parse --short HEAD
c++ -std=c++17 -O0 -Iport/compat port/tests/private_forest1_id2_id10_command_gate_c22b60d4.cpp port/game/LevelScene.cpp Game/tinyxml/tinyxml.cpp Game/tinyxml/tinyxmlerror.cpp Game/tinyxml/tinyxmlparser.cpp Game/tinyxml/tinystr.cpp -o /tmp/private_forest1_id2_id10_command_gate_c22b60d4
/tmp/private_forest1_id2_id10_command_gate_c22b60d4
```

Results: `e10084a`; compiler exit 0; test exit 0 with `PASS: Forest-1 ID2 before ID10; texture/flip; no occlusion fade; ID10 opacity=.7 tint.a=179`.

The standalone test is [private_forest1_id2_id10_command_gate_c22b60d4.cpp](tests/private_forest1_id2_id10_command_gate_c22b60d4.cpp). It asserts the shipped XML ID2 `kPlayer` / ID10 `kMiddleForeground` layers (port depth map 3000/1000), ID10 texture/flip/alpha/fade flag, and records actual `DrawSprite` calls after the update. The render-only filter selects IDs 2 and 10 without changing source. The body-path command is before the unique ID10 texture-path command, and ID10 has `opacity=.7`, `tint.a=179`, `flipX=true`, `flipY=true`. `opacity` is command metadata; `SDLRenderer.cpp:220-234` uses the tint byte as the ordinary copy's alpha modulation, not an additional multiplication by `opacity`. The byte conversion represents `179/255≈.70196`; this CPU test does not sample or blend pixels.

## Source trace and limits

Original XML `Game/XmlFiles/levels/forest_exploration_1.xml:33-46,176-190` authors ID2 first at `kPlayer`, and ID10 at `kMiddleForeground`, alpha `0.7`, `tree2_blur.png`, horizontal flip true, and `fade_when_player_occluded=false`. Original `GameObjectManager.cpp:302-330,1021-1025` sorts and traverses drawables by depth, so ID2 precedes ID10; `DrawableObject.cpp:41-55` enters fade only under the true flag; `Sprite.cpp:626-637` sends alpha to the effect. Port `LevelScene.cpp:28-40,556-574,624,918-942,1335-1350,1385-1388,1431` maps the layers, loads alpha/fade, sorts descending, gates fading, and submits the command. The compiled gate exercises this production path, but cannot establish the original Windows runtime's pixels or a same-state residual. The separately owned Windows matched-frame lane remains the next threshold only if a visual parity claim is needed.

No GPU/Xorg, Docker, Wine/Windows, shared build directory, production source, shared checkpoint/status, Git index/commit/push, or unrelated dirty work was touched. Only the uniquely named private test and this report were written in the repository; the test executable is isolated under `/tmp`. CPU lane released.

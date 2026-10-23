# Fresh-save X prompt GPU integration — 2026-10-23

## Result

The Linux fresh-save forest-1 X prompt now draws in a player-adjacent callback immediately after the player body's `LevelScene::Draw` work, before the next same-depth sprite. This replaces the depth-2999 approximation. The callback is opt-in and leaves other painter items' sort/order intact. `--capture-suppress-intro-prompt` is a capture-only A/B diagnostic; `--steam-deck` explicitly selects Deck art because the Steamworks device query is unavailable. No Windows pixel parity or Jerome acceptance is claimed.

## Source and CPU gate

- Original source: `Game/src/GameObjects/Player.cpp:986-1000,1177-1202` advances the phase total while Looped and draws after `Character::Draw` only when strictly >8 seconds and raw Interact is up; `DrawUtilities.h:24-66` supplies the authored world quad. `GameObjectManager.cpp:302-329` visits each object draw as a unit. The previous integer depth-2999 slot was after all depth-3000 objects, not immediately after the player.
- Changed `port/linux/main.cpp` and the narrow `port/game/LevelScene.{h,cpp}` player-adjacent slot. The X remains world-space at player (-35,+50), default 150x90 or Deck 60x60, through the same shaken camera and world zoom; scene postprocess still precedes HUD.
- New private fixture `port/tests/fresh_save_x_prompt_integration_test.py`: PASS. It compiles the production clock and checks strict >8, release/rehold accumulation, held suppression/reset, art/geometry, and source-coupled draw ordering. It is not a renderer spy. `g++ -std=c++17 -fsyntax-only -Iport -Iport/linux port/linux/main.cpp`: PASS. `git diff --check` for owned files: PASS.
- The predecessor `fresh_save_x_prompt_private_cpu_test.py` still asserts the superseded depth-2999 expression. It was not modified outside this ownership slice; use the new integration fixture for this version.

## Current-tree Docker / physical NVE7 gate

Before build/capture, process check found no concurrent `make`, `g++` or game capture; Xorg :0, `/dev/dri/renderD128`, and `deathleak-linux-build:bookworm` were present. `docker run ... deathleak-linux-build:bookworm make -j2` completed exit 0 with only pre-existing TinyXML warnings. Binary SHA-256 `6e2b9d3a48b4a72a0a79cd36cfde430d945ac78b13526913b137db69d5ab45f9`; main source SHA-256 `6b577bf7bbb22650eec9a8cca8e0ac96921afd2ebd89d7b54351c979885f690e`. All captures used the same current-tree binary, `--fresh-save-intro --play Game/XmlFiles/levels/forest_exploration_1.xml --fixed-dt 0.0166667`, fresh per-process startup, actual Xorg :0 (`--network host --hostname jerome-Linux-MacBookPro`, mounted LightDM Xauthority and `/dev/dri`). Logs identify `video_driver=x11 name=opengl accelerated=1 software=0 mode=strict-accelerated`, `vendor=nouveau renderer=NVE7`, GPU sepia, and 71 original level sprites loaded. Files: `port/build/fresh-save-x-prompt-gpu-integration-2026-10-23/` (BMP/log pairs).

- Frame 479 unheld, script `.:479`: pre-threshold (`479 × 0.0166667 ≈ 7.983s`) capture `pre-479.bmp`.
- Frame 481 unheld, script `.:481`: after strict threshold (`≈8.017s`) capture `unheld-481.bmp`. Same-frame/same-history `off-481.bmp` used only `--capture-suppress-intro-prompt`. RGB differences are confined to bbox `[567,429,665,482)` with 2,922 differing pixels; no HUD/body/burst relocation or second icon was seen in this matched A/B. This is a direct physical prompt pixel gate, not a Windows comparison.
- Frame 481 held, script `.:480,I:1`: `held-481.bmp`; frame 482 released, script `.:480,I:1,.:1`: `released-482.bmp`. The source/CPU predicate hides while held and restores immediately on release. Same-history suppression controls `held-off-481.bmp` and `released-off-482.bmp` were captured. The released pair differs only in the same X bbox (2,940 pixels). The held pair and pre-threshold pair have broad scene differences across separately launched runs, so they are **not** valid zero-difference pixel proofs; wall-clock-driven shader/animation state varied between runs. They remain physical state captures plus CPU predicate evidence, not causal A/B isolation for absence.
- Frame 481 `--steam-deck`, script `.:481`: `deck-481.bmp`. Against the default-art off-481 control, differences are confined to bbox `[597,436,636,475)` (1,216 pixels), narrower than default art as authored.

Captures and logs preserve actual viewport 1280×720 and fresh-start scripts. Exact per-frame shake/zoom and original Windows rendered reference are not available in these files, so no Windows pixel parity claim is made. The clock times above are fixed-delta arithmetic; final log `elapsed` is wall time, not the prompt phase clock.

## Ownership release

Owned files: `port/linux/main.cpp`, the narrow `port/game/LevelScene.h` and `.cpp` hook, new private fixture, this report, and private capture directory. No shared status, renderer, combat, other tests, or unrelated dirty changes were edited. The Docker build and physical Xorg :0/NVE7 GPU lane are complete and released. The listed files are released after this handoff.

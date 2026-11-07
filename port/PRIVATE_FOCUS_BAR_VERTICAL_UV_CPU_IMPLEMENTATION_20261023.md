# Private focus-bar vertical UV CPU handoff — 2026-10-23

## Source contract

`Game/XmlFiles/UI/game_hud.xml:88-98` gives `91_xp_meter` `vflip="true"` and `xp_meter_bar_middle.png`. `Game/src/UI/UISprite.cpp` reads that flag and uses vertically reversed texture coordinates in `Reset` and buffer rebuild. `Game/src/UI/UIMeter.cpp` changes horizontal flip per meter piece but does not clear vertical flip before drawing the colored fill. Linux `port/game/Hud.cpp` previously omitted `flipY`, defaulting false in `port/render/Renderer.h`.

## Bounded change

Only the focus fill `DrawSprite` aggregate in `port/game/Hud.cpp` now supplies `1.f, 0, false, true` for opacity, rotation, horizontal flip, and vertical flip. The preexisting texture, source, destination, tint, progress timing, and following HUD commands are unchanged. No renderer-wide mapping or other HUD command was modified.

Added `port/tests/focus_bar_vertical_uv_private_cpu_spy_20261023.cpp`: a renderer spy maps the loaded focus texture by its path/ID, drives actual `Hud::Draw` at positive fill (initial, settled, decrease, increase), asserts positive width and expected timing, destination, tint, source, opacity, rotation, X/Y flip and neutral extended fields. It checks nine sprite commands and asserts non-focus sprites remain `flipY=false`.

## CPU verification

- Focus vertical UV private spy: PASS (`g++ -std=c++17 -O0 -g ... port/game/Hud.cpp`, output in `/tmp`).
- Existing `hud_transition_cpu_spy_test.cpp`: PASS.
- Existing `health_meter_length_cpu_spy_test.cpp`: PASS.
- `git diff --check -- port/game/Hud.cpp`: PASS.

There was no active compiler/build process at preflight. First draft spy had mistaken sprite ordinals and a mistaken preexisting focus-right-edge `flipX` expectation; corrected those fixture assumptions. The implementation itself was unchanged after the first application. No shared build directory, Docker, Xorg/GPU, capture, Git index, commit, or shared status was touched. Unrelated dirty files were preserved.

## Release / acceptance boundary

This CPU implementation lane is released to the parent/exclusive GPU owner for integration. CPU draw-command parity with source vertical UV is established; Windows pixel parity and visual acceptance are **not** claimed.

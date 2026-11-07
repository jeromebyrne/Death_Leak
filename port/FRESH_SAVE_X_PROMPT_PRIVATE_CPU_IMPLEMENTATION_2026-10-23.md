# Fresh-save X prompt — private CPU implementation (2026-10-23)

## Scope and source

Owned edits: `port/linux/main.cpp`, new private `port/tests/fresh_save_x_prompt_private_cpu_test.py`, this report. No shared status, renderer, scene, combat, camera-shake, existing-test, build, or GPU file was edited. Source contract: `Game/src/GameObjects/Player.cpp:991-1000,1177-1202`, `Game/src/Utils/DrawUtilities.h:24-66`, and `port/FRESH_SAVE_POST_INTEGRATION_NEXT_VISUAL_GATE_2026-10-23.md`.

## Implementation

- Added a cumulative Looped-phase clock, advanced once per active gameplay update regardless of held Interact. It is distinct from the existing 2.75-s continuous held clock, persists across release/rehold, and resets on Pull, completion, and scene replacement. Prompt predicate is Looped, strictly `>8.f`, raw Interact up. The held pull threshold, control suppression, zoom, shake, burst and blood sites are unchanged.
- Loads `x.png` by default; explicit `--steam-deck` selects `x_deck.png`. Linux has no Steamworks Deck query, so controller connection does not select Deck art. Draws one default-opaque white sprite in the world painter pass using the same shake-adjusted camera and renderer world zoom as the body, centered at player `(-35,+50)`, with authored 150x90 or 60x60 world dimensions. HUD remains after scene postprocess.
- Original depth is `kPlayer+0.1`. `PainterItem.depth` is integral and sorts descending; depth 2999 is the nearest front-of-player bucket after the 3000 player draw and before lower-depth foreground. Fractional depth cannot be represented without changing a shared type outside this ownership slice. This is a painter-order approximation, not exact numeric depth parity.

## Verification and limits

- `python3 port/tests/fresh_save_x_prompt_private_cpu_test.py`: PASS. The fixture compiles/runs the production clock, covering strict threshold, hold/release, cumulative release/rehold, reset and phase suppression. Source-coupled checks confirm single clock advancement, art/dimensions, world offset, draw-camera and pre-postprocess placement. Static checks are not a renderer-spy or pixel test.
- `g++ -std=c++17 -fsyntax-only -Iport -Iport/linux port/linux/main.cpp`: PASS.
- `git diff --check -- port/linux/main.cpp`: PASS.
- No shared make/Docker build or physical `:0`/GPU capture was run. No matched Windows frame or human acceptance was obtained; visual parity is **not claimed**. A real renderer-spy and GPU prompt-off/on capture remain integration gates.

## Handoff

The patch remains uncommitted/unpushed so the integration owner can combine it with concurrent dirty shared work without a branch/push race. Unrelated dirty files were preserved. **Exclusive ownership of `port/linux/main.cpp`, `port/tests/fresh_save_x_prompt_private_cpu_test.py`, and this report is explicitly released.**

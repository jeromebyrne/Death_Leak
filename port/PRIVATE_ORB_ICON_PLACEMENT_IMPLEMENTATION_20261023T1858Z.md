# Private orb-icon source-exact placement implementation — 2026-10-23 18:58 UTC

## Change

`port/game/Hud.cpp` now projects the `40_orb_count` XML rectangle from the original 1920×1080, centered Y-up UI into the port's 1280×720, top-left Y-down viewport. XML `posX=-900`, `posY=360`, `width=101`, `height=64` yield destination `{x=40, y=77.3333, w=67.3333, h=42.6667}`. This replaces the approximate `{40,80,67.3,42.7}`. The sprite retains alpha byte 191 and default `flipY=false` (also `flipX=false`). No renderer path or text placement changed.

## Focused verification

- Compiled `port/game/Hud.cpp` and a temporary C++ renderer spy outside the repository (`/tmp/deathleak_orb_hud_spy_20261023.cpp`) with `c++ -std=c++17 -I.`. The spy loaded the HUD, recorded its first draw command, and compared destination against an independent projection of the XML constants, plus image identity, alpha, opacity and flip flags. Result: `PASS orb destination 40,77.3333,67.3333,42.6667 alpha=191 flipY=0`.
- `git diff --check -- port/game/Hud.cpp` passed; the scoped diff contains only the authored-orb destination and explanatory comment.

## Limits and lane release

This is a source/CPU command verification, **not matched-Windows visual acceptance**. No D3D runtime, GPU, Xorg, SDL pixel capture, or actual Windows reference frame was exercised; blending/sampling and full HUD visual parity remain outside this slice. The preceding orientation gate (`PRIVATE_ORB_ICON_VFLIP_CPU_GATE_20261023T1857Z.md`) explains why XML `vflip=true` does not warrant Linux `flipY=true`. Unrelated dirty files, including `port/linux/SDLRenderer.cpp`, were not modified. **Exclusive orb-icon placement lane released upon completion.**

# Post-ID37 read-only Forest-1 visual gate — 2026-10-23 20:20 UTC

## Nomination

**Orb-balance numeral face is the highest-confidence, source-proven distinct active Forest-1 visual fidelity gap outside ID37.** The original HUD requests Windows **Impact** via D3DX10Font; the current port draws custom **5×7 filled-cell digits**. This affects a small HUD area, not the lower-band/foliage area; “highest” here means confirmed discrepancy, **not** largest pixel footprint. Player darkness under ID10 may have larger visual impact, but authored foreground occlusion and no same-state Windows reference prevent calling it a proven port defect. Foliage ID41–45 mip/wrap remains a conditional sampling candidate, not proven original-pixel mismatch. No parity claim follows.

## Direct source and current-port evidence

- `Game/src/UI/UIGameHudScreen.cpp:230-240,244-260` draws the spendable orb count using `DrawTextA` at original UI rectangle `(105,135)`, with `D3DX10_FONT_DESC.Height=32`, `MipLevels=1`, and `FaceName=L"Impact"`. `Game/XmlFiles/UI/game_hud.xml` contains the active HUD orb sprite. Thus this is active-gameplay HUD, not a menu/dead path.
- `port/game/Hud.cpp:85-91` draws the icon then the same balance with `DrawBitmapText(...,70,90,3,{179,179,179,204})`. Anchor `(70,90)` and tint are the 2/3 projection and byte-rounded original values, so the bounded difference is **glyph form/raster**, not placement or color. `port/game/BitmapText.h:7-9,45-65` defines and paints 5×7 digit masks as square rectangles; it cannot render Impact outlines.
- `port/PRIVATE_CURRENT_CLIP_PLAYER_HUD_LEGIBILITY_GATE_20261023T2000Z.md` records visibly blocky current-tree digits in the 10-second Forest-1 clip. `port/PRIVATE_HUD_IMPACT_NUMERAL_FIDELITY_20261023T1942Z.md` already owns this issue and establishes the blocker: no authenticated usable Impact face in accessible font paths; Noto/DejaVu fallback is not equivalent. This report is coordination, not a duplicate implementation claim.

## Executable next bounded gate

After the ID37 worker releases the build/GPU lane, **first acquire an authorized authentic Impact font file** and record face identity, SHA-256, path and permission to use it. If unavailable, stop: no substitute-font patch or extra GPU run. If available, the HUD owner alone should make an **orb-count-only** font/atlas variant (leave other 5×7 callers unchanged), preserving original `Height=32`, projected anchor, tint and draw order. CPU-gate atlas identity and deterministic masks plus decimal cases `0, 5, 12, 375`; then, under the exclusive GPU lane, capture one controlled original-current HUD comparison with identical balance/state. A same-render-ordinal original-Windows lossless HUD crop remains required for pixel-parity acceptance; current Linux stills alone can show the correction's effect, not prove Windows equivalence. Do not change ID10 opacity or foliage sampling based on this gate.

Only this uniquely named report was written. No source/XML, shared status, build, game, Docker, GPU capture, commit or push was touched. Read-only lane released.

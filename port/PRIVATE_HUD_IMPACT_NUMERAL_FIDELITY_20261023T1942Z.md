# Private HUD Impact numeral fidelity gate — 2026-10-23 19:42 UTC assignment

## Result

**Blocked at the required authentic font asset; no defensible numeral-fidelity source change was made.** The source-backed correction is to rasterize the original Windows `Impact` face for the orb count, not to substitute a different installed font or rescale the existing 5×7 mask. Neither a usable Impact TTF/OTF nor its rights/provenance is present in the accessible checkout/system paths. Therefore this lane did not generate a replacement atlas, make an unverified code change, commit/push software, or claim current visual parity.

## Focused evidence

- Original `Game/src/UI/UIGameHudScreen.cpp` uses `D3DX10_FONT_DESC` with `Height=32`, `Width=0`, `Weight=0`, `FaceName=L"Impact"`, and `DrawTextA` for the decimal spendable orb balance. Draw rectangle starts at `(105,135)` in 1920×1080 UI; tint is `(.7,.7,.7,.8)`; draw follows children. The source does **not** include actual font outlines or define Windows D3DX raster pixels.
- Port `port/game/Hud.cpp` currently calls `DrawBitmapText(renderer, std::to_string(orbBalance), 70, 90, 3, {179,179,179,204})` after the orb icon. `port/game/BitmapText.h` emits fixed 5×7 filled cells. `(70,90)` is the correct 2/3 projected anchor and byte tint rounds the source floats correctly; the missing face is the isolated defect.
- The predecessor `PRIVATE_HUD_GLYPH_SOURCE_CONTRACT_20261023T183754Z.md` and `PRIVATE_ORB_HUD_IMPACT_FACE_FEASIBILITY_20261023T1854Z.md` had already established the same blocker. This lane rechecked `/usr/share/fonts`, `/usr/local/share/fonts`, and `/home/openclaw` for case-insensitive exact `impact.ttf`/`impact.otf`; none was found. Earlier broader workspace/mount scan and `fc-match Impact` proved the only resolved Linux face was Noto Sans fallback. The bundled DejaVu Sans example is not Impact. Current HUD source files are clean at HEAD `e9e6be4e86e345705d3a318ba6b9609154516668`.

## Verification disposition

No code was changed, so a new CPU/build gate would reverify the existing fallback rather than a correction. A physical Xorg `:0` NVE7 ON/OFF comparison is **not feasible** without an ON variant bearing the authentic Impact face; any Noto/DejaVu or hand-stretched bitmap comparison would answer a different question. The prior strict accelerated current-tree 300-frame active-gameplay capture and its logs remain available in `port/build/private-current-tree-active-gameplay-20261023t1939z/`, as documented by `PRIVATE_CURRENT_TREE_ACTIVE_GAMEPLAY_VISUAL_GATE_20261023T1939Z.md`. It shows the known blocky digits but is not matched-Windows evidence.

## Actionable handoff

Supply an actual authorized Impact font file (record its path, SHA-256, face identity, and use rights). Then implement an orb-only raster/atlas path, retaining the other bitmap-text callers, and CPU-check decimal values 0, 5, 12, 375, anchor/color/order, atlas identity/mask determinism, and fallback rejection. Only then run an isolated strict physical NVE7 current-tree capture against the old glyph path and seek a state-matched original Windows frame. Corrected gameplay clip delivery and Jerome review remain outstanding; no global parity claim follows from this gate.

## Ownership release

No production source, tests, shared status/checkpoint, existing report, Git index, commit or push changed. The only output is this unique report. No Docker build/capture or physical Xorg GPU process was started; process inspection found no competing Death Leak game/FFmpeg capture/build process. **Exclusive Docker build and physical Xorg `:0`/NVE7 capture lane released.**

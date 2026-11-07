# FPS HUD source QA (read-only, 2026-10-23)

## Source-parity finding

The shipped Windows game loop has **no in-game FPS readout** in the inspected game path: `Game/src/program.cpp:140-179` updates, renders, and presents; `:293-324` draws game and UI then swaps, with no FPS text call. `Game/src/UI/UIGameHudScreen.cpp` likewise has no FPS text. The bundled but separate DXUT utility does compute a once-per-second floating-point `"%0.2f fps "` string (`Game/DXUT/DXUT.cpp:6085-6120`); no `DXUTGetFrameStats`/`DXUTGetFPS` call exists in `Game/src`. Thus the new Linux counter is a port-only diagnostic, not a visual parity requirement. Do not treat DXUT's two-decimal format or one-second cadence as evidence of an original game HUD.

## Current Linux integration / verification checklist

- `port/linux/main.cpp:523-539` initializes a monotonic sampling window and, each loop, updates a rounded integer after >=0.5 wall-clock seconds. `:1446-1447` counts completed `EndFrame` calls. Check a live play session: initial `FPS --` becomes a plausible nonzero integer after ~0.5 s and changes with render load; fixed/capped simulation delta should not dictate it.
- `port/linux/main.cpp:1422-1426` draws the readout only in the `playLevel` gameplay branch, at virtual (10,693), 2x 5x7 white glyphs. `port/game/BitmapText.h:8-69` supports every glyph used in `FPS --` and decimal digits. Check bottom-left visibility in normal gameplay and against light/dark scenes.
- Check overlay ordering: paper and upgrade modals are drawn **after** the counter (`main.cpp:1427-1438`), and death art is drawn last (`:1440-1441`). If the counter is meant to be always visible, it will be occluded by those overlays. This is a concrete layering defect for the death state (full-screen death sprite); move the diagnostic draw after overlays if always-on visibility is the requirement.
- The menu / options / language paths (`main.cpp:1288-1325`) and the `playLevel.empty()` scene branch (`:1326-1329`) have no counter. Check intended scope: "FPS HUD during gameplay" is met structurally; "FPS everywhere" is not.
- Compare displayed FPS against a real-time external frame counter under vsync/capped and uncapped conditions. The source proves loop-completion timing, not actual scanout/presentation timing. An interval that includes a long pause or hidden window can yield a transient low value. No GPU or runtime verification was performed here.

## Scope / state

Read-only inspection only. No tracked source edited, build/game/GPU run, or captures. Concurrent dirty `port/linux/main.cpp` and `port/game/Combat.cpp` were left untouched. Findings reflect the files as read and may need recheck after concurrent edits settle.

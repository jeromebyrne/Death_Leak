# Complete Linux port checkpoint

- Goal and latest steering: deliver a playable, performant Linux port of Death Leak, retaining a swappable renderer seam for a later macOS version.
- Acceptance criteria: native Linux launch; original menus/level loading and gameplay; keyboard/gamepad input; audio; saves; visual effects; automated build; Linux playtest and measured performance; commit and push tested branch.
- Execution owner: current agent, direct workspace work; no unattended worker has been launched.
- Write scope / branch: `feature/linux-renderer-foundation`; clean baseline at start `5207dd9`.
- Last verified milestone (2026-10-19): native Linux build parsed `forest_exploration_1.xml` and rendered 72 original sprites to a nonblank 1280×720 headless capture; 10 assets were empty, missing, or unsupported. Renderer demo smoke test also passed.
- Pending criteria / next action: correct vertical-flip and repeat/parallax fidelity; integrate animation; port original game state/update/collision/player/input; then UI/audio/saves/effects, test an end-to-end level and profile frame time.
- Blocker: legacy gameplay, update and rendering rely on Win32, DXUT, D3D10 and Steam in 114 files; no executable is checked into the repository. No external approval needed. This is implementation scope, not a solved port.
- Implemented: original XML static scene loader, asset normalization, texture reuse and depth ordering, reproducible Docker build definition.
- Tested: Linux compile, existing smoke capture, forest preview capture. Neither is gameplay testing.
- Deployed: not applicable. Committed/pushed: level-preview milestone `b621359` pushed and remote SHA verified; complete port pending.
- Evidence: `port/game/LevelScene.cpp`, `port/build/forest-preview.bmp` (local/ignored), `port/README.md`.
- Notification destination: originating Discord direct conversation with Jerome. Current turn replies directly; no background notification mechanism is configured.

## Acceptance ledger

- Linux native executable: renderer and static level preview only; game integration pending.
- Original gameplay and levels: XML scene display only; simulation, interactions and transitions pending.
- Input, audio, saves, UI, effects: pending (preview camera pan is not gameplay input).
- Performance/playtesting: pending; no playable build exists.
- Tested commit pushed to GitHub: preview milestone verified at `b621359`; playable port pending.

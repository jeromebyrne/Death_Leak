# Frame-424 pre-HUD packet scoped QA — 2026-10-23 23:39Z

Reviewed `8272e97` (`port/linux/main.cpp` and its packet report) without GPU/Xorg or shared-file edits. The committed report records a fresh isolated build and physical-GPU first-postfade frame-424 run: 1280×720 pre-HUD and final BMPs, 37 ordered scene sprite submissions with resolved paths, valid hashes and ordered ordinals. This QA does not independently rerun the GPU capture and does not establish Windows pixel parity.

Source review found one schema regression: the frame-415 path gained `preHudImage:null` and `sceneDrawIdentifierScope`, despite its v1 schema being intended unchanged. Scoped correction makes both fields postfade-only. The postfade pre-HUD capture remains after scene postprocess and before UI; ordered draws are decorator-level sprite calls only, not XML IDs or backend subdraws. `g++ -std=c++17 -Wall -Wextra -Wpedantic -fsyntax-only -Iport/compat port/linux/main.cpp` passed. `git diff --check -- port/linux/main.cpp` passed. No concurrent build process was present; no full build or GPU run was performed. Previous packet GPU evidence belongs to `8272e97`.

Only the main.cpp correction is intended for commit/push. Existing dirty files and shared checkpoints are outside this QA.

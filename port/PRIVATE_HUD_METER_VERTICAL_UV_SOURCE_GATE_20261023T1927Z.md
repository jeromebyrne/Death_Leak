# Private HUD meter vertical UV source gate — 2026-10-23

## Result

**Source discrepancy fixed; CPU and clean-build gates PASS.** Original `Game/XmlFiles/UI/game_hud.xml` sets `91_xp_meter` to `vflip="true"`. `UISprite::XmlRead` retains that flag, `UISprite::Reset` reverses vertical UVs, and `UIMeter::Draw` changes only horizontal flip while drawing fill, left edge, right edge and middle overlay. Prior Linux HUD state flipped the focus fill and right cap but not the left cap or middle overlay. `port/game/Hud.cpp` now supplies `flipY=true` on those two commands only, preserving their geometry, tint, texture and horizontal flip. The health meter remains unflipped.

The source art explains the live-pixel outcome: `meter_edge_2.png` (4×19) and `meter_overlay.png` (1×19) have exact top/bottom row symmetry across all rows. Consequently a vertical UV flip of these two assets is a source-contract correction but produces no image change with the current assets. `xp_meter_bar_middle.png` is asymmetric; its separately committed focus-fill flip remains the visible vertical-UV correction.

## Verification

- Focused renderer-spy `port/tests/hud_meter_vertical_uv_source_gate_20261023.cpp` compiles and runs PASS. It drives actual `Hud::Draw` through positive and transitioning focus states, checks the current HEAD geometry/tint and asserts all four focus pieces flip Y while the five other HUD commands do not.
- Clean source snapshot from HEAD `ce8e7ed` (`git archive` of `port` and `Game/tinyxml`, with only patched `Hud.cpp` overlaid) built successfully in `deathleak-linux-build:bookworm`. Controlled UV-off binary also built successfully. No dirty `port/linux/SDLRenderer.cpp` or other worktree edits entered either snapshot.
- Physical Xorg `:0`/nouveau NVE7, SDL X11/OpenGL accelerated strict mode, active Forest-1 `--play`, one fixed frame, focus=100: ON/OFF/ON-repeat all exited zero and reported identical gameplay endpoints. Three 1280×720 BMPs are byte-identical (SHA-256 `11f4b859acf521e1959aa637d5031c952d9b666b23d7a4a514eabd805de3262d`), matching the exact asset symmetry; no positive pixel delta is claimed. Logs, snapshots and BMPs are in ignored `port/build/private-hud-meter-vertical-uv-source-gate-20261023/`.
- `git diff --cached --check` passed. Scoped commit `95eebfb` (`Hud.cpp` and focused test only) was pushed to `origin/feature/linux-renderer-foundation`. Unrelated dirty files, including `port/linux/SDLRenderer.cpp`, were neither staged nor changed.

## Ownership release / next bounded action

`port/game/Hud.cpp`, Docker/build and physical GPU/Xorg/capture lanes are released. No original-Windows same-state focus-meter screenshot or acceptance claim exists. A future visual gate should compare the **asymmetric focus fill** in a matched positive-focus Windows/Linux HUD crop, or take the separate forest source audit's next unowned visual target; repeating an A/B on the symmetric cap/overlay art cannot yield a pixel discriminator.

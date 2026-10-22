# Player ID2 finite firing-arm parity — 2026-10-22

## Scope and source

The forest-1 Player ID2 now loads the XML `<arm>` Still and Running sequences (ten authored frames each, 30 fps) separately from the body. Its arm draws in the same world transform, tint, flip and depth slot immediately **before** the body, as in `Character::Draw`. The arm begins at frame zero when the source initializes Still, advances finitely, and disappears when its cursor reaches the sequence length. A successful projectile creation (`Combat::Fired()` increment) restarts it; held fire with no new projectile does not. The fire step and animation update both advance it, matching `Player::Fire` plus `Character::UpdateAnimations`. A body-sequence transition changes/finishes the arm, so unsupported body sequences do not leave a stuck overlay. No XML or art was changed.

Source anchors: `Game/src/GameObjects/Player.cpp:467-475`, `Game/src/GameObjects/Character.cpp:524-526,1099-1112,1888-1907`, `Game/src/Animation/AnimationPart.h:59-65`, `Game/src/Animation/AnimationPart.cpp:201-232`, and `Game/XmlFiles/animation/player_anim_default.xml:1483-1510`.

## Verification

- Docker `make -C port -j2`: pass (only pre-existing TinyXML warnings).
- `port/tests/player_arm_parity_test.cpp`: pass. Checks source-initialized finite Still, non-fire expiry, successful-fire restart/frame 001, arm-before-body draw order and shared transform, horizontal facing, Running transition suppression/restart, and finite Running expiry. Build/test log: `port/build/player-arm-parity-2026-10-22/build-test.log`.
- Existing accelerated `port/tests/player_visual_test.py`: pass in the physical-GPU container; idle crop changes 5,995 pixels and melee 7,427.
- Physical Xorg `:0` / nouveau NVE7 / accelerated OpenGL: matched deterministic forest-1 frame 121 captured for `. :121` (without the space in the actual CLI) and `.:120,F:1`, fixed dt 1/60. Both finish at identical player coordinates `(-43.0146,3206.9)`; fire log says `fired=1`, no-fire says `fired=0`. With only XML ID2 rendered, the entire 1280×720 difference is confined to x=632–687, y=436–480: **762 changed pixels**. This isolates the arm from projectile/environment sprites. `fire-frame121.bmp` SHA-256 `3b76ef520e4dd74bdb111b03d20f1e6d8ebb06e31d4e60772830e8e3140da42b`; `no-fire-frame121.bmp` SHA-256 `99f55050d415238ba4cc25d8afafc78a6c2a23da7d0d26a1bc33c8c2f010dc6c`. Crops are `fire-player-crop.png` / `no-fire-player-crop.png` in the ignored evidence directory.
- Active forest route `B:300`, fixed dt 1/60, physical NVE7: completed with `fired=18`, player `(2328.99,3114.51)`, five orbs, health 25, 71 original sprites / zero missing; `active-right-fire-frame300.bmp` SHA-256 `43b4bb8005b002e05685628919cef5ad8c6f56b4c4e1be788e4e1436203a7bcf`.

All BMP/log evidence lives in ignored `port/build/player-arm-parity-2026-10-22/`. The deterministic paired crop proves the port composition changed as intended, not exact Windows pixel identity; a matched original Windows camera/state/time frame remains unavailable. The arm is local to the player and does not address the screen-wide foreground/shaft difference.

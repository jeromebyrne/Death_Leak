# Remote-desktop playtest repair — 2026-10-29

Status: implemented; automated verification collected in root checkpoint; attended physical/local+remote acceptance pending coordination. No new user-display game has been launched.

## Close-first evidence

Original jerome PID2073815 was closed via `_NET_CLOSE_WINDOW` on exact window0x3c0000a using the existing documented Xorg authority path in `tools/run_physical_xorg_route.sh`. Subsequent process query returned no PID. No authentication changes, save deletion or unrelated worktree edits.

## Defects and corrections

- Original UI PNGs store inverse V relative to SDL copies. Apply inverse authored V to background/blur/title/frame/film/buttons/cursor; use exact original mainmenu.xml background rectangle. Draw original `Game/Media/UI/cursor.png` (28x42 authored units) into frames, including remote video capture.
- Visible PLAY/OPTIONS/QUIT labels were outside the icon-only click targets; language page had no pointer hit-testing. Include whole visible rows and all language/back targets.
- Remote keydown/up may arrive in the same fixed-update pump, leaving SDL_GetKeyboardState released. Retain event press edges and held state, support missing scancodes via keysyms, clear on focus loss. Retain rapid mouse press/release for one update too. Native controller mapping/lifecycle retained. This is a reproduced synthetic-delivery failure; actual remote transport still requires attended verification.
- Ctrl+Q and Alt+F4 terminate through normal save-on-exit handling, independently of menu/modal/controller state. SDL_QUIT and WINDOWEVENT_CLOSE honored. Escape remains supported.
- Four custom OpenGL paths used window-style top-down projection while drawing into SDL's bottom-up target. This mirrored effect positions around logical y360 and disagreed with ordinary SDL art. Target-aware projection corrects wobble/damage/parallax/rotated repeat, including letterboxing. Dynamic enemies/debris/blood also now use Sprite's inverse authored V; effect radians convert through the established rotation helper. HUD meters/film/frame/death/loading UI use inverse authored V selectively; orb/key icons with authored vflip=true remain unflipped.
- Left knives combined an X launch reflection with an already-leftward +/-acos angle, reversing their tip. Local pitch plus one launch reflection points the actual texture tip along current flight, including gravity/diagonals; inverse V handled separately.
- Pixel-wobble fallback previously sampled top-first SDL textures with the private reversed-row texture's unconditional 1-V. Shader now normalizes by actual storage; forced fallback regression provided.

## Original evidence (not replacement art)

Video: https://www.youtube.com/watch?v=03EPlXOJLtg . Retained 640x360 original, 37 timestamped samples/contact sheets, provenance+SHA256 in `/srv/openclaw/workspace/death_leak_analysis/playtest_repair_20261029/reference/`. See `REFERENCE_FINDINGS.md` and `MANIFEST.json`. 54s shows left-facing player;40:00 shows blade-forward knives. Sampled opening did not show menu; menu conclusions come from original UISprite.cpp, UIManager.cpp, mainmenu.xml and PNGs. No art approximations/substitutions.

## Verification and acceptance

- `make -C port input-backend-test`: virtual controller lifecycle; WASD/arrows held/released/coalesced remote events/missing scancode; focus loss; rapid mouse edge; Ctrl+Q/Alt+F4/window-close. Non-display regression only.
- `port/tests/knife_visual_lifecycle_test.cpp`: fade/lifetime/contact plus transformed texture-tip dot flight-direction >0.9999. CPU only.
- `make -C port build/render_target_orientation_test`; run binary with WIDTH HEIGHT PREFIX. Strict hardware OpenGL required; checks four colored corners, absolute placement, no mirrored ghost, all six draw families. Run at1280x720 and1440x900; force alternate wobble texture with `DEATHLEAK_TEST_WOBBLE_SDL_TEXTURE=1`. Offscreen GPU is diagnostic, not physical acceptance.
- Full existing `port/tests/integration.sh`: simulation/software safety gates, routes, progression, settings and mouse flows. Not human playability.
- Remaining attended acceptance: actual deployed Play-DeathLeak.sh flags/native host libraries, visible original cursor, upright scene/menu, full mouse and keyboard options/language/back/Play/Quit, local hardware AND user's remote WASD/arrows, forward knives left/right/diagonals, Escape/Ctrl+Q/Alt+F4/window-close and save preservation, physical accelerated renderer identity. No software rendering accepted as this gate.

Evidence/logs: ignored `port/build/playtest-repair-20261029/`. Publication/deployment receipts and final acceptance go in root `CHECKPOINT_STATUS.md`.

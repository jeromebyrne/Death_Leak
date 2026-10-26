# Fixed-update HUD repeat-presentation packet — 2026-10-26

Base: clean detached `235dc43d99609531d4d25163e1212581ecd20d35` (`origin/feature/linux-renderer-foundation`). Primary dirty tracked/untracked paths were not modified, except append-only `PERFORMANCE_CHECKPOINT_STATUS.md`.

## Scope and contract

`HudPresentationPacket` owns a `const Hud` snapshot and `const orbBalance`, copied after each fixed simulation update. Its const `Draw` submits the prepared HUD at the former HUD painter slot, after scene postprocess/readback and film/bars, before modals/loading/debug and the post-overlay capture. The original `Hud::Prepare` mutation moved from that slot to after fixed visual updates and before the present/skip branch; no game logic or clock advance occurs during packet `Draw`. The fixed 60-Hz simulation pacer, shader clock, scene/combat/weather painter, blend/shader state, film selection, pre-HUD and post-overlay capture stages remain in their existing order. Catch-up updates can now advance HUD preparation even when an intermediate draw is skipped, as required for future update-free presentation; this slice does **not** create a repeat-display loop or make the full scene renderer read-only.

## Gates

- Focused fake-renderer C++ fixture passed 100,000 same-packet repeat submissions with identical sprite/rect command streams. A subsequent `Hud::Prepare` changed the health backing, while the old packet remained unchanged. `git diff --check` passed.
- Clean-base and candidate isolated Docker native production builds passed; final const-field candidate rebuilt after temporary positive-control source restoration.
- Strict serial physical X11/OpenGL Mesa nouveau NVE7 `accelerated=1 software=0 vsync=1`, 1280×720 Forest-1 fixed-dt 100-update A(base)/B(packet)/A(base) full BMPs byte-identical SHA-256 `519d7e641db9b03b6e48b78daaf628e997285ebb3b8feb9ff93717ba3e353ef4`; final const-field candidate repeated that hash. All endpoints matched. Native film timing happened to align; no synchronized test clock was installed. This is one 60-Hz visual route, not broad gameplay or performance acceptance.
- Test-only physical omission of the packet HUD changed 10,373 BMP payload bytes / 3,473 RGB pixels on the same frame, confirming the branch was visible. Temporary source was restored and production binary rebuilt; no omission is committed.

## Display-mode audit

Current physical SDL/X11 enumeration includes 1280×720@60 and 1440×900@60, plus **1440×810@120** and several smaller 120-Hz modes. A strict accelerated NVE7 two-frame fullscreen `--window-size 1440x810 --refresh-hz 120` probe selected active 1440×810@120 and produced a 1440×810 BMP. This corrects the narrower prior observation that 1440×900@120 is unavailable. It proves mode selection/output only: the live loop still couples accepted fixed updates to renders, so 120-Hz repeat presentation, pacing, visual parity, input latency and sustained FPS are **not** proven. No 8K display mode was advertised or tested; 8K output is not claimed.

Evidence: ignored isolated `port/build/repeat-gate/`, full BMPs in both isolated worktrees. Next separate slice: make the remaining scene/combat/weather/menu draw inputs read-only or record a complete ordered command packet, then introduce a refresh-slot present path without mutating simulation and measure serial physical 60/120-Hz parity and pacing. Do not infer that the HUD packet alone enables full-frame repeat.

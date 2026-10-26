# Opt-in refresh-slot presentation gate — 2026-10-26

Base: pushed `774ec1bc2628d37f0d0284fceb51028f844adcb7`, clean isolated `/tmp/deathleak-refresh-display-20261026`. Primary `a5e39ac` and its unrelated dirty files were not used for the build or changed. Physical display ownership was serial; a residual earlier owner's Docker A/B/A process was observed at the start and allowed to exit before any run here.

## Accepted narrow software slice

`--refresh-slots` is an opt-in **live fullscreen** service requiring an explicit advertised `--refresh-hz`. It uses the sealed ordered renderer packet on a display slot only while the authoritative 60-Hz fixed-step pacer has no update due. The slot clock skips missed slots; it never requests simulation catch-up, resource loading, mutable scene draws, SaveFrame, or a second EndFrame recording pass. Packet replay targets the underlying renderer and `PresentReplay` performs only the required physical present. The ordinary painter/postprocess/film/HUD/capture pipeline, existing one-update/one-draw default, and logical projection/input remain unchanged.

The SDL backend fails closed if fullscreen, shown/input focus, window size, renderer output size, or active XRandR display mode/refresh differs from the requested surface. It checks before and after both ordinary and repeat presents; a changed surface returns a nonzero result rather than silently accepting a 1440×778 capture as 1440×810. This is a safety gate, **not** a fix for the intermittent external focus/mode initiator.

A focused C++17 fixture tested independent 60-Hz fixed updates against 60/75/120/144/240-Hz slots over 10 simulated seconds (601 fixed updates), ordinary versus update-free slots, and missed-slot skip without a burst. The existing 100,000-repeat ordered-packet fixture passed. Isolated Docker production build and `git diff --check` passed. Final native-film binary SHA-256: `2b61ba8cf7875e7aa51f1796e6c6bb957ffbe6a6ada4f3cce9c09547bb9c4cd2`.

## Strict serial physical NVE7 menu gate

Single native-clock binary A(off)/B(on)/A(off), 100 fixed 60-Hz updates in live fullscreen menu on X11/OpenGL Mesa nouveau NVE7, accelerated=1, software=0, vsync=1. Main-menu film time is advanced by the same fixed update delta in each arm, so this route has synchronized film without a source shim. Full BMPs were **byte-exact** within each mode:

- 1440×810@120: SHA-256 `a638460e9e1e9245150d0836b011c553402beb413e0b06b3288ace0d508c2e02` in A1/B/A2. Candidate produced **two** update-free physical presents, no surface failures; A1/B/A2 interval p50 16.6727/16.683/16.6652 ms, p95 17.406/17.0043/17.1208 ms, p99 65.5036/17.6389/67.2066 ms. The two repeat slots are the positive branch; 100 update iterations were unchanged.
- 1280×720@60: SHA-256 `68c56224b08ba564cd64afbcf92f55ba59ea3b334f9d4b3ba54b6e0cc716d77c` in A1/B/A2. Candidate produced **zero** update-free presents, no surface failures; A1/B/A2 p50 16.6762/16.6764/16.6565 ms, p95 17.1699/17.4251/17.3083 ms, p99 47.5413/54.4991/52.3261 ms.

These are full-run presentation intervals with startup/transition tails, not steady-state GPU timings. Two extra 120-Hz presentations out of 100 updates do **not** establish 120 FPS or a sustained gain. The 60-Hz arm only checks that the optional service preserves ordinary output when no spare slot is available.

## Display attribution and rejected broader gates

Read-only `xprop -spy -root _NET_ACTIVE_WINDOW` during the dedicated serial menu run recorded only alternation between the desktop window `0x3200049` (Nemo desktop PID 4288) and newly created game windows on launch/exit. The XRandR read-only query after the runs returned the pre-run 1440×810 desktop mode. No 1440×778/focus-loss drift recurred in this session, so the **external initiator remains unidentified**; X11 active-window notifications alone do not identify the client that requested a prior mode switch. Original drift evidence remains `port/REFRESH_OUTPUT_DRIFT_DISCRIMINATOR_20261026.md`.

A temporary synchronized-film Forest 100-update 1440×810@120 diagnostic was **rejected as a pixel gate**: A1 reached a 1440×810 capture, but candidate's final update was an intentional no-present catch-up update, so the existing `--capture` hook emitted no B BMP. A1 saw only 61 presentation intervals over 100 updates (p50 23.1291 ms); B saw 55 (p50 23.0746 ms) and **zero** update-free presents. The source-only film shim was restored; the final binary above is native-clock. This route demonstrates the GPU-bound Forest path has no spare 120-Hz repeat slots; it cannot establish extended visual parity, high FPS or a performance gain. Do not treat the missing B capture as a match.

Physical logs/BMPs and focus trace: `/tmp/deathleak-refresh-display-evidence/`; build logs: `/tmp/deathleak-refresh-display-final-build2.log`, `/tmp/deathleak-refresh-slot-fixture.log`. Remaining blockers: attribute the intermittent focus/XRandR initiator with X-server request tracing or controlled client isolation; provide stable long Forest/Waterfall/combat/transition synchronized-film A/B/A with capture on a known presented ordinal, resource/thermal/VRAM checks, and performance headroom. Atlas coverage, shader-pass work and 8K output remain separate open goals. **No 120-FPS, 8K or release acceptance claim.**

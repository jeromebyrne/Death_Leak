# Linux Waterfall frame-415 atomic image/state packet — 2026-10-23 23:22Z

## Result

**Implemented and physical-GPU validated, Linux-only.** `port/linux/main.cpp` now accepts `--atomic-frame415-packet <json>` only with a distinct `--capture <bmp>`, `--frames 416`, `--fixed-dt`, `--script`, and `--play`. It snapshots update/draw state during ordinal 415, saves the final post-overlay/pre-EndFrame BMP, verifies first-reveal Waterfall gameplay and BMP 1280×720×32 header, hashes the saved image, current executable, and active XML, then atomically renames a temporary JSON as the last packet operation. A failed precondition does not publish JSON. The optional path changes no normal capture/UI output; no software fallback is enabled.

The sidecar explicitly sets `matchedStateEligible=false`. It records `unavailable` with reasons for actual renderer matrices, per-object ordered draw submissions/texture hashes, body/arm clip-frame, displayed HUD meter fill, content manifest, and nondeterministic effect history. Thus this is an atomic **image plus partial same-ordinal state** packet, not a complete matched-state parity packet. Do not infer original-D3D10 parity from it. A source-built original D3D10 frame/state packet is unavailable here.

## Reproduction / evidence

Ignored evidence directory: `port/build/private-watchdog-linux-frame415-atomic-20261023t2322z/` (`capture.sh`, final `capture.log`, `build.log`, `frame415.bmp`, `frame415.json`, review PNG, unique executable). Build: `deathleak-linux-build:bookworm`, `make -C port TARGET=build/private-watchdog-linux-frame415-atomic-20261023t2322z/deathleak-linux`, passed; only existing TinyXML warnings. Final executable SHA-256 `6cd7c6bfc81d645ba13b927e31651a1466092a28ed0bd2632e82f8b27882b25e`.

Physical `:0` Xorg with LightDM authority, `/dev/dri`, SDL x11/OpenGL strict accelerated: `accelerated=1 software=0 vsync=1`, `vendor=nouveau renderer=NVE7`, GPU Sepia before HUD. Command in `capture.sh`: Forest-1 `--script B:1800 --frames 416 --fixed-dt 0.016666667 --capture /out/frame415.bmp --atomic-frame415-packet /out/frame415.json`. Exit 0. Trigger at frame 398, Waterfall installed; ordinal 415 BMP is 3,686,522 bytes, 1280×720, 32-bit, SHA-256 `42c5ad22188a99843ed7f18010b706eb4b01232a81d78600bdb6cb40783fec49`. Independent Python decode/hash validation passed and matches JSON.

Same-ordinal JSON: phase `FadeOut`/3 at update entry and draw; loading time `0.0666666701`, raw alpha `0.666666627`, submitted tint alpha `170`; gameplay, weather, scene visuals, and player visuals all updated; pause false. Effective dt `0.0166666675`, shader total `6.93336916`, pixel wobble `6.91670227`; drawn camera `(-146.067642,3000)`, player `(-2190.01465,3130.31812)`, 7 orbs, health 25, focus 100. Prior saved corrected Linux `frame415.bmp` SHA-256 `d87e39bc...` is a **different run**; do not mix it with this sidecar. Final pixels may differ across runs due wall-clock film/effect state.

## Ownership / disposition

Edited only previously clean `port/linux/main.cpp` plus this unique report. Preexisting dirty `port/linux/SDLRenderer.cpp`, `port/tests/integration.sh`, other dirty files, shared `CHECKPOINT_STATUS.md`/`PORT_WORK_STATUS.md`, and their owners were untouched. The dirty renderer was incorporated by this unique build. Temporary Xauthorization copy was removed after capture; no game/FFmpeg/capture container remains. Exclusive physical GPU/build lane **released**. Next matched parity step: instrument actual matrices, ordered visible scene draws and HUD displayed state, then acquire one original D3D10 same-state packet; this Linux packet alone is not a matched-parity verdict.

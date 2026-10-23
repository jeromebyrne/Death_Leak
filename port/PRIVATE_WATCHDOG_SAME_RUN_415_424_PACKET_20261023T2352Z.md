# Same-run frame-415 / postfade frame-424 Waterfall packet gate — 2026-10-23 23:52Z

## Verdict

**PASS for the bounded Linux packet gate.** The new opt-in `--atomic-same-run-waterfall-packets <415.bmp> <415.json> <424.bmp> <424.json>` captures both atomic v1 packets within one process using one frozen binary and one Forest-1 `B:1800` / fixed-dt input history. Existing single-packet flags retain their separate behavior. This does **not** establish Windows matched-state/pixel parity or Jerome acceptance; both packets say `matchedStateEligible=false`.

## Reproduction

- Source scope: `port/linux/main.cpp` only. New option requires `--frames 425 --fixed-dt --script --play` and four distinct paths. Frame 415 is captured after overlay before EndFrame; frame 424 retains its existing pre-HUD scene readback, ordered scene-draw trace, and final overlay capture. Each JSON commits only after its BMP and hashes exist.
- Docker image `deathleak-linux-build:bookworm`; `make -C port TARGET=build/private-watchdog-same-run-415-424-20261023t2352z/deathleak-linux -j2` exited 0. Build log and ignored outputs: `port/build/private-watchdog-same-run-415-424-20261023t2352z/`. Binary SHA-256: `216e2d53024c87f304b43337e8c9b0811a0818144decd9b25b38356313459a32`.
- CPU functional run: Docker dummy video with `DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1`; `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:1800 --fixed-dt 0.016666667 --frames 425 --atomic-same-run-waterfall-packets /out/cpu415.bmp /out/cpu415.json /out/cpu424.bmp /out/cpu424.json`. Exit 0. This is test-only software rendering, not visual fidelity evidence.
- Physical-GPU run: same flags/input and same binary, substituting `/out/gpu415.bmp /out/gpu415.json /out/gpu424.bmp /out/gpu424.json`; Docker host network/hostname, physical Xorg `:0` socket, root LightDM Xauthority and `/dev/dri`. Exit 0; log reports `video_driver=x11 name=opengl`, `accelerated=1 software=0 mode=strict-accelerated`, `vendor=nouveau renderer=NVE7`, and Sepia GPU postprocess. Both runs trigger Forest-1 exit at frame 398 and load Waterfall. Audio was unavailable in the containers but did not block capture.

## Packet checks

- GPU frame 415: `deathleak-linux-frame415-v1`, render/update ordinal 415, FadeOut draw phase 3, loading alpha `0.666666627`; final BMP SHA-256 `a4c2956ab4e1739bf2679d47bb455696bb4ce58cfb6b68b2fb8578e8e19320e2`.
- GPU frame 424: `deathleak-linux-postfade-waterfall-v1`, render/update ordinal 424, `firstPostfadeOrdinal=424`, `loadingSplashSubmitted=false`, no loading phase at draw; final BMP SHA-256 `fd4eabffa51a7e86af5a707bb5eb8615056ef7cccf47d23ac5575e4af2f65ad8`; pre-HUD BMP hash matches JSON; 34 ordered scene draws.
- CPU frame 415/424 final BMP SHA-256 respectively `442c75678e614db217571b773f9c3d7adb48c9e5f1ab8326bfee6772dd6ceec6` / `399adc71a79cec2f6264cc5b250ad4d12633bde6c49bb4e69dfcb718b5221f1a`. Both image and pre-HUD hashes match their JSON.
- Python checked all four JSON files parse, both ordinals and packet schemas, all BMP hashes, executable identity matching the built binary, and `matchedStateEligible=false`. The new GPU frame-415 JSON has **the same recursive key sets** as the prior separate-run frame-415 v1 packet; it has no `preHudImage` or postfade-only transition keys. Its final image hash also matches the prior separate-run frame-415 capture. `git diff --check` passes.

## Boundary

This is same-run Linux state/history evidence only. There is no instrumented original Windows packet, content manifest, or matched Windows render matrix. Physical NVE7 GPU lane released after capture.

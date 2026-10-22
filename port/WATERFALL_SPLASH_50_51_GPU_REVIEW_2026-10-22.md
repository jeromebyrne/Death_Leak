# Waterfall-top ParticleSpray IDs 50/51 — physical-GPU review (2026-10-22)

## Outcome

The bounded Linux admission of authored waterfall-top emitters 50 and 51 is implemented and verified. This establishes their presence and measurable contribution at one fixed waterfall gameplay state, **not** matched-Windows pixel parity or Jerome's visual acceptance.

## Source-backed change

At starting pushed HEAD `8b4abc7`, `LevelScene` admitted forest ID47 and waterfall ID49 only. It now additionally admits only waterfall-top IDs50/51, retaining actual XML IDs, order and `kMiddleForeground` depth (1000) beside ID49 at `kGroundFront` (1750). Each pair member uses its authored origin/direction, ten `waterfall_splash.png` particles, negative max speed `-32`, gravity 22, lifetime growth to 4×, fade and loop properties through the existing `ParticleSpray` path. The whole-emitter 300×300 dimensions remain update bounds, not a rendered quad. Distinct private stable capture seeds avoid perfectly correlated pair patterns; they do not reproduce Windows' global `rand()` history. Forest ID47's stream and ID49's seed are unchanged. The previous ID49 test now isolates ID49 by the opt-in diagnostic filter because 50/51 are correctly admitted.

The original-source audit `WATERFALL_SPLASH_50_51_SOURCE_AUDIT_2026-10-22.md` documents original XML/source semantics and outstanding scale/RNG/culling discrepancies. No art or XML changed. The port still uses its existing world-size transform rather than claiming exact Windows `GameScale` behavior; it retains approximate broad-zone update culling and per-particle viewport draw culling rather than the original whole-emitter camera gate.

## Verification

- Focused Docker test `port/tests/waterfall_splash_50_51_test.cpp` passed: one shared splash texture load; exactly 20 texture quads at fixed upper-falls camera after a 1/60 update, split ten left/ten right, no giant bounds quad, no distant-camera draw. Its independent state check passed pre-advance draw coordinates, `-32→-31.68` drag, gravity/ID51 X drift, size growth and loop reset. Existing ID49/forest regression test passed after isolating ID49. Build log: ignored `port/build/waterfall-splash-50-51-build.log`.
- Production Docker `make -C port -j2` passed in `deathleak-linux-build:bookworm`; only pre-existing TinyXML warnings. `git diff --check` passed.
- Physical Xorg `:0` fixed `--play waterfall_top.xml --script '.:120' --frames 120 --fixed-dt 0.016666667` captures passed sequentially: full, hide50/51, only50, only51. Exact invocation, logs and lossless BMPs: ignored `port/build/waterfall-splash-50-51-gpu-2026-10-22/`. Each log reports SDL X11/OpenGL strict accelerated (`software=0`), direct nouveau/NVE7, GPU Sepia, 81 original sprites, zero missing assets, and the same terminal player `(818.985,2933.64)`, health 25, orbs 0. Full minus hide50/51 changes 63,941 RGB pixels, bounding box x22–632/y329–593. The hide capture's SHA-256 `fb0e2edfe35eee9aee6dfc8d54d66dc4cb1295ff99071e7a24a7ca1764010d42` exactly matches the previous ID49 full capture at the same state, supporting no unintended baseline change. New full BMP SHA-256 `86b44a179732223e9fc3ee9d32b7c21c7c642c9a9a1d16514362996567ac197f`.

## Limits and handoff

This is a single Linux GPU same-state A/B with scripted idle input and dummy audio. It proves rendered contribution, not human playability, audio quality, animation over time, all camera-edge cases, exact original RNG/timeline/backbuffer equivalence, or painter parity at every overlap. There is no matched Windows waterfall-top frame; do not label it Windows-parity or Jerome-approved. Next owner can compare the captured full frame against a matched original Windows state if available, including backbuffer/game-scale and particle RNG history. Shared `CHECKPOINT_STATUS.md` and `PORT_WORK_STATUS.md` were not edited; unrelated working-tree changes were preserved.

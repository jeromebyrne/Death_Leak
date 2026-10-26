# Loading-overlay presentation packet seam — 2026-10-26

Base: clean detached remote `feature/linux-renderer-foundation` at `d9786ff58d4a948d052a6526e76467467dc34826`. Sole source/build/physical-NVE7 owner session `agent:main:subagent:9c8f7213-bbbb-416d-a94e-d99466db62c8`. Primary checkout's seven unrelated tracked dirty files and untracked artifacts were preserved.

## Decision and boundary

Accept a bounded fixed-update `LoadingOverlayPacket`. Immediately before the catch-up/presentation fork, it snapshots the current loading visibility, original fade alpha and quantized tint. Presentation submits the immutable packet, and atomic capture JSON reads the **same** alpha/tint snapshot. This removes draw-time loading-state computation and draw-only mutation of capture metadata. It preserves the fade-in, hold-exit, fade-out and opaque cases, including the `-1` no-overlay sentinel. No capture readback or JSON writing was moved: `SaveFrame` must remain at its exact pre-HUD/post-overlay stage, and atomic JSON still commits after the image/hash exist.

The ordinary gameplay draw still depends on mutable scene/combat/weather objects and lazily calls `loadFilmFrames()` in the gameplay HUD path. The renderer and scene traversal can have internal state. This seam does **not** make the whole draw update-free, and does not wire extra presentation slots. A future packet must snapshot/bind the remaining gameplay inputs and eliminate draw-time lazy film loading, while retaining capture stage semantics.

## Verification

- C++17 fake-renderer test covers absent, fade-in, hold-exit, fade-out, sprite and fill fallback, and 100,000 repeated const submissions of a prepared packet without mutation. `git diff --check` and final native-clock Docker production build passed.
- Strict serial physical X11/OpenGL Mesa nouveau NVE7, `accelerated=1 software=0 vsync=1`, 1280×720 fixed-60 Forest→Waterfall route (`--script B:1800 --fixed-dt 0.016666667 --frames 416`) captured the frame-415 loading fade via the atomic packet. Baseline/candidate/baseline post-overlay BMP SHA-256 was `3e88eb77d1a6edd79dc0093ec62695a3f92a29c6b5b5bdf494e80b6efdd95218` for all three, byte-identical, zero changed pixels. Atomic transition metadata matched at fade alpha `0.666666627`, tint `170`, gameplay/player updates true, and identical player/vitals/score endpoint.
- Both physical builds used the same **test-only** frame-indexed 11-FPS film clock to remove unrelated native wall-time phase variation. The substitution was removed before final production build and commit. Physical evidence/logs/JSON/BMPs are in ignored primary `port/build/packet-seam-20261026/`.

The evidence proves this specific loading-fade packet and synchronized 60-Hz pixels, not native arbitrary film-phase parity, actual >60-Hz output, 8K, sustained FPS, or release acceptance.

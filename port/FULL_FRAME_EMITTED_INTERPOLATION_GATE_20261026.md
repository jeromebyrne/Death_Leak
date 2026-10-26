# Full-frame emitted-command interpolation topology gate — 2026-10-26

**Decision: accept diagnostic whole-packet identity admission; reject visible interpolation B.** Exact pushed base `bbff8a3ccf6f738e259e613debb78010cbb963b3` in clean detached `/tmp/deathleak-fullframe-interpolation-20261026`. Primary unrelated dirty tracked/untracked work was not modified except append-only performance checkpoint.

## Actual implementation and observations

`LevelScene::Draw` now exposes opt-in post-submission observers for every authored scene sprite, player arm/body and authored/attached/orb-trail spray particle. Existing ordinary-sprite observer remains unchanged. Each actual `PresentationRecordingRenderer` sprite command can carry an observer-assigned scene-object/subpart key or spray-slot/generation key; default callers supply no observers, so the renderer's visible path is unchanged. Sealed packet copies retain the tag and command order. The new `FullFrameInterpolationGate.h` conservatively rejects missing/duplicate tags, painter kind/order changes, scene epoch/camera discontinuity, births/deaths/generation replacement, non-positional sprite state changes, and unmodeled sprite/fill batches. It checks camera-parallax displacement under stable factors, and allows a fully stable tagged synthetic packet. Hold-current is whole-frame, never actor-only motion.

The actual emitted Forest-1 `LevelScene::Draw` diagnostic (XML IDs 1,2,17,19,37,47) emitted six sprite commands and tagged all six. With camera X +10 world units, painter commands 1 and 6 changed normalized parallax UV X phase `0→0.00059` and `0→0.0069`. That full packet is **not** an ordinary position-only interpolation candidate, even before later game-level dynamic combat, HUD, UI, postprocess, shader time and fill/batch commands are included. The current production recorder also keeps only one packet and presents the ordinary current state; no previous/current paired full-frame runtime presenter exists. Consequently coherent same-alpha ordinary+repeat semantics are **not proven**, and no runtime interpolation, physical NVE7 A/B/A, exact-pixel or high-Hz benefit is claimed.

The test exercises immutable packet copies, identical-alpha repeated admission, born/dead/mismatched generation/order, cut/epoch and untagged policy, and 30/60/75/120/144/165/240-Hz synthetic clocks (601 fixed updates/10 s and zero drops). The synthetic complete packet passes while actual moving-camera Forest holds current. This is topology/clock evidence, not active-gameplay frame-rate evidence.

## Verification

Docker `deathleak-linux-build:bookworm`: focused full-frame, prior real-scene topology, prior CPU topology and presentation-command-packet tests passed. Isolated `make -B -j2` production build passed; SHA-256 `3482cafba70571caab79f637bfbcdfd7b520b8935b4fd690a1c9673e8a1e020d`. `git diff --check` passed. No physical session was started because there is no sound B candidate; no 60-FPS, 8K, sustained high-Hz, VRAM, thermal or release acceptance. Default atlas coverage remains 15/896.

## Distinct next implementable gate

Give parallax/scroll/shader commands explicit phase/time sampling semantics in a two-sealed-packet **complete game frame** recorder (including dynamic combat and UI ownership), with scene epoch and one-update-latency previous→current alpha on both ordinary and repeat presents. First prove the emitted-command oracle across real running, landing, aim toggle and scene transition routes, including alpha=1 equality to current-state baseline and positive spare display presents. If any untagged painter command or shader phase remains, hold current. Only then build an opt-in runtime B and attempt serial physical NVE7 exact-pixel A/B/A and timing at actual advertised modes.

# Parallax-phase and complete-game packet interpolation gate — 2026-10-26

**Decision: accept bounded phase-aware packet implementation; reject visible runtime B.** Clean isolated checkout from exact pushed `8127cf8827655458d1bbd3bddee5d955baa4d520`; primary dirty checkout untouched.

## Implemented vertical slice

- `PresentationRecordingRenderer` now retains the previous sealed immutable command packet when the next frame begins, while the current sealed packet remains replayable. Every command keeps painter order and shader/UV inputs. Tags can carry an explicit static, camera-parallax, auto-scroll-Y or shader-clock phase policy and camera UV coefficients.
- `AdmitFullFrame` pairs actual emitted commands, rejecting missing/duplicate identities, changing kinds/order/texture/source/tint/culling state, camera/scene cuts and unmodeled batch commands. It verifies the observed parallax UV delta against the actual raw-camera delta and authored sign/parallax factors. Shader phase sampling requires matched frame clock metadata; unstamped auto-scroll remains an intentional hold. `SampleFullFrame` samples position, UV and shader-time fields at one alpha only when the **whole** packet admits; alpha=1 copies current bytes/float fields exactly. No default presentation behavior changed.
- Opt-in `--full-frame-topology-check` wraps the actual game renderer and captures complete frame command streams, including scene, dynamic combat, postprocess, film and HUD; it tags emitted scene/particle commands and pairs consecutive sealed whole-game frames with scene epoch/camera/shader-clock metadata. This is diagnostic only, not an interpolation switch.

## Verification and blocker

Focused emitted Forest-1 fixture: six of six scene sprites tagged, moving camera parallax phase admitted, deliberately perturbed UV rejected. Synthetic repeat-X/shader-clock packet uses one alpha; wrong clock rejected; alpha=1 retains current packet. Birth/death, generation/order/cut/epoch and unowned HUD remain hold-current. 30/60/75/120/144/165/240-Hz synthetic clock fixture retained fixed 60-Hz simulation counts. Prior 100,000-repeat presentation packet regression passed.

An actual active Forest game invocation under dummy/software rendering with `--play ... --frames 3 --fixed-dt 0.0166666667 --full-frame-topology-check` produced 55-command whole-frame packets, 46/50 sprites tagged on frames 1 and 2, decision `HoldCurrent` on both. The four unowned sprites are outside `LevelScene` emission (game-level film/HUD or dynamic paths); this diagnostic does not establish their individual classification. Preview separately produced 48 commands and 0/46 tagged because that branch does not use the new scene observer. Thus **there is no sound complete-frame visible B**, even though the bounded emitted scene packet admits camera phase. No NVE7 A/B/A, timing, visual parity, high-refresh or FPS gain is claimed. The software fixture is not physical acceptance.

Next distinct code gate: give every game-level sprite/batch/fill command stable ownership and paired semantics (combat, weather, film, HUD, loading/menu), and add an explicit once-per-selected-update auto-scroll phase stamp. Run whole-game admission across active running, landing, aim toggle and transitions with alpha=1 exact current-packet replay and no unowned commands before enabling one-update-latency ordinary/repeat sampling. Only then run serial physical NVE7 advertised-mode exact-pixel A/B/A and timing. Existing fixed 60-Hz simulation must remain unchanged.

Final isolated Docker production build passed, SHA-256 `ac7a9a4956312b71a50292565b07186097b38a84e63db5d5015ad667aa268776`; `git diff --check` clean. No 8K/high-Hz/60-FPS, VRAM, thermal or release acceptance.

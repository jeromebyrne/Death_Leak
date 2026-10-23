# Running stored-rate order — scoped CPU integration

## Disposition

Integrated the Forest-1 ID2 Running rate-order correction into the Linux port. `LevelScene::UpdatePlayerVisual` advances Running with the sequence's stored `currentRate`, then stores the velocity-derived clamped rate for the next update. This mirrors the source call order without altering the pre-existing velocity-to-rate multiplier, other body modes, one-frame-per-update behavior, or re-entry rate persistence.

The unique live-XML/draw-command fixture `port/tests/running_rate_order_cpu_test_RUNNING_RATE_ORDER_20261023T2136Z.cpp` is now invoked at the top of `port/tests/integration.sh` beside the existing strafe CPU test. Its `tick` calls are inside `assert`, so the durable command explicitly compiles with `-UNDEBUG`. It checks authored initial 60 fps, stored-rate-first transition to low velocity, fractional/excess carry, one-step maximum, and Still→Running re-entry via ID2 texture paths.

## Verification

- Host scoped `g++ -O2 -g -std=c++17 -Wall -Wextra -Wpedantic -UNDEBUG -Iport/compat` compile/run of unique fixture and production `LevelScene.cpp`, `PlayerMotion.cpp`, four TinyXML TUs: **PASS**.
- Docker image `deathleak-linux-build:bookworm`: `make -C port` current dirty tree: **PASS**. Existing TinyXML parser warnings only.
- Same assertion-enabled unique fixture compile/run inside Docker: **PASS**.
- `sh -n port/tests/integration.sh` and scoped `git diff --check`: **PASS**.
- Full `integration.sh`, physical GPU capture, and Windows same-state pixel comparison were **not run**. CPU draw-path gate does not prove rendered pixel parity.

## Commit scope

Stage only the narrow `LevelScene.cpp` hunk, unique fixture, and the new integration invocation; preserve all pre-existing unrelated dirty `integration.sh` expectations and other dirty files. No renderer/GPU ownership or checkpoint/status file changes in this slice.

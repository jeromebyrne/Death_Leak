# Waterfall-top foliage phase execution — 2026-10-23

## Decision

**No production source edit or GPU capture was made.** The implementation preflight explicitly says the shared dirty `port/game/LevelScene.cpp` is not released: its right-stick aim-line changes remain unverified/uncommitted and the owner has not supplied an explicit handoff. The current diff still contains that adjacent draw hunk and loader/animation changes; `port/RIGHT_STICK_AIM_LINE_SOURCE_IMPLEMENTATION_2026-10-22.md` still says build/CPU/GPU/commit are pending. This satisfies the task's fallback condition, not the condition to integrate the narrow fix. I preserved the entire dirty file and all other shared edits. The coordinator's assignment of this waterfall task/GPU lane does not itself resolve the separate dirty-file ownership gate in the preflight.

## Executable CPU draw-spy discriminator

Ignored, non-source artifacts: `port/build/waterfall-foliage-phase-2026-10-23/draw_spy.cpp`, `run.sh`, `draw_spy`, and `cpu.log`. Reproduce from repository root:

```sh
docker run --rm -v "$PWD":/src -w /src deathleak-linux-build:bookworm sh port/build/waterfall-foliage-phase-2026-10-23/run.sh
```

The fixture loads the actual `Game/XmlFiles/levels/waterfall_top.xml` through a path-to-texture spy and invokes actual `LevelScene::Draw`, one process/XML-ID filter per ID because that filter is process-static. All 12 expected submissions occurred at the per-ID visible cameras. Every destination/foliage-enabled/time/intensity invariant passed. At fixed camera, time 0→0.5 moved the CPU sine output without changing either reported centre; camera changes did not move the current reported phase centre. The observations at time 0:

| ID | Camera | Drawn sway centre | Source-derived active parallax centre | Bottom X shift, drawn vs expected |
|---|---|---|---|---|
| 64 | (0,3000) | (-1179.01,3660) | (-1296.92,3693) | -15.14 vs +12.89 |
| 64 | (200,3200) | (-1179.01,3660) | (-1316.92,3683) | -15.14 vs +19.83 |
| 65 | (1000,3000) | (2059.99,3706) | (2165.98,3741.3) | -11.29 vs +16.70 |
| 65 | (1200,3200) | (2059.99,3706) | (2145.98,3731.3) | -11.29 vs -0.24 |
| 60 control | (-1500,3000) | (-2063.01,3571) | same | +6.56 vs +6.56 |
| 60 control | (-1300,3200) | (-2063.01,3571) | same | +6.56 vs +6.56 |

Docker compilation and all fixture executions exited 0; `DRAW_SPY_CONTRACT=PASS` for all three IDs, `FOLIAGE_CENTRE=MISMATCH` for IDs 64/65 and `PASS` for control ID60. This is an intentional diagnostic mismatch, not a patched-test pass. The expected shifts use the source-side bottom-world-Y sine formula and fixed T=0/G=1, not image samples. The sprite draws are direct calls without an `Update`, so they establish the current port draw contract and active-draw correction target, not original paused/out-of-update-zone cadence.

## Remaining handoff

After the aim-line owner explicitly releases or commits/transfers its dirty patch, re-read `LevelScene.cpp` diff and implement only the preflight's enabled ordinary static sprite `foliageSway.worldX/Y` assignments from already computed world/camera/layer values. Re-run this spy expecting IDs 64/65 centre PASS while ID60 remains PASS; then verify relevant existing CPU fixtures and Docker build, and use the exclusive physical GPU lane for same-state waterfall foliage crops. No original Windows matched frame exists here, so no Windows pixel parity or Jerome visual acceptance is claimed.

No `LevelScene.cpp`/other source/shared status file was modified, no build of the game or physical Xorg/GPU capture was warranted for an unchanged tree, and no software commit/push was made.

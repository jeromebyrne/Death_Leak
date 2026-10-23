# Waterfall foliage phase integration verification — 2026-10-23

## Ownership and production result

At task start, `port/game/LevelScene.cpp` was clean at HEAD `9ba65c3a`, and `git show 82e9b6a:port/game/LevelScene.cpp` established that commit `82e9b6a` had already integrated the exact bounded production correction after `draw.foliageSway = object.foliageSway`. Current code remains identical at that site: enabled ordinary, non-player, non-shadow, non-aim-line, non-`ParallaxLayer` foliage submits `worldX - (cameraX-worldX)*(layerX-1)` and the Y analogue. No duplicate production edit was made. This follows `GameObject::Update` World translation and leaves the pre-existing destination, `ParallaxLayer`, intensity, UV, weather, time, painter order and renderer untouched. Current `LevelScene.cpp` SHA-256: `fc106ad7e11126563f637bd83b5699681ddde48d738b2e11ec05858f5498031b`.

All unrelated dirty/untracked work observed in initial `git status` was preserved; only this report and `port/tests/waterfall_foliage_phase_parallax_source_equivalence_20261023.cpp` are owned by this lane. Shared status files were not touched. The older patch/owner-gate reports described an earlier HEAD and do not reflect the already integrated `82e9b6a` code.

## CPU verification

The new standalone XML-backed draw spy loads actual waterfall-top IDs 64, 65 and 60 one ID per process, takes two visible camera positions per ID and held-camera times 0/0.5, and asserts source `World` centre, unchanged projected destination, independently calculated source-bottom sine phase/X displacement, authored bottom X intensity and fixed amplitude. It checks that time advances phase without moving centre or placement. ID60's `(1,1)` depth is the no-change control. Docker `deathleak-linux-build:bookworm` compiled and ran it: PASS for all three IDs. The already committed `waterfall_foliage_phase_centre_draw_spy_test.cpp` independently passed `fixed` mode for all three IDs and four camera/time rows each. The Docker `make -C port -j2 all` production build passed (only existing TinyXML warnings). Logs/binaries are in ignored `port/build/waterfall-foliage-phase-owner-20261023/`.

This is active-draw command equivalence. It does not prove update-zone/pause temporal parity: source may use the last successful object-update camera while the port uses the current draw camera.

## Physical GPU same-state discriminator

The physical Xorg `:0`/nouveau NVE7 lane was free of game, FFmpeg and GL capture owners before use. A single frozen diagnostic executable was built from the current source, adding only an environment guard around the already integrated foliage-centre correction in an ignored copy of `LevelScene.cpp`. Guard unset is the before state; guard set is the integrated after state. Its SHA-256 remained `19b9d769fc300cccd496396c2008b6eca40d6ff56b62dff3d4e628b5a8e2cbe7`. All eight sequential runs used this one binary, identical assets/renderer, waterfall_top, 1280×720, stationary input, fixed dt 0.016666667, X11/OpenGL `accelerated=1 software=0 mode=strict-accelerated`, GPU Sepia, and the same logged simulation state within each pair. Capture script, source copy, lossless BMPs and logs are in ignored `port/build/waterfall-foliage-phase-owner-20261023/`.

Pillow lossless RGB comparison (per-pixel maximum-channel absolute delta):

| Pair | Changed pixels / 921,600 | Changed bbox (half-open) | Max channel delta | Mean changed-pixel max delta |
|---|---:|---|---:|---:|
| Full scene, frame 1 | 24,006 (2.60%) | (0,0)-(395,109) | 64 | 7.53 |
| ID64 only, frame 1 | 22,764 (2.47%) | (0,0)-(394,100) | 13 | 2.73 |
| Full scene, frame 120 | 15,864 (1.72%) | (898,0)-(1280,87) | 73 | 9.03 |
| ID65 only, frame 120 | 14,453 (1.57%) | (900,0)-(1280,86) | 8 | 2.16 |

ID64 is visible at frame 1 and ID65 at frame 120; these are not a single camera/time state containing both. The changed pixels are confined to top foliage regions, confirming a bounded visible Linux A/B effect. CPU asserts unchanged destination/placement and ID60's unchanged centre; no ID60 GPU invariant is claimed because it is culled at these gameplay cameras. This is not an original-Windows matched-state frame, pixel-parity claim, visual-severity ranking against other gaps, or Jerome visual acceptance.

## Release-owner lane

The requester/main integration owner should consume this scoped test/report and existing production commit `82e9b6a`, decide branch release and any Windows-matched visual acceptance. This lane stages/pushes only its verified new test and this report. Shared status, renderer/main, unrelated tests, and other dirty files remain with their owners.

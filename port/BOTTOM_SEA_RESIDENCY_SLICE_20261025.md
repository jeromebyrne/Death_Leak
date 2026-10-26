# Waterfall-bottom sea-water residency slice — 2026-10-25

## Causal map

`LevelScene::Load` eagerly decoded and uploaded every referenced image during the frame-762 Waterfall-top→bottom transition. The prior clean-HEAD physical NVE7 trace measured `sea_water.png` 2680×1133: ~27 ms PNG decode plus a variable ~15–65 ms `SDL_CreateTextureFromSurface` interval, out of ~96 ms scene load in the first run. Its RGBA payload is 12,145,760 bytes (11.58 MiB); decode buffer and GPU allocation can coexist. No reliable live-VRAM/eviction telemetry was available, so this is a payload/residency estimate, not measured VRAM usage.

Bottom XML Sprite ID4 is a static, native-sized, kGroundFront PixelWobble sheet centered at world X=-5512.19, width 2680. Its right edge is -4172.19, over 3422 world units left of the bottom camera's authored minimum X=-750. The sprite has no collider or scripted interaction. Loader `native=true` disables the authored `repeatX` path. An isolated draw-admission overlay observed the actual first bottom draw **on frame 762, immediately after load**: screen quad X=-4172.12..-2385.45 at camera X=365.985, `SpriteMayIntersectViewport=false`. It was never admitted over all 38 bottom draws through frame 799. Thus first visible draw is **none on this route**, and the authored camera envelope excludes it by a much larger margin than viewport/shake. The production skip is narrowly guarded by level, ID, kind, asset, depth, native/static/unrotated geometry and a 2048-world-unit margin from the minimum camera X. Edited geometry falls back to ordinary loading.

## Physical gate

The production candidate was built in ignored `port/build/performance-residency-20261025/` from the current tracked LevelScene plus the clean HEAD SDLRenderer blob, excluding the unrelated dirty raft-mip renderer experiment. Controls use the prior clean-HEAD `transition-linux` executable. Runner: `run-production.sh`; serial control/candidate/control for final capture frames **762, 763, 779 and 799** (`--frames` 763/764/780/800), fixed-dt scripted Forest→Waterfall-top→bottom route. Each arm reported physical Xorg `:0`, SDL x11/OpenGL accelerated=1/software=0, nouveau/NVE7, 1280×720. For every sampled frame, decoded 1280×720 **RGBA was byte-identical across all three arms**, despite BMP file metadata hashes varying. All sampled gameplay endpoint fields matched. The isolated diagnostic same-binary skip had independently matched RGBA at frames 762, 763 and 799; its logged sea draw was never admitted.

Frame-762 total milliseconds, serial A/B/A by capture route:

- Frames 763: **104.942 / 46.347 / 100.076**.
- Frames 764: **78.598 / <33 / 79.277**.
- Frames 780: **74.118 / <33 / 65.819**.
- Frames 800: **84.262 / <33 / 81.989**.

`<33` means frame 762 did not cross the profiler's slow-frame logging threshold, **not** a precisely measured value. The repeated bottom-entry tail improvement is causal, though exact magnitude varies with driver/cache state. Whole-route p50 remained ~21.04–21.13 ms; there is no steady-FPS gain or 60-FPS acceptance. The separate Forest→Waterfall-top transition still produces its own long tail. Profiles/captures/build logs are in the ignored directory.

## Decode/PBO feasibility and disposition

The 364 scripted frames between top entry and bottom entry are ample wall-clock opportunity for a background ~27-ms PNG decode, but predecoding would retain ~11.58 MiB of CPU RGBA until upload and add scheduling/contention. A chunked SDL update or GL PBO path would also require allocating the destination texture and proving completion before the *first visible* draw; nominal bytes-per-frame alone cannot bound NVE7 allocation/driver synchronization. In this case the first visible draw is absent throughout the authored bottom camera envelope, so background decode and PBO upload would spend CPU, RAM and GPU residency on unused pixels. They were not implemented or credited. A future PBO experiment should target an actually visible transition asset with explicit fence/residency timing and VRAM telemetry.

Scoped production candidate accepted for the bottom transition only. No claim of arbitrary-mode parity, full-campaign tails, 8K/high-Hz, or release readiness.

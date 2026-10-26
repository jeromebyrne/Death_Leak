# Death Leak: atlas, batching, shader, and display scaling work

Requested by Jerome on 2026-10-25. Status: implementation work authorized and active; **not yet shipped or accepted**.

## Required outcome

- Pack every technically eligible texture into densely filled atlas pages, with 4096×4096 as the maximum dimensions, not a required page size. Fill existing compatible pages before creating another and trim each emitted page to its occupied bounds. Keep an auditable manifest of packed assets and exceptions. Preserve authored filtering, alpha, mipmaps, wrap/repeat, source rectangles, tint, rotation, shader sampling and painter order; use virtual subtextures or dedicated textures where wrapping/filtering makes a shared page unsafe. Do not claim that an atlas alone reduces draw calls.
- Batch compatible consecutive draws as aggressively as exact output permits; track real submissions, texture binds, flushes, CPU and GPU times. Batch boundaries must preserve blend modes, opacity, shader uniforms, render target, scissor and painter order. Optimize the measured shader/pass bottlenecks, especially Waterfall-top fog/ground-front, rather than merely rewriting shader text.
- Accept output window/display resolutions through 8K (up to 7680×4320) while retaining the existing 1280×720 authored coordinate space unless content adaptation is explicitly proven. Map input, HUD and captures correctly; validate actual monitor modes and GPU target-size limits rather than promising any device supports 8K.
- Decouple gameplay speed from presentation refresh. Support the rates a device can actually present, including >60 Hz, without frame-count-based gameplay, animation, shader or audio drift. Preserve deterministic fixed-dt route for regression tests and use real-time/fixed-step simulation with appropriate rendering interpolation or an equivalent proven design. Verify 30/60/75/120/144/240-Hz timing behavior where hardware/software permits.
- Strict release gates: physical display + hardware-accelerated GPU, exact-frame visual checks, representative gameplay/transition parity, repeated serial A/B p50/p95/p99, memory/VRAM and thermal checks. Headless or software-rendered tests alone cannot establish acceptance. Commit and push only verified scoped changes, preserving unrelated dirty source.

## Starting facts

- 896 PNGs under Game/Media; sum of image rectangles is 451,933,320 pixels, equivalent to 26.94 perfectly packed 4096² pages before padding/packing waste. No individual PNG exceeds 4096 in either dimension; 21 exceed 2048 in at least one dimension. All-full RGBA 4K pages consume 64 MiB each, so 27 pages alone would be ~1.69 GiB before other render targets/mips. Pack by scene/usage and load on demand, not all pages resident by default.
- The SDL renderer currently sets a 1280×720 logical size and creates a 1280×720 scene target. `main.cpp` initializes a 1280×720 window and uses many 1280×720 UI/capture assumptions. The game loop caps modified update delta at 1/60; this needs an audited timing redesign for high refresh.
- `port/linux/SDLRenderer.cpp` contains pre-existing unstaged raft-mip experimental edits. Do not overwrite or stage those accidentally. The existing performance automation is scheduled every 15 minutes and its prompt has been updated to this scope. The old hourly report remains active.

## First vertical slices

1. Inventory all PNG usage, GL_MAX_TEXTURE_SIZE and GPU memory; classify each asset by ordinary atlas-safe draw versus repeat/wrap, private-GL shader, render target, or source-rect variant. Build deterministic density-first packer with a 4K ceiling and manifest with padding/extrusion plus verification that reconstructed sprites equal originals. Audit page occupancy/VRAM and choose scene-aware page groups.
2. Integrate a small ordinary-sprite family through atlas references; exact-pixel compare and physical-NVE7 serial A/B. Then expand until all eligible families are mapped; record each exception and reason. Implement compatible consecutive-draw batching on top of atlas texture identity and measure true submission reduction.
3. Isolate and optimize measured shader/pass costs, preserving pixels. Treat a rewrite with no demonstrated gain as rejected.
4. Add explicit window/fullscreen resolution mode and verify logical scaling/input/capture at multiple sizes; then rework the simulation/render clocks and test high-refresh behavior on a real display/GPU.

## Queued optimization: XML-to-JSON migration (requested 2026-10-25)

- Replace the game's XML data files and XML parsing paths with JSON, including levels, animations, materials, strings, settings and save data where applicable. Inventory all 95 XML assets and every runtime reader/writer before changing formats; keep authored data and gameplay behavior equivalent.
- Define and validate a stable JSON schema/converter, migrate references and generated assets, and provide a compatibility path for existing XML settings/saves so user progress is not lost. Remove TinyXML only after no required parser remains.
- Benchmark representative load/parse time and memory before and after; this is a queued optimization, **not** a demonstrated performance gain or a mandate to rewrite during the current atlas/GPU slice. Verify level transitions, combat, UI text, settings and save/load on the hardware-accelerated Linux build before accepting it.

## Current checkpoint

2026-10-25 19:15 PDT: Scope and live source inventory recorded. No 4K atlas has yet been integrated, no new shader gain measured, and 8K/high-refresh acceptance has not been claimed. Next scheduled performance run owns the first atlas/usage vertical slice and must leave real code or an evidence-backed blocker, not a no-work report.

### First implementation checkpoint — 2026-10-25 19:17 PDT
- Added `port/tools/atlas_pack.py`: deterministic RGBA page generation with a 4096×4096 ceiling for explicit PNG selections, 2-pixel extruded edges, source SHA-256 and per-image manifest rectangles, and exact RGBA reconstruction verification. This is a build pipeline only; the runtime does **not** yet sample atlas pages.
- Built and verified a first page containing 9 gamepad-icon PNGs. Each manifest crop byte-matched its source converted to RGBA. The output is under ignored `port/build/atlas-first-pipeline/`.
- No FPS, draw-call or memory improvement is claimed from this pipeline test. Next: integrate atlas references for that ordinary sprite family, check filtering/source-rectangle behavior and exact physical-GPU pixels, then expand coverage by usage class.

### Atlas-sizing correction — 2026-10-25
- Jerome clarified that 4K is a ceiling, not a target. Pack as many compatible assets as feasible into each page before spilling; do not allocate 4096² for sparse pages.
- The packer now uses deterministic free-rectangle placement and emits pages cropped to occupied dimensions. The nine-icon test is one 2156×320 page, versus the prior 4096×4096 allocation; exact source-pixel reconstruction still passes. This is a pipeline/memory-layout result, not runtime integration or FPS improvement.
- Before runtime use, evaluate actual page occupancy, sampling constraints and VRAM; maintain scene/usage groupings where they avoid unnecessary residency.

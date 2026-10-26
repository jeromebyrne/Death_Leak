# Cave ID3 decode-only worker handoff — 2026-10-25

## Scope and provenance

Sole source/build/physical-NVE7 owner `agent:main:subagent:5b021a40-af52-4c0f-b6a9-fa33369fb6a7`, HEAD `8478429` at start. The renderer/`main.cpp` changes were prepared from clean HEAD in ignored `port/build/performance-cave-handoff-20261025/`, then applied to the index and working tree as a scoped patch. Seven unrelated dirty tracked files remain preserved, including the unstaged raft-mip renderer hunk. No GPU texture is allocated in Forest. The sole advance buffer is the original PNG's **9,191,000-byte** decoded RGBA payload (8.77 MiB), plus decoder/vector/thread overhead.

The existing `LoadDedicatedTexture` path caches by path, decodes PNG into RGBA32, wraps that buffer in an SDL surface, then calls `SDL_CreateTextureFromSurface`. Top ID3 is visible on the first Waterfall-top draw; no lazy draw deferral is valid. The worker starts at Forest frame 120, decodes the unchanged PNG on CPU, validates 1750×1313 and payload size, and publishes readiness with release/acquire synchronization. At top scene load, the renderer moves the ready vector into the **same** SDL surface/create path. Missing/late/invalid worker data falls back to synchronous libpng decode. Join-on-exit guards early-return paths; normal exit joins explicitly. No driver allocation is moved into Forest.

## Physical evidence

Same isolated candidate binary (`c6f3427f6b3975fd658bb58b8e4373d8c39e499592e19a14bc1ca94307ac896a`) ran serial control/handoff/control/handoff/control on physical Xorg `:0`, nouveau/NVE7, SDL x11/OpenGL accelerated=1/software=0, 1280×720, fixed-dt `R:800,L:1200`. All arms reached the same gameplay endpoint. An explicit fixed-dt film-clock diagnostic and hidden FPS overlay were applied **equally to both arms** only for synchronized visual captures; neither is in the production patch.

- Atomic first-reveal top frame **415** and postfade moving-camera top frame **424**: all five 1280×720 BMP pixel payloads matched byte-for-byte at each ordinal. First-reveal SHA-256 prefix `7afac7b236d6889db438`; postfade `bac1d69aa2415bd8447b`. The two frames differ from each other; equality is cross-arm at the same ordinal. The packet JSON validates transition state. These captures add large readback stalls and are excluded from the clean-tail result.
- Capture-run frame-411 totals: control **130.027 / 127.356 / 112.627 ms**; handoff **95.153 / 105.967 ms**. Cave decode controls **30.779 / 33.159 / 31.437 ms**; handoff **0**; worker completed in **40.298 / 36.008 ms** before top entry.
- Independent **no-readback** serial control/handoff/control/handoff/control top frame-411 totals: **98.618 / 78.100 / 121.630 / 76.496 / 100.137 ms**. Each candidate beats both adjacent controls, by **20.5–45.1 ms**. Controls decoded cave PNG in **31.725 / 31.959 / 32.559 ms**; candidates did not decode it at scene load. SDL create/upload remained variable (**12–29 ms**) and was not changed. All five had exactly one >33.333-ms frame: top entry, not Forest. Route p50 **19.256–19.378 ms**, p95 **21.378–21.470 ms** overlap; no sustained FPS gain claimed.
- Exact staged production-source build passed. A production-binary physical smoke run selected strict accelerated NVE7, decoded the buffer on worker (**39.975 ms**), consumed all **9,191,000 bytes**, and reached top with a **61.387-ms** frame-411 transition. This is a smoke result, not an added comparative A/B.

## Decision and limits

**Accept the scoped decode handoff** for the repeatable top-entry tail reduction and exact two-frame top pixel parity. The first top load remains >60 ms, and Forest median remains above 16.667 ms; this does not establish 60 FPS, 8K/high-refresh, whole-campaign or release acceptance. `ru_maxrss` does not calibrate incremental live RAM/VRAM; the known minimum temporary buffer is 8.77 MiB. Worker scheduling only covers this Forest start/route; late or skipped starts retain the synchronous fallback. A subsequent measured slice should inspect the remaining top-entry SDL allocation/driver tail or another distinct transition cost, without repeating rejected full preallocation or raw-sidecar substitutions.

Reproduce with ignored `port/build/performance-cave-handoff-20261025/run.sh` (atomic exact captures) and `run-nocapture.sh` (tail); logs, binaries, source overlays, and captures reside alongside them.

# Ordinary SDL painter/flush-span gate — sampler setter rejected for frame rate

Source: clean detached exact pushed `671339549917a27e112f43fd9ba58722cfa5be56` in `/tmp/deathleak-ordinary-painter-20261026`. The unrelated dirty primary at `a5e39ac` was not used for source/build. Docker production baseline and candidate builds passed. Physical route was authenticated X11 `:0`, SDL OpenGL Mesa nouveau **NVE7**, `accelerated=1 software=0 vsync=1`, output 1280×720, fixed 1/60 update, Forest-1 and Waterfall-top. No competing game/build/apitrace process was observed at preflight; all physical runs were serial. Evidence is under `/tmp/deathleak-ordinary-evidence/`. An initial unauthenticated offscreen Intel probe was discarded before physical profiling.

## Measured ordinary span and concrete B arm

Fresh exact-ref three-frame physical apitraces and two serial GPU-query replays measured the third active frame. Before the Sepia scene-target switch, **27 ordinary SDL draws in Forest** took 10.087/10.022 ms query-on; **22 in Waterfall** took 7.325/9.262 ms. Waterfall's ID14→ID117 interval contained 15 ordered draws, 11 ordinary SDL and four private fixed-function, not an empty flush. Private `glBegin` draws before Sepia were 3 Forest and 8 Waterfall. Query replay variation, especially Waterfall's second run, prevents treating these sums as query-off frame time or removable overdraw. SDL queue flushes at private-GL boundaries are required to retain painter order; the prior explicit target-flush deletion was independently rejected for no frame gain.

A distinct recurring **state-submission** candidate was found. `DrawSprite` calls `SDL_SetTextureScaleMode` on every ordinary copy; the physical GL dump shows repeated `glBindTexture` plus two `glTexParameteri` even when the mode remains identical. Waterfall texture 259 emitted 20 setter triplets before its coalesced SDL draw, with both actual linear↔nearest transitions and redundant same-mode repetitions. The smallest isolated candidate read the current `SDL_ScaleMode` via `SDL_GetTextureScaleMode` and called the setter only on error or a different mode. This preserves the requested sampler state, SDL queue order, texture IDs, vertices, blend, shader, resolution and assets. It did not touch previously rejected atlas, sheet/fog or Sepia families.

Focused fixed-dt physical three-frame call fixture, strict serial A/B/A (baseline/candidate/baseline), with identical test-only frame-indexed film clock in both binaries:

| Third active frame | Forest A/B/A | Waterfall A/B/A |
| --- | --- | --- |
| `glBindTexture` | 103 / **46** / 103 | 205 / **69** / 205 |
| `glTexParameteri` | 140 / **26** / 140 | 346 / **74** / 346 |
| `glDrawArrays` | 32 / 32 / 32 | 27 / 27 / 27 |
| private `glBegin` | 6 / 6 / 6 | 15 / 15 / 15 |

The reduced GL state calls recur in both sampled scene frames; draws and raster work are unchanged. They are not measured GPU-memory or frame-rate savings. Strict serial physical 100-update full post-HUD BMP A/B/A with the same test-only film clock matched byte-for-byte: Forest SHA-256 `47567cec22c0e00e956800cd1aee8d61a90cba49a46fcc3d538a8515bc044bbf`; Waterfall `5c39f71ba7495263ba5958d2b7d805104870ba3bd3460282bdf28f2f73b40bde`. All gameplay endpoints matched. This proves those synchronized endpoints, not every frame or arbitrary native wall-clock film phase.

## Query-off timing and disposition

Native-clock production binaries, no apitrace/GPU query, 300-update physical A/B/A per scene; frame samples are after 120 warmup updates (180 each). Units milliseconds:

| Scene | A1 p50/p95/p99 | B p50/p95/p99 | A2 p50/p95/p99 |
| --- | --- | --- | --- |
| Forest-1 | 21.298 / 24.578 / 24.661 | 21.259 / 24.572 / 24.633 | 21.325 / 24.579 / 24.614 |
| Waterfall-top | 20.471 / 23.789 / 23.892 | 20.482 / 23.791 / 23.839 | 20.527 / 23.795 / 23.860 |

Forest's 0.04–0.07-ms median decrease is small; Waterfall B lies between controls at p50/p95. p99 reverses relative to one Forest control and Waterfall A2. Presentation interval tails also overlap, with Waterfall B maximum 28.80 ms versus A1 24.59 and A2 26.24. **Reject the sampler guard as a frame-rate change:** despite exact sampled pixels and large recurrent state-call reduction, there is no reproducible end-to-end p50/p95/p99 gain across the two active routes and no raster work removed. Candidate and test-clock source were removed from the isolated worktree; no runtime patch/page is accepted. Its B binaries, trace and BMP evidence remain local only. No measured VRAM or thermal change; atlas coverage stays 15/896 PNGs (881 unatlased).

Next distinct gate: attribute the **visible ordinary SDL fragment workload**, not more setter/flush calls. Use ordered painter-ID plus camera-position coverage on Forest ID1→ID22/tree-strip and Waterfall far/middle ground, with an exact source-alpha/sampler-aware opaque coverage oracle or fragment-invocation/occlusion query that identifies a fully covered region *after* its painter successors. Only then prototype a conservative geometry/scissor skip and require synchronized multi-camera full-frame physical A/B/A, steady GPU calls, query-off latency/tails, and unchanged gameplay. The existing ID1 hide/scissor changes authored pixels and is not a candidate. No 8K/high-Hz, sustained 60-FPS, combat/transition, VRAM/thermal or release acceptance is established.

# Active Waterfall-top scene-target flush gate — no accepted optimization

Starting point: detached clean fetched remote `dc7fe70569af7a54f9a8f9cd3083eab594d9fedf`. The primary tree's seven pre-existing dirty tracked files, especially `port/linux/SDLRenderer.cpp`, were not built, edited or staged. Test-only instrumentation, ordinal film clock and opt-in candidate were confined to `/tmp/deathleak-target-transition-20261026`.

## Measured boundary

The accelerated 1280×720 scene is queued into the SDL scene target, then `ApplyScenePostprocess` explicitly calls `SDL_RenderFlush`, switches to the physical output with `SDL_SetRenderTarget(renderer_, nullptr)`, binds the scene texture, submits the Sepia quad, and unbinds. The prior Sepia gate already measured zero framebuffer copies; the present gate isolates **CPU call-wall times** at the non-Sepia flush/target boundary. It does not equate these timings to GPU elapsed or VRAM bus traffic. The one opt-in candidate skipped only the explicit pre-target `SDL_RenderFlush`; target switch, Sepia shader/draw, HUD and renderer state otherwise remained unchanged. The explicit flush call count was 300/0/300 across control/candidate/control; the target switch and scene bind still occurred once per active frame.

Serial physical Xorg `:0`, SDL x11/OpenGL strict accelerated=1/software=0/vsync=1, nouveau NVE7, actual/logical 1280×720. Each fixed-dt 300-update run stayed in `waterfall_top.xml`, ending at `(818.985,2933.64)`, one orb, 25 health, zero shots. Profiling discarded 120 warmup frames and used 180 samples per arm. The first control's CPU API-call p50/p95/p99 were: explicit flush **0.0002/0.0002/0.0003 ms**, target switch **0.0017/0.0029/0.0035 ms**, scene texture bind **0.0016/0.0021/0.0031 ms**. Candidate target-switch p50 remained **0.0017 ms**. This does not reveal a material main-thread serialization stall at the explicit flush or target switch. Deferred GPU work may be paid later and is not attributed by these CPU call timers.

| Warmed whole-frame wall ms | control A | skip-flush candidate | control B |
|---|---:|---:|---:|
| p50 | 21.8627 | 21.8112 | 21.8142 |
| p95 | 22.0778 | 22.0531 | 22.0264 |
| p99 | 22.2053 | 22.1977 | 22.1147 |

The candidate falls inside control variation and is slower than control B at p95/p99. The last-frame capture adds a ~55-ms slow frame in each arm (54.67/54.81/56.10 ms maximum); it is not steady gameplay. Timed captures are not a full-pixel parity control because `--profile` enables an FPS glyph: the first control differs from both other arms in 16 glyph pixels. A **separate no-FPS** serial 300-update physical A/B/A with a test-only ordinal film clock produced byte-identical full post-HUD BMPs in all arms, SHA-256 `342ebe1b2a01a4e59371add1207bb6503b96d3beae728350e73512c68d2d69fc`. This verifies the sampled synchronized endpoint only, not all frames, scenes, resolutions or SDL backends.

## Disposition and next distinct task

**Reject skipping the explicit pre-target flush as a production optimization:** it removes one nearly free CPU API call and produces no repeatable frame-latency improvement. We do not infer that the call is universally redundant merely from one matched output sample, nor claim measured GPU-time/VRAM savings. No production source change, 60-FPS, 8K/high-Hz or release acceptance follows. Physical raw logs, BMPs, runner, test-only source diff and build log are retained under ignored `port/build/target-transition-20261026/` in the primary project after worktree retirement.

Next distinct bounded task: instrument **ordinary SDL sprite overdraw** outside Sepia at the current Waterfall-top far/middle-background span (previous depth-band query p50 about 7.4 ms with ID58 already optimized). Count actual SDL queued draw/texture changes and isolate GPU time for ID1 versus the subsequent ID22/tree-strip span on moving-camera ordinals. Only test an occlusion/geometry candidate if ordered opaque coverage is proven in scene-target pixels; require synchronized multi-camera full-frame exact pixels and repeated strict physical A/B/A GL/latency/traffic. A visual-changing hide/scissor diagnostic is not shippable; prior ID22 hide saved only ~0.9 ms median.

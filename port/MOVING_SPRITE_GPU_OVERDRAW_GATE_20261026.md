# Moving-camera ordinary-sprite GPU/overdraw gate — 2026-10-26

## Provenance and method

Clean detached worktree from `de413d64d1c2735db13ae9196364e66bd089e9b3`; primary dirty tree untouched. Strict physical Xorg :0 SDL x11/OpenGL accelerated=1/software=0/vsync=1 on nouveau NVE7 at 1280×720. Test-only `GL_TIME_ELAPSED` instrumentation brackets each tagged `DrawSprite` and forces SDL command flush; therefore timings attribute serialized isolated draw spans, **not** query-off frame savings. Fixed-dt Waterfall-top `R:120` moves player x818.985→1750.99 with no level transition. Instrumentation/logs/trace retained under ignored `port/build/moving-sprite-gpu/` in primary project. A 30-frame apitrace was dumped and command spans mapped in log order to sprite IDs.

| ID | 120-frame spans | GPU p50/p95/p99 ms | Sum ms | 30-frame GL commands within span |
|---|---:|---:|---:|---|
| 1 far tree-strip background | 119 | 1.257/3.553/3.603 | 198.378 | 29 immediate-mode `GL_QUADS`, 116 binds, 290 texture-parameter calls |
| 22 ground blur | 120 | 0.875/0.978/1.061 | 107.555 | 109 `glDrawArrays` for 30 instances, 60 binds; median 4 draws/frame |
| 25 tree strip | 120 | 0.502/0.700/2.440 | 64.964 | 30 `glDrawArrays`, 60 binds |
| 24 foreground tree | 120 | 0.934/3.278/3.295 | 169.411 | 30 `glDrawArrays`, 35 binds |
| 41 later sprite | 120 | 0.044/0.067/0.072 | 5.477 | 30 `glDrawArrays`, 60 binds |

ID42 appears only 31/120 frames (p50 0.120 ms); the 30-frame trace includes one ID42 draw each frame. Overall trace: 4,090 `glDrawArrays`, 7,079 `glBindTexture`, 390 `glTexSubImage`, zero `glFinish`/`glFlush` calls. Those totals include startup and all other scene/UI draws; they are not per-frame workload savings. ID1 uses the private fractional-parallax immediate-mode GL path, not an SDL `glDrawArrays` copy; trace confirms one quad plus wrap/sampler state changes per measured ID1 instance.

## Ordered exact-pixel candidate gate

The apparent opaque-cover opportunity is **rejected**. PNG `ground_blur.png` has fully opaque source rows 9–869 and covers the viewport horizontally on this route, but the actual renderer draw sequence per frame is **22 → 25 → 42 → 24 → 41 → 1**, not the XML declaration order/depth intuition. Apitrace query order confirms this. ID1 is drawn **after** ID22 and tree strips, so opaque ID22 cannot overwrite any ID1 pixel. ID1 source `tree_strip_bg.png` is fully opaque but authored alpha 0.9 (runtime tint 230/255) and ordinary `SRC_ALPHA, ONE_MINUS_SRC_ALPHA` blending. Removing/clipping ID1's lower band changes the composite wherever it covers those already-drawn layers. ID25 `tree_strip.png` has no fully opaque scanline, and both repeated strips may expose filtered-alpha tile boundaries. There is no provable copy-free, exact-output opaque-coverage/geometry candidate in this bounded route.

Thus no production edit, synchronized candidate pixel comparison, or serial A/B/A candidate benchmark was run. GPU query spans establish priority but cannot be interpreted as FPS gain. Next distinct investigation: ID1 fractional-parallax private-GL state churn and shader/geometry path, or broader atlas/batching eligibility, each requiring its own exact-output proof before performance acceptance. Sustained 60 Hz, 8K/high-Hz, VRAM/thermal and release gates remain open.

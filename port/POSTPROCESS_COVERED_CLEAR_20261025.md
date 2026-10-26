# Physical NVE7 covered-postprocess-clear optimization — accepted

Starting HEAD `90b0fc2`; sole source/build/physical GPU owner used a detached clean-HEAD worktree. The active-gameplay Sepia pass previously cleared the physical output black then painted an opaque full-viewport quad before HUD. The scoped patch omits `glClear(GL_COLOR_BUFFER_BIT)` only if the Sepia viewport exactly covers the drawable; letterboxed outputs retain the original clear, preventing stale bars. Scene painter order, shader, target sizes and authored pixels are unchanged. Seven unrelated dirty tracked paths in the primary tree, including unstaged raft-mip `port/linux/SDLRenderer.cpp`, were preserved.

## Corrected physical gate

A test-only isolated build toggled the proposed skip with a literal `on` flag. Its initial `getenv`-presence predicate mistakenly treated `off` as enabled; that first B/B/B comparison is invalid and excluded. Corrected runs used one binary and serial true control/candidate/control on physical Xorg `:0`, SDL x11/OpenGL accelerated=1/software=0/vsync=1, nouveau/NVE7, 1280×720, fixed-dt active Forest `B:360`. Test-only renderer counters established **360 / 0 / 360** executed postprocess clears per 360-update route. An attempted LD_PRELOAD GL interposer returned zero on both arms and was discarded; these are pass-branch counts, not driver bytes or GPU-time queries.

With a test-only ordinal gameplay-film selector applied identically to all three arms, full 1280×720 final post-HUD BMPs matched byte-for-byte, SHA-256 `79b527c0cbbbf51a832a54f9ae05b16028aff8bebf9e5219839e29e3b884955d`. Every arm reached Forest player `(2808.99,3118.11)`, 7 orbs, 25 health, 22 fired. This is synchronized endpoint parity, not all-intermediate-frame parity. Native-clock capture varied even between controls due to gameplay film sampling monotonic wall time; no native-clock pixel claim is inferred.

Separate serial untraced profiling (120 warmup, 240 samples per arm) was repeated twice. Values are milliseconds, control/candidate/control:

- First corrected A/B/A p50 **20.6008 / 20.1437 / 20.6625**, p95 **22.456 / 21.9715 / 22.4716**, p99 **22.656 / 22.0652 / 22.5528**.
- Second A/B/A p50 **20.5325 / 20.1223 / 20.6484**, p95 **22.463 / 21.9841 / 22.4639**, p99 **24.9344 / 22.0703 / 22.5679**. The first second-series control had two >25-ms outliers, but p50/p95 and the other five arms support a repeatable ~0.4–0.5-ms gain versus neighboring controls.

The production patch removes all diagnostic flags, branch counters and film-clock changes. Exact-source isolated build passed and a physical NVE7 360-update Forest smoke reached the same gameplay endpoint with p50/p95/p99 19.9958/21.9969/22.1606 ms (120 warmup, 240 samples). This is a smoke result, not a third controlled A/B/A. Acceptance is limited to 16:9 full-viewport postprocess traffic and this active Forest route; no direct VRAM-traffic byte estimate, all-frame parity, 60-FPS, 8K/high-Hz or release claim. Letterboxed modes retain the clear by code contract; a distinct physical 1440×900 parity gate remains worthwhile. Ignored logs/BMP/build evidence: `port/build/shader-clear-20261025/`. The detached worktree was retired after copying the three exact-pixel BMPs; no test-only binary or film-clock source was shipped.

Next distinct task: instrument Waterfall-top fog/ground-front shader spans and target bind/copy boundaries with GPU timing queries or apitrace on a repeatable active-top route; optimize only a cost above control variance while preserving exact synchronized pixels and painter order.

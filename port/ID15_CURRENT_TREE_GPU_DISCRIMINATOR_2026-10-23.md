# ID15 current-tree physical-GPU discriminator — 2026-10-23

## Outcome and release lane

**Read-only feasibility audit; no current-tree GPU A/B was run.** Physical Xorg `:0` had no game/recorder display owner, and the existing binary supports an XML-ID render filter, but a concurrent integration owner was actively running a long, CPU-intensive integration suite against the same checkout and executable. A 540-frame accelerated capture pair would add substantial CPU/GPU load and risk perturbing that owner's timed fixture/run, so this lane was left idle under the assigned non-interference condition. No build, source/test edit, baseline/hide capture, shared status edit, or commit was made. The physical-GPU lane is **released**; integration ownership is unchanged.

The prior physical-GPU frozen-frame evidence **quantifies authored ID15 causation on its earlier tree**, not the current tree: hiding ID15 changed 81,525 pixels in bbox `[985,323,1280,612)` and 47,343 pixels in ROI x=900–1199/y=300–549; at `(1100,350)`, baseline RGB `(149,129,93)` became `(95,82,58)`, and at `(1200,400)` `(120,103,74)` became `(52,44,31)` (`port/PALE_PATCH_GPU_AUDIT.md`). Its only-ID15 pass reproduced the slab silhouette. The current 9-second clip's pale field is geometrically consistent with that attribution (`port/CURRENT_TREE_PALE_EDGE_ATTRIBUTION_2026-10-23.md`), but **current-tree delta pixels/bounds are unmeasured**. No matched original-Windows frame exists for this shaft (`port/FOREST_ID15_MATCHED_REFERENCE_2026-10-22.md`); therefore neither this audit nor the historical A/B supports an ID15 parity/art fix.

## Exact read-only evidence and feasibility

Commands were run from `/srv/openclaw/workspace/death_leak_analysis/source`:

```sh
ps auxww | grep -E 'Xorg|Xvfb|death|Death|love|ffmpeg|compare|capture' | grep -v grep
ps -eo pid,ppid,stat,etime,args | grep -E '[d]eathleak-linux|[d]ocker run.*deathleak|[g]\\+\\+|[c]make|[X]org|[f]fmpeg' | head -35
sed -n '1,120p' port/tests/integration.sh
stat -c '%y %s %n' port/build/deathleak-linux port/game/LevelScene.cpp port/linux/main.cpp
sha256sum port/build/deathleak-linux
grep -n 'DiagnosticRenderFilter\\|filter.Draw' port/game/LevelScene.cpp
strings port/build/deathleak-linux | grep -E '^DEATHLEAK_RENDER_(HIDE|ONLY)_XML_IDS$'
git rev-parse --short HEAD
```

At preflight, Xorg `:0` was alive (PID 1360); no Death Leak renderer, FFmpeg, or capture process owned the display. The concurrent `docker run ... deathleak-linux-build:bookworm sh -x port/tests/integration.sh` (host PID 2805261) was active. Its game child was `--simulate-only` (first observed `sea_2.xml`, later `forest_exploration_1.xml`), with the test moving through the suite. The script deliberately selects `SDL_VIDEODRIVER=offscreen`, compiles a CPU fixture, and runs many game simulations. Thus it did not claim physical Xorg, but **did** own an active integration/CPU lane and the same working-tree binary.

Checkout HEAD was `19c28af`; existing executable was 8,974,616 bytes, mtime `2026-10-22 22:46:47 -0700`, SHA-256 `4011c2fc6de7a758b46331b9c8bffcf49da933743806f8572f334dcdb318e158`. Both filter names occur in that executable. Source `port/game/LevelScene.cpp:64-87,1189,1233` parses comma-separated XML IDs and gates ordinary sprite drawing; blank/unset filter preserves baseline. `port/linux/main.cpp:92,102,1218` supports `--capture`, `--fixed-dt`, and saving the final frame. The historical physical-Xorg route in ignored `port/build/pale-patch-gpu-audit-2026-10-22/capture.sh` uses `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:300,F:240 --frames 540 --fixed-dt 0.0166667 --capture ...`, with `SDL_VIDEODRIVER=x11 SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy`; the prior report documents the host's `/dev/dri`, Xauthority, `nouveau/NVE7`, and strict accelerated verification. This makes a sequential same-frame baseline / `DEATHLEAK_RENDER_HIDE_XML_IDS=15` comparison feasible **after** integration relinquishes its lane, using this frozen binary and distinct output paths, with no rebuild required.

A later authorized capture should record binary hash before/after, baseline and hide-15 final BMPs plus logs, verify both x11/OpenGL accelerated strict/NVE7 gates and identical simulation endpoint, then compare decoded RGB changed-pixel count, bbox, ROI, and edge samples. An `ONLY_XML_IDS=15` diagnostic can confirm silhouette, while `HIDE_XML_IDS=38,39` separates broad brightness. It must not be represented as Windows pixel parity without a matched Windows camera/frame/effect-state capture.

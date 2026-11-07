# Forest-2 ID37 natural active-gameplay route GPU gate — 2026-10-24 01:09Z

## Verdict

**Natural-route relevance proven, small effect.** At HEAD `eed88f0`, unforced active Forest-2 leftward gameplay (`--play forest_exploration_2.xml --script L:150`) produces a shown/hidden ID37 difference of **693 RGB pixels** in a 1280×720 physical-GPU frame. The effect is one small leaf at the far left (`[17,245,44,290)`) and does not cross the player's central silhouette. This demonstrates the ID37 admission is visible on a reachable route, not merely under the prior forced-camera diagnostic. It does **not** establish original-Windows pixel parity or a large visual improvement.

## Route/camera and bounded sampling

The authored Forest-2 camera is `posy=3400`, `followy=false`, `followx=true`, `xLag=15`, zoom 1 (`Game/XmlFiles/levels/forest_exploration_2.xml:3`). ID37's emitter is centered at `(-20.714647,4345.097168)` with size `50000.269531×2000.207764` (`:595-615`); its vertical cull boundary is camera Y≈2804.993, so the normal Y=3400 camera intersects the **emitter from the initial active frame**. That is only emitter admission; the warmed leaves' X/Y positions determine whether any pixel survives the viewport and painter stack. The original frame-0 unforced A/B was 0 pixels (prior `PRIVATE_WATCHDOG_FOREST2_ID37_IMPLEMENTATION_GPU_20261024T0102Z.md`).

Using the same unforced camera and a single held-left input from the authored Forest-2 start, the first **sampled** positive is frame 150; frame 120 was 0. This is a bounded sample, not a claim that ordinal 150 is the exact first nonzero frame. The route remains in Forest-2 with player `(1285.99,3110.82)`, health 25, no enemies/hits at frame 150. At frame 180, still Forest-2, player `(1045.99,3111.33)`, health 25. At frame 240, player `(565.985,3112.37)`, health 25. Independent `L:500 --simulate-only` stays in Forest-2 rather than requiring a forced scene/camera. Idle `.:240` at the starting X remained 0, distinguishing natural camera intersection from natural visible leaf relevance.

| Input / captured frame | Shown-hidden changed RGB pixels | Difference bbox, half-open pixels |
| --- | ---: | --- |
| `.:240` / 240 | 0 | none |
| `L:120` / 120 | 0 | none |
| `L:150` / 150 | 693 | `[17,245,44,290)` |
| `L:180` / 180 | 713 | `[167,319,194,363)` |
| `L:240` / 240 | 697 | `[391,465,417,510)` |

At frame 150 the difference footprint is 27×45 px, **0.075%** of the 921,600-pixel frame by changed-pixel count; at frame 240 it is 26×45 px and 697 pixels. The leaf moves into view as the naturally following X camera tracks the left-moving player. All changed pixels are confined to the left-side leaf bboxes; the player remains in the central gameplay area, with no A/B difference over its silhouette. Thus ID37 adds atmosphere without a demonstrated player-legibility regression in these sampled states. It is not a player-contrast fix.

## Capture provenance / reproduction

Lossless BMP A/B and complete logs are in `port/build/private-forest2-id37-natural-route-20261024t0109z/`. Both arms used the **same** previously built current-source binary `/tmp/deathleak-id37-gpu-0102/port/build/deathleak-linux`, same authored scene, held-left script, frame limit, fixed dt `0.0166667`, assets, and physical Xorg `:0`; hidden control alone set `DEATHLEAK_RENDER_HIDE_XML_IDS=37`. No camera script or `DEATHLEAK_ID37_DIAG_CAMERA_Y` was supplied. A/B final game-state lines match at each sampled frame after excluding the measured wall-clock `elapsed` field (player, orbs, health, enemies, movement, and scene all match). Logs confirm `SDL_VIDEODRIVER=x11`, `SDL_RENDER_DRIVER=opengl`, strict accelerated `software=0`, physical `nouveau/NVE7`, initialized Sepia.fx GPU postprocess, and 62 loaded scene sprites with no missing assets. Exact pixel counts used Pillow `ImageChops.difference` in RGB and counted pixels with any nonzero channel, avoiding grayscale cancellation. The copied binary was inherited from the prior ID37 GPU gate; no project source or binary was rebuilt for this continuation.

A representative Docker invocation is the same as `port/build/private-id47-gpu-gate-20261024t0057z/capture.sh`, substituting source mount `/tmp/deathleak-id37-gpu-0102:/src`, this report's artifact directory as `/out`, scene `forest_exploration_2.xml`, `--script L:150 --frames 150 --fixed-dt 0.0166667 --capture /out/shown-left-150.bmp`; rerun identically with `-e DEATHLEAK_RENDER_HIDE_XML_IDS=37` for `/out/hidden-left-150.bmp`. The host source `Game` directory was mounted read-only to `/src/Game`.

## Scope / disposition

No authored art/XML, source, renderer, test, checkpoint, or unrelated dirty file was changed. No software fix was indicated, so no commit/push. Only this unique report and its ignored capture artifacts were written. GPU/build lane released. Further original-Windows matched-frame work remains a separate parity question.

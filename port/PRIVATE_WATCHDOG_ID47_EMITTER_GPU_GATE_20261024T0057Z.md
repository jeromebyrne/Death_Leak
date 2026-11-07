# Forest-1 ID47 emitter draw admission — physical-GPU gate

Disposition: **CPU and physical-GPU gate pass for the source-backed ID47 admission change; natural-route pixel impact is not established.** The current-tree software is already committed/pushed as `f56ad84630cf912b00dc4895f3617ab802c5b16c`; no additional software edit or commit was made here. No matched Windows D3D10 packet or visual-parity acceptance.

## Build and oracle

- Inspected `PORT_WORK_STATUS.md` and scoped Git first; unrelated dirty renderer/test/checkpoint files were preserved. Current-tree `make -C port -j2` passed in `deathleak-linux-build:bookworm` with SDL2/libpng/GL available (only pre-existing compiler warnings). Focused `forest47_emitter_draw_cpu_test.cpp` passed in that same image with `-UNDEBUG`.
- For **same-binary** old/new render-only A/B, copied current-tree source into isolated `/tmp/deathleak-id47-gpu-0057` and compiled it once in that Docker image. Two temporary diagnostic hooks, not present in the project tree or committed binary, allow `DEATHLEAK_ID47_OLD_DRAW=1` to bypass only the new ID47 draw gate, and `DEATHLEAK_ID47_DIAG_CAMERA_Y` to override only the final draw camera Y. Particle update, input route, assets and renderer are common between A and B. Binary: `/tmp/deathleak-id47-gpu-0057/port/build/deathleak-linux`. Capture script and six 450-frame BMP/log packets are in `port/build/private-id47-gpu-gate-20261024t0057z/`.
- Every capture log reports strict physical X11/OpenGL (`accelerated=1 software=0`), `nouveau`/`NVE7`, GPU Sepia.fx initialized, 71 loaded sprites, zero missing assets. Captures were sequential on Xorg `:0`, same binary, level `forest_exploration_1.xml`, fixed dt `0.0166667`, script `.:450`, final frame 449, 1280×720.

## Pixel results

| Draw-camera state | Source gate (new) vs prior per-leaf draw (old) | Decoded RGB delta |
| --- | --- | --- |
| Forced Y=2793.892212, 1 world unit outside source whole-emitter lower admission edge | Old draws a warmed leaf; new rejects emitter | **679 pixels**, bbox `[916,33,942,75)`; `old-off.bmp` vs `new-off.bmp` |
| Forced Y=2795.892212, 1 unit inside admission edge | Both admit emitter and draw leaf | **0 pixels**; BMP SHA256 identical (`d841aae4e40cf0e9...`) |
| Authored camera, first gameplay frame | Both admit emitter | **0 pixels**; `old-normal-f0.bmp` vs `new-normal-f0.bmp` |
| Authored camera, frame 449 | Both should admit emitter | 571,199 pixels vary across broad screen; **unusable as a regression discriminator** because broad unrelated temporal rendering variance was not isolated. Do not attribute that delta to ID47. |

The forced off-edge capture is a genuine GPU-rendered gameplay-scene pixel demonstration, but the forced draw-camera Y is diagnostic, **not a naturally reached active-gameplay route**. Forest-1 authors `followy="false"` and camera Y=3400 in its XML, whereas the ID47 lower admission boundary is Y≈2794.892212. Therefore ordinary Forest-1 play remains well inside the ID47 emitter bounds and this available route cannot establish a natural active-play pixel improvement at that edge. The inside-edge exact match and normal first-frame exact match cover the immediate no-regression case. No claim about other routes or Windows pixels.

## Scope and handoff

Project source/tests remain clean in the ID47 scope after the already-pushed `f56ad84`; unrelated working-tree dirt untouched. Keep the patch as source-equivalent admission behavior with a small **conditional** visual effect. If a genuine active camera route later approaches Y≈2794.9, recapture there without the diagnostic camera override before claiming player-visible improvement.

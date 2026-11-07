# Forest-1 ID21 pre-sepia control capture plan — 2026-10-23

## Decision

A **Linux-only, same-frame pre-sepia ID21 hide ladder is executable after a one-line capture-only hook in an isolated binary**; the current `--capture` option alone is not pre-sepia. It saves after scene postprocess and HUD. No new renderer/art correction is justified by the present evidence: original source and current Linux agree on ID19 → ID21 → ID22 painter order, and the existing post-sepia GPU ladder shows ID21 occluding x320 in this Linux frame. The missing discriminator is how its *sampled alpha transition* behaves before sepia, followed by a matched original-Windows packet for parity. Do not interpret a Linux-only edge difference or mip A/B as Windows mismatch.

## Exact seam and non-racing capture method

`port/linux/main.cpp:1191-1194` calls `scene.Draw(...)` then `renderer->ApplyScenePostprocess()` in the same render iteration; `:1224-1227` calls `SaveFrame` only after HUD and just before `EndFrame`. `SDLRenderer::SaveFrame` uses `SDL_RenderReadPixels` to a lossless RGBA32 BMP (`port/linux/SDLRenderer.cpp:776-790`). `ApplyScenePostprocess` flushes SDL and copies the already-composited framebuffer to GL before applying Sepia.fx (`:674-724`), so a SaveFrame immediately **between** those two calls is a pre-sepia scene readback with no HUD. It runs synchronously on the render thread, before postprocess; it does not race a separate `xwd`, ffmpeg, GPU process, or display read. `SDL_RenderReadPixels` itself flushes queued SDL work. Preserve the ordinary post-sepia `--capture` too as a registration/control image.

Build a private overlay from the *current dirty source snapshot*, not from HEAD or the old shared binary. In the overlay copy of `port/linux/main.cpp`, replace the **single** exact `renderer->ApplyScenePostprocess();` statement with:

```cpp
if (!capture.empty() && frame == frameLimit - 1) {
    std::string preError;
    if (!renderer->SaveFrame(capture + ".pre.bmp", preError)) {
        std::cerr << "Pre-sepia capture failed: " << preError << '\n'; return 1;
    }
}
renderer->ApplyScenePostprocess();
```

This is a capture hook, **not** a proposed shared-source patch. Concrete isolated compilation route, using the existing `port/Makefile` translation units and flags: make `OUT=port/build/forest-id21-presepia-control-2026-10-23`; copy only `port/linux/main.cpp` to `$OUT/overlay/linux/main.cpp`; symlink `$OUT/overlay/{render,game,audio}` to the corresponding absolute current-tree directories so its relative includes resolve; patch only that copy with a Python exact-one-replacement assertion; then in the existing `deathleak-video-capture:bookworm` build image run from `/src/port` the Makefile's g++ command with `linux/main.cpp` replaced by `$OUT/overlay/linux/main.cpp` and output `$OUT/deathleak-linux.presepia` (do not use `make`'s shared `build/deathleak-linux` target). Snapshot `git status --short`, source hashes, executable SHA-256, and the compiler command; compare the snapshot after. The `Game/tinyxml/*.cpp` inputs are unchanged. No build or edit was made for this report.

Before using Xorg, verify no other Death Leak physical-GPU container/process owns `:0`, and coordinate the exclusive lane. Run **sequentially** in one physical-GPU X11 container using one frozen binary, one image, one mount, and no concurrent display capture. Existing proven route: `--play Game/XmlFiles/levels/forest_exploration_1.xml --script R:180 --frames 180 --fixed-dt 0.0166667 --capture <variant>.bmp`, with `SDL_VIDEODRIVER=x11 SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy`, `DISPLAY=:0`, `/dev/dri`, `/tmp/.X11-unix`, and the LightDM Xauthority read-only bind (`port/FOREST_ID21_OVERDRAW_GPU_DISCRIMINATOR_2026-10-23.md`; `port/build/forest-raft-fog-same-frame-gpu-2026-10-23/capture.sh`). Four environment variants, in order: `DEATHLEAK_RENDER_HIDE_XML_IDS=22`, `22,21`, `22,21,19`, and **unmasked** (unset, not empty). Name both `<variant>.bmp.pre.bmp` (pre-sepia scene) and `<variant>.bmp` (post-sepia/HUD). Run a second hide-22 repeat as a reproducibility control. Do not use old BMPs as pixel baselines. Hash every BMP and log the exact env, binary, viewport, renderer/device, loaded-sprite count, gameplay endpoint, and capture boundary. Verify the instrumented post-sepia hide-22 BMP against a fresh uninstrumented **same-source** build/route before interpreting pre-sepia results; if the readback perturbs subsequent postprocess or state, stop and use a GPU debugger/alternative staged readback instead.

The old report's frozen binary was SHA-256 `ba78aa9a...`; its GDB-measured last-draw camera was `(1248.98621,3400)`, player `(1368.98535,3107.30444)` and its hide-22 BMP matched the primary capture byte-for-byte. Those are **registration targets**, not permission to compare across binaries blindly. Add one read-only GDB draw breakpoint or capture-only log on the new binary to record final frame camera/player and render ordinal; ensure 1280×720, frame index 179 of 180, endpoint player/orbs/health and effect initialization all match within float print precision. If not, recompute all screen controls from the new camera. Hold weather/animation phase via the same fixed route and require repeat hide-22 pre-BMP equality (or locate the first differing pixel and clock cause).

## Registered pixels and alpha-transition control

For measured camera `(1248.98621,3400)`, ID21 projects to approximately `[-664.30,-269.95]..[659.03,555.39]` in 1280×720 top-down coordinates (`port/game/LevelScene.cpp:1250-1271`), with ordinary V inversion (`:1289-1304`). Its immutable 1985×1238 `tree_strip_dark.png` source yields these **nearest-pixel predictions, not GPU samples**:

- **Opaque interior control:** x320, y440–500. Mapped source x≈1476; source alpha is 255 in 7×7 neighborhoods at y440/450/460/500. Existing post-sepia hide-22 → hide-22,21 changes 61/61 pixels here; then hiding ID19 changes 60/61 (`FOREST_ID21_OVERDRAW_GPU_DISCRIMINATOR_2026-10-23.md`). Pre-sepia should retain a broad ID21 contribution; a missing contribution indicates registration or draw-path failure, not an edge-filter result.
- **Alpha-transition control:** x360–395, y440–500, with the crisp seed **x369–376 at y450**. At y450 the source-column mapping crosses alpha 255→partial beginning ~x369 (source x1549), then 0 by ~x376 (source x1560); at y440 onset/end ~x368/375; y460 ~x371/378; y500 ~x383/394. These values came from a read-only scan of `Game/Media/objects/tree_strip_dark.png` under the recorded projection. Inspect a two-dimensional edge ROI `(350,430,405,511)` rather than assuming a vertical straight edge. Keep neighboring full-alpha and zero-alpha pixels in the same ROI; evaluate the incremental hide-22→hide-22,21 RGB delta across the transition against source alpha and the exposed underlay. This control can detect edge width, halos, and alpha-vs-RGB filtering effects without conflating them with the opaque middle band.
- **Transparent/underlay controls:** x400/y450 is safely outside that selected opaque lobe (source alpha 0 at the point, but check the sampled footprint); x640/y440–500 maps near source x1956 and has 7×7 alpha-0 neighborhoods. The prior hide-22→hide-22,21 x640 delta was 0/61, while incremental ID19 removal changed 38/61. x960 lies outside the ID21 quad. Keep these as no-ID21 and positive-ID19 controls, not as edge-width measurements.

Compare pre-sepia images **within the new frozen binary**: A=`hide-22`, B=`hide-22,21`, C=`hide-22,21,19`; compute `A↔B` and `B↔C` changed-RGB counts, bboxes, and per-row RGB/color-distance profiles for x320 and edge ROI. Require A↔B broad x320 delta and B↔C ID19 x320 positive control, zero/near-zero A↔B at x640, then examine whether edge delta tapers over the predicted 7–16 screen-pixel alpha transition. Check unmasked separately to show ID22's masking. Keep BMP orientation/top-down coordinates explicit and retain raw lossless files; do not compare a pre-sepia image to a post-sepia BMP as a filtering test. If the edge behaves differently only after sepia, investigate the postprocess sampling/transform rather than ID21 texture filtering. If pre-sepia edge is abnormal but interior/transparent controls pass, trace the *actual* ID21 SDL draw branch/scale/UV state before changing code.

## Acceptance boundary and next correction

Windows source declares `MIN_LINEAR_MAG_POINT_MIP_LINEAR` with wrap in `BasicLightTexture.fx:10-16`, but `D3DX10CreateShaderResourceViewFromFile` null load-info does not establish populated/bound mip levels (`TextureManager.cpp:50-79`). Linux uses minified `SDL_ScaleModeLinear` on ID21's ordinary full-texture copy, not the repeat-tile branch (`port/linux/SDLRenderer.cpp:209-269`). Do **not** turn this generic mip uncertainty into a defect claim. A renderer/art correction is gated on a lossless original-D3D10 **same camera/viewport/render ordinal/effect phase** pre-sepia ID21 edge packet, including bound SRV range and texture `MipLevels`, and matching Linux packet. The checkout currently has no executable Windows matched-frame route (`FOREST_WINDOWS_MATCHED_FRAME_CURRENT_ROUTE_2026-10-23.md`). If that comparison proves a localized alpha-edge mismatch with the same opaque interior and authored order, the one source-backed candidate correction is a **scoped ID21 sampling/alpha-edge treatment in the ordinary sprite path**, tested by a reversible A/B against the measured Windows edge; retain current behavior otherwise. No ID19/ID21 order, XML alpha, or global mip change is supported now.

## Scope

Read-only audit of source, existing reports/scripts/runtime artifacts and immutable source PNG; this report is the sole write. No build, GPU/Xorg, code/shared status/checkpoint, ID59 work, commit, or push. Dirty tree preserved.

### Copy-pasteable private build skeleton (future owner only)

The following refines the overlay description above by snapshotting all compiled port code, so concurrent dirty-tree changes cannot alter the frozen executable. It has **not** been executed. Run from repository root; use a fresh output directory and do not overwrite a previous evidence packet.

```bash
OUT=port/build/forest-id21-presepia-control-2026-10-23
mkdir -p "$OUT/source/port" "$OUT/source/Game"
rsync -a port/linux port/render port/game port/audio port/compat port/Makefile "$OUT/source/port/"
rsync -a Game/tinyxml "$OUT/source/Game/"
python3 - "$OUT/source/port/linux/main.cpp" <<'PYHOOK'
from pathlib import Path
import sys
p = Path(sys.argv[1])
s = p.read_text()
needle = '            renderer->ApplyScenePostprocess();'
assert s.count(needle) == 1
hook = '\n'.join([
    '            if (!capture.empty() && frame == frameLimit - 1) {',
    '                std::string preError;',
    '                if (!renderer->SaveFrame(capture + ".pre.bmp", preError)) {',
    '                    std::cerr << "Pre-sepia capture failed: " << preError << \'\\n\'; return 1;',
    '                }',
    '            }',
    ''
])
p.write_text(s.replace(needle, hook + needle))
PYHOOK
docker run --rm --mount "type=bind,source=$PWD,target=/src" -w "/src/$OUT/source/port" deathleak-video-capture:bookworm make -j1
cp "$OUT/source/port/build/deathleak-linux" "$OUT/deathleak-linux.presepia"
sha256sum "$OUT/deathleak-linux.presepia"
```

For each variant, the proven GPU command form is `docker run --rm --device /dev/dri --mount type=bind,source=/tmp/.X11-unix,target=/tmp/.X11-unix --mount type=bind,source=/var/run/lightdm/root/:0,target=/tmp/auth,readonly --mount "type=bind,source=$PWD,target=/src" -e DISPLAY=:0 -e XAUTHORITY=/tmp/auth -e SDL_VIDEODRIVER=x11 -e SDL_RENDER_DRIVER=opengl -e SDL_AUDIODRIVER=dummy -e DEATHLEAK_RENDER_HIDE_XML_IDS=22 -w /src deathleak-video-capture:bookworm ./port/build/forest-id21-presepia-control-2026-10-23/deathleak-linux.presepia --play Game/XmlFiles/levels/forest_exploration_1.xml --script R:180 --frames 180 --fixed-dt 0.0166667 --capture port/build/forest-id21-presepia-control-2026-10-23/hide-22.bmp`; substitute `22,21` and `22,21,19`, and omit the hide-ID `-e` entirely for unmasked. Place timeouts and logs around each run as in the cited existing capture script. `SaveFrame` names the pre image `hide-22.bmp.pre.bmp` by construction.

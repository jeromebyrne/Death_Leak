# Forest middle band: current-frame physical-GPU causal gate — 2026-10-23

## Verdict

At the frozen current dirty-tree executable's forest-1 `R:180` frame, **IDs 19 and 17 each measurably contribute to the composited middle-band transitions where exposed**. ID19's visible hide delta begins at x640/y462 and x960/y454; ID17's begins at x640/y506 and x960/y527. Their isolated onset rows and immutable PNG alpha support agree with the measured camera. At x320 both IDs have isolated texture onsets, but hiding either changes no pixel in y380..559: another later layer occludes their contribution there. Thus the two authored strips are causal for exposed portions of this current Linux frame, not necessarily every apparent full-scene line or all columns. No matched original-Windows state exists; this is not a Windows parity or regression verdict.

## Frozen route, camera, and GPU proof

Repository HEAD `22950f87a0c37207f234b859506759ec0c3f3ca3`, current dirty tree snapshot at `port/build/forest-middle-band-current-frame-gpu-2026-10-23/tree-status.txt`. Shared source/build files were not edited. Copied `port/build/deathleak-linux` to `port/build/forest-middle-band-current-frame-gpu-2026-10-23/deathleak-linux.frozen`; before/after frozen and shared SHA-256 all equal `70739562d96395f4731674b0ad64a356e6d94f20d18d306bed9ff40269ebbacb` (see `binary-before.sha256`, `hashes-after.sha256`). All five 1280×720 lossless BMPs and logs are in that ignored build directory. Baseline BMP SHA-256 `ff755ee685accafbdfc5990b8f292c9ac61127fae1bd0e238c14ddbdfcf94df6`.

Sequential captures in one exclusive physical-Xorg container, no offscreen/software fallback:

```sh
docker run --rm --name deathleak-middle-band-exclusive --network host \
  --hostname jerome-Linux-MacBookPro --device /dev/dri \
  -v "$PWD:/src" -v /tmp/.X11-unix:/tmp/.X11-unix \
  --mount type=bind,source=/var/run/lightdm/root/:0,target=/xauthority,readonly \
  -e DISPLAY=:0 -e XAUTHORITY=/xauthority deathleak-video-capture:bookworm sh -eu -c '
    cd /src
    out=port/build/forest-middle-band-current-frame-gpu-2026-10-23
    for mode in baseline hide-17 hide-19 only-17 only-19; do
      unset DEATHLEAK_RENDER_HIDE_XML_IDS DEATHLEAK_RENDER_ONLY_XML_IDS
      case "$mode" in
        hide-17) export DEATHLEAK_RENDER_HIDE_XML_IDS=17;;
        hide-19) export DEATHLEAK_RENDER_HIDE_XML_IDS=19;;
        only-17) export DEATHLEAK_RENDER_ONLY_XML_IDS=17;;
        only-19) export DEATHLEAK_RENDER_ONLY_XML_IDS=19;;
      esac
      SDL_VIDEODRIVER=x11 SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy \
        "$out/deathleak-linux.frozen" \
        --play Game/XmlFiles/levels/forest_exploration_1.xml \
        --script R:180 --frames 180 --fixed-dt 0.0166667 \
        --capture "$out/$mode.bmp" > "$out/$mode.log" 2>&1
    done
  '
```

Every log reports `video_driver=x11 name=opengl ... accelerated=1 software=0 ... mode=strict-accelerated`, `OpenGL device vendor=nouveau renderer=NVE7`, initialized GPU `Sepia.fx`, and 71 loaded original sprites / zero missing. Each gameplay endpoint has player `(1368.99,3107.3)`, five orbs, health 25, balance 5, zero shots, grounded; all printed simulation fields agree. Complete endpoint lines are **not byte-identical** because `elapsed=` is wall time (baseline 4.04597 s, hide17 3.96815 s, hide19 3.9606 s). This disqualifies literal full-line equality but not the fixed-frame/state attribution.

Exact camera was obtained without source/build edits by a read-only GDB breakpoint on the frozen executable's `LevelScene::Draw`, printing its `cameraX/cameraY/playerX/playerY` arguments on each draw. The 180th/final draw is `cameraX=1248.98621`, `cameraY=3400`, player `(1368.98535,3107.30444)`. The debugger run used the identical route and physical Xorg display; its final `gdb-baseline.bmp` hash is **byte-identical** to `baseline.bmp`. The temporary Xauthority copy used for host debugger access was removed after use; debugger command/output remain as `camera.gdb`, `camera-gdb.log`, and `gdb-stdout.log`. This equality ties the logged camera to the captured baseline frame, rather than inferring camera X from player X or an older frame.

## Line coordinates and source-backed attribution

All coordinates below are top-down pixel coordinates in the 1280×720 BMP. Pillow decoded RGB comparison of baseline vs each draw-hidden image uses any-channel nonzero differences. ID19 changes 38,279 pixels, bbox `[194,440,1280,557)`; ID17 changes 32,091 pixels, bbox `[0,505,1280,632)` (half-open bounds). The first changed pixels in the middle-band columns and examples are:

| Column | Hide ID19: first composited delta | Hide ID17: first composited delta | Isolated texture colored rise (19 / 17) |
| --- | --- | --- | --- |
| x320 | none in y380–559 | none in y380–559 | y451 / y520 |
| x640 | y462; e.g. baseline `(77,66,47)` vs hidden `(79,68,48)` | y506; baseline `(49,42,30)` vs hidden `(59,51,36)` | y463 / y505 (stronger ID17 jump at y515) |
| x960 | y454; baseline `(95,82,58)` vs hidden `(96,82,58)` | y527; baseline `(69,59,42)` vs hidden `(90,78,56)` | y457 / y528 |

At x640 ID19's delta continues chiefly y462–515 (with small gaps); ID17's begins at y506–507 and resumes y514 onward. At x960 ID19's spans y454–527; ID17's begins y527. This establishes composited causation at those exposed line portions. It does not imply each later pixel in the band is a pure strip pixel, since layers overlap and sepia postprocess acts on the scene.

The read-only command reconstruction from `LevelScene.cpp:1185-1312`, XML IDs 19/17, `RepeatX.h`, and the immutable 734×1024 RGBA textures predicts ordinary light-texture copies, white tint/opacity 1, repeat-X, horizontal flip and SDL vertical flip. ID19 (depth 5500) draws before ID17 (depth 5000). At measured camera Y=3400, destination tops are approximately y401.43 / y464.83. At measured X=1248.98621, source-column point estimates and alpha support, scanning each PNG column bottom-up, are:

| ID | screen x | repeat tile / approx source x | first alpha>0 y | alpha≥128 y |
| --- | ---: | --- | ---: | ---: |
| 19 | 320 | 4 / 702 | 440.10 | 450.76 |
| 19 | 640 | 4 / 222 | 459.43 | 463.43 |
| 19 | 960 | 3 / 476 | 453.43 | 457.43 |
| 17 | 320 | 4 / 448 | 519.49 | 520.16 |
| 17 | 640 | 3 / 702 | 514.16 | 514.83 |
| 17 | 960 | 3 / 222 | 527.49 | 527.49 |

The x640 ID17 isolated image has a dark preliminary change at y505 and a stronger colored jump at y515; PNG alpha≥128 at ~514.8 explains the latter. Point sampling is an approximation to SDL linear filtering, not an asserted exact GPU texel. The measured hide masks, isolated onset profiles, camera, order, and authored alpha jointly identify IDs 19/17 as causes of the exposed current-frame transitions. At x320, absence of a composited hide delta despite isolated onsets identifies occlusion by another layer, without naming that layer from this experiment.

## Remaining gate

There is no matched original-Windows screenshot/trace at camera `(1248.98621,3400)`, the same viewport, simulation frame, and effect phase. It remains unknown whether these authored transitions match the original or constitute a Linux-specific visual difference. The current-frame GPU causal question is closed for the exposed x640/x960 portions; any x320 full-scene line requires isolating the later occluding layer, not attributing it to ID17/19 solely from alpha geometry.

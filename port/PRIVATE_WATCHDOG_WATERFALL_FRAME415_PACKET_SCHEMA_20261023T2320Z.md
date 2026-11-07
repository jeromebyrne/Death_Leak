# Waterfall first-reveal frame 415: atomic packet schema (2026-10-23 23:20Z)

## Decision and available evidence

**No parity verdict or patch.** The saved corrected Linux `frame415.bmp` is the first partially revealed, gameplay-active Waterfall image in the zero-based deterministic Forest-1 `--script B:1800 --fixed-dt 0.016666667` route, but is **not an atomic state packet**. Its file is `port/build/private-loading-fade-correction-20261023t2218z/frame415.bmp`: 1280×720, 32-bit uncompressed BMP, 3,686,522 bytes, SHA-256 `d87e39bc71c42145dee1444faa2985f787a0ee3a6bd566144af28edc694fa457`. The saved `cpu-observer.log` and `gpu-observer.log` in that directory both have `FADE_OBSERVER frame=415 entry=3 exit=3 gameplay=1 weather=1 scene=1 player=1 alpha=170 hold_exit=0 time=0.0666667`; frame 414 has alpha 255 and all four update flags 0. The image's companion `frame415.log` records strict X11/OpenGL nouveau/NVE7, Waterfall installation, and **terminal** state, not draw-time camera/pose/clock. Another older physical-GPU `frame415.bmp` has a different hash (`f6b8786a...`); never silently mix these runs. The separate 30-fps MP4 is not ordinal 415.

## Minimal v1 pair: field names and exact read points

Capture one **source-built original D3D10** first partially revealed Waterfall frame and one Linux frame at matched *state*, not assumed equal frame numbers. Each image/JSON pair must share a render ordinal and image hash; absent draw-state fields are `unavailable` with reason and make the packet not matched-state eligible. This is the smallest useful packet for classifying whether the first residual is transition, camera/scene, dynamic effect, or HUD/overlay:

| Field group (sidecar keys) | Required contents and source/read seam |
|---|---|
| `identity` / `image` | `engine` (`source_built_original_d3d10` or `linux_port`), EXE SHA-256 and revision, level XML/content-manifest hashes, input/initial-save recipe, `renderOrdinal` (and `updateOrdinal` if distinct), `captureStage=post_hud_pre_present` or Linux `post_overlay_pre_endframe`, image filename/SHA-256, decoded native width/height/pixel format/color-space, actual viewport, postprocess flag. Windows: hook after `UIManager::Draw`, before `Present` (`Game/src/program.cpp:293-322`), query actual backbuffer/viewport. Linux: `SaveFrame` is after splash and before `EndFrame` (`port/linux/main.cpp:1549-1552`); existing CLI image is this stage, but has no same-ordinal sidecar. |
| `transition` | Active level path; loading-screen stack/phase; pause-at-update-entry and pause-at-draw; loading alpha **submitted at draw** (raw float and quantized tint if applicable); hold-exit flag; whether gameplay/weather/scene/player update ran. Windows: snapshot `UIGameLoading::Update/Load/FinishedLoading` (`Game/src/UI/UIGameLoading.cpp:60-129`), plus ordered `UIManager::Draw`; Linux: existing observer fields, `loadingPhaseOnEntry`, `loadingPhase`, `loadingPhaseTime`, `loadingHoldExitedThisFrame`, tint at `port/linux/main.cpp:568-576,686-730,1535-1547`. Do not equate Windows `mBGSprite` float alpha to Linux byte 170 without recording both. |
| `timing_camera_player` | Effective update dt, total simulation time, prior rendered update dt, actual submitted world/view/projection matrices, camera stored XY/zoom/shake and prior rendered XY, drawn player XY/velocity/facing/body+arm clip/frame. Windows: loop/timing (`program.cpp:148-177`), `Game::Draw` matrices (`Game.cpp:354-369`), player draw (`Character.cpp:1888-1907`). Linux: `dt`, `shaderClock.Draw()` snapshot, `precedingRenderedModifiedDelta`, `drawCameraX/Y`, `motion` and player painter submission (`port/linux/main.cpp:550-576,1395-1477`). Record matrices, not just camera position: original shake can make them differ. |
| `dynamic_draw` | Submitted vertex/noise/foliage time, pixel-wobble time **and reverse direction**, weather/foliage multiplier and rain/lightning state, plus ordered actually submitted scene draws in the reveal ROI: object ID/type/depth, visibility/cull decision, texture identity/hash, effect, transform/UV, opacity, blend/sampler; at minimum ID46 and whichever curtain/foliage/fog sprites intersect the registered ROI. Windows read at `Game::Draw` immediately after pixel-wobble branch (`Game.cpp:371-405`) and object draw predicate (`GameObjectManager.cpp:302-355`); Linux read `shaderFrame`/weather and `LevelScene::Draw` submission (`port/linux/main.cpp:1474-1477`, `port/game/LevelScene.cpp:1360-1437`). XML alone is not an actual draw list. For nondeterministic effects record RNG/history or a declared matched freeze. |
| `ui_order` / `pixels` | Ordered film, bars, HUD meters **displayed** fill, orb text/count, optional interactable icon, then loading splash: texture, rectangle, alpha and blend for each; raw health/focus/orbs and active modal/UI stack; any port-only FPS text flag. Windows `UIManager::Draw` (`Game/src/UI/UIManager.cpp:174-229`) and `UIGameHudScreen.cpp:91-170,226-240`; Linux `Hud::Draw` and `port/linux/main.cpp:1497-1547`. Native lossless final pixels are mandatory. To attribute the first divergent pass without adding a second run, include optional same-ordinal lossless pre-UI/post-scene and post-HUD/pre-splash readbacks/crop hashes; these are diagnostics, not substitutes for final image. |

`renderOrdinal` is per binary/run, **not** assumed to be Linux 415 on Windows. Select the Windows render whose loading screen first has `0<alpha<1` **and** gameplay resumed, then align on content identity, level entry, drawn player/camera matrices, HUD, clocks and ordered draws. An original backbuffer screenshot without private state is a qualitative image only. The existing Windows atomic-ordinal design in `PRIVATE_WINDOWS_FOREST1_ATOMIC_ORDINAL_IMPLEMENTATION_GATE_20261023_D539FA.md` can be reused at this Waterfall target; no Windows runtime/EXE is available in this workspace (`PRIVATE_POST_FADE_ORIGINAL_PACKET_ACQUISITION_GATE_20261023T2243Z.md`).

## Non-GPU extraction now; missing instrumentation later

Run this from repository root. It reads only the **saved** corrected files; no game, build, Docker, display, or GPU invocation. It verifies byte identity and extracts the already-instrumented 414/415/416 observer rows. It does **not** manufacture missing camera/draw sidecar fields:

```sh
python3 - <<'PY'
from pathlib import Path
import hashlib, re, struct
base = Path('port/build/private-loading-fade-correction-20261023t2218z')
p = base/'frame415.bmp'; b = p.read_bytes()
assert b[:2] == b'BM' and len(b) == 3686522
w,h,planes,bpp = struct.unpack_from('<iiHH', b, 18)
assert (w,h,planes,bpp) == (1280,720,1,32)
sha = hashlib.sha256(b).hexdigest()
assert sha == 'd87e39bc71c42145dee1444faa2985f787a0ee3a6bd566144af28edc694fa457'
print('image', p, 'bytes', len(b), 'size', (w,h), 'bpp', bpp, 'sha256', sha)
for name in ('cpu-observer.log','gpu-observer.log'):
    rows = [s for s in (base/name).read_text().splitlines()
            if re.match(r'FADE_OBSERVER frame=41[456] ', s)]
    assert len(rows) == 3, (name, rows)
    print(name, *rows, sep='\n  ')
PY
```

For a future atomic Linux packet, an isolated owner should add **opt-in** one-frame in-memory snapshots at the existing update/draw read points above and serialize alongside `SaveFrame` for `--frames 416` (`frame==frameLimit-1`), writing image first, JSON last with ordinal/hash/dimensions validation. Do not use the terminal `Simulation ended` line as frame state. Windows owner uses the corresponding one-shot post-UI/pre-Present staging readback and JSON-last transaction; only compare after both packets validate and the same-state predicate passes. Register a static ROI away from HUD/overlay and inspect the first differing draw/state plus localized residual. This report requests no source edit and does not open a new visual defect.

Scope: saved evidence/source read and this unique report only. No GPU, build, source/test/XML, shared checkpoint/status, index, or commit touched; other owners retain FPS-default capture and original-runtime acquisition.

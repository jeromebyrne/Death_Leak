# Forest screenshot 14: reproducible Linux camera-state search (2026-10-22)

## Bounded result

**Best next physical-capture candidate: forest-1, `--script R:98 --frames 98 --fixed-dt 0.0166667`.** This is a deterministic, source-calculated camera/composition search state, **not an observed GPU frame or a matched original Windows state**. It should shift the static branch, ID117 pickup and raft approximately **64 backbuffer pixels right** compared with the existing R:110 Linux still, within the approximate 50–70 px left-of-reference observation in `FOREST14_LINUX_MATCH_CAPTURE_2026-10-22.md`. R:97–100 bounds that hypothesis (roughly 69–53 px rightward). The player will also be 96 world units / 64 backbuffer pixels farther left at R:98, so its pose/location and 13-orb HUD cannot be matched by this camera adjustment alone.

The saved Steam `../store_media/screenshots/14.jpg` is a 1920×1080 JPEG with no original camera/player telemetry, backbuffer provenance, simulation frame, orb collection, animation/weather phase, or lossless source. The R:110 Linux image is 1280×720; its log records player `(808.986,3135.97)`, 3 orbs, health 25, but **does not log camera coordinates**. Thus neither an exact original camera state nor exact visual parity can be recovered from these artifacts. Treat screenshot 14 as a qualitative framing target only.

## Evidence and reproducible calculation

- `Game/XmlFiles/levels/forest_exploration_1.xml:3` authors camera `(0,3400)`, `xLag=15`, `followx=true`, `followy=false`, zoom 1, and bounds `[-2300,3000]` horizontally. `port/linux/main.cpp:323,641-680` initializes from those properties, clamps to the 1920×1080 logical view, follows **pre-motion** player X by `(playerX-cameraX)/15` each update when no enemies exist, and leaves Y=3400. The level bounds do not clamp this candidate interval. `port/game/LevelScene.cpp:978-980` projects ordinary objects with `(worldX-cameraX)*2/3` to 1280×720.
- Authored landmarks are fixed: ID117 katana/pickup `(1167.985,3350)`; lower branch ID30 `(964.685,3254)`; upper branch ID29 `(1012.785,3773)`; raft IDs 11/57/58 at X `267.985/640.085/-132.215` (`forest_exploration_1.xml:191,442,457,928,943,1614`). `FOREST_REFERENCE_AUDIT.md` identifies the same grouping in Steam screenshot 14; its X≈700 is an *estimate*, not telemetry.
- `port/game/PlayerMotion.cpp:7-11,49-67` gives the unupgraded right-input acceleration of 1 world unit/update in each of updates 1–8, then constant 8/update. Starting authored player X is `-43.014648` (`forest_exploration_1.xml:34`). Therefore R:110 predicts X=808.985 (agreeing with the saved log to rounding). Recursing the camera equation from Xcamera=0 gives these **calculated, not logged** states, assuming the same no-enemy, collision-free R route:

| Final update | Player X | Camera X | Static-landmark shift from R:110 at 1280×720 |
| --- | ---: | ---: | ---: |
| R:97 | 704.985 | 585.231 | +69.2 px |
| **R:98** | **712.985** | **593.214** | **+63.9 px** |
| R:99 | 720.985 | 601.199 | +58.6 px |
| R:100 | 728.985 | 609.185 | +53.3 px |
| R:110 (saved capture) | 808.985 | 689.085 | baseline |

The recurrence is: for update `n`, `C_n=C_(n-1)+(X_(n-1)-C_(n-1))/15`, then advance the player by `min(n,8)`; start `C_0=0`, `X_0=-43.014648`. No source or game run was used for this calculation. The output predicts a coherent X translation for **static** branch/pickup/raft geometry; it does not predict pixel landmarks inside rotated/occluded textures, parallax backgrounds, changing orbs, or pose.

## Capture suggestion for the GPU/build lane owner only

Use the existing physical-X11 Docker capture recipe in `FOREST14_LINUX_MATCH_CAPTURE_2026-10-22.md`, changing only `--script R:110 --frames 110` to `--script R:98 --frames 98`, writing to a new output/log name. If necessary, capture R:97 and R:100 as endpoints; **at most three frames** before reviewing registered static landmarks. Reuse the owner's current built binary only after verifying its build/HEAD provenance. Keep `--play Game/XmlFiles/levels/forest_exploration_1.xml`, `--fixed-dt 0.0166667`, physical `/dev/dri`, X11 OpenGL, and `SDL_AUDIODRIVER=dummy`; do not use `--level-preview` or a software renderer. `--camera-script` is **simulation-only** (`port/linux/main.cpp:258-275`), so it cannot directly force a physical screenshot camera. Do not alter XML, renderer geometry, or source to make the landmarks appear aligned.

On each image, normalize the Steam JPEG to 1280×720 and register **ID117 pickup, lower/upper branch silhouette, trunk edge, and raft segment junctions** before comparing atmospheric haze or sprite color. Record player/health/orb differences separately. If R:98 is closer, preserve its exact script/frame/HEAD/log/capture provenance; if static landmarks disagree nonuniformly, investigate transform/occlusion rather than fitting one horizontal shift. For a defensible exact comparison, an original Windows forest-1 PNG plus actual camera X/Y, player X/Y, viewport, zoom, frame/time, pickup/orb and weather/particle states remains necessary.

Scope: read-only inspection/calculation plus this report. No source, tests, build, Docker run, GPU/display use, shared checkpoint, or unrelated change touched.

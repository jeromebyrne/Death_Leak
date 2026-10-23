# Fresh-save forest-1 world zoom implementation — 2026-10-23

## Result and scope

Implementation commit `ec4a62c499aeeb215abb395afd6e61493aed3645` was pushed to `origin/feature/linux-renderer-foundation`; `git ls-remote` returned that exact SHA. This continues caller `2c7bb59` and animation seam `8d1cd1a`. The intro route remains opt-in: `--fresh-save-intro --play Game/XmlFiles/levels/forest_exploration_1.xml`, without `--load-save`. It does not reroute New Game from grass or alter continuation saves.

`main.cpp` sends the caller's finite effective zoom to the renderer only while `hasPulledSword` is false. `Renderer.h` exposes a default-no-op `SetWorldZoom` so existing CPU spy renderers remain source-compatible, plus the deterministic `ProjectWorldRect` projection. SDLRenderer resets zoom at BeginFrame and before sepia; only the scene/painter pass is projected. Ordinary, rotated, UV/parallax, wobble and damage-burst sprites use projected destinations. Repeat-X tiles retain source-world generation/clipping then project each tile, preventing gaps at 0.75. Foliage sway retains original world displacement/phase and projects completed vertices. World FillRect projects; postprocess, film, bars, HUD and overlays stay screen-aligned. Source rectangle, tint, UV clocks, and texture coordinates are unchanged. Zoom 1 follows the unchanged coordinates.

## CPU/build evidence

- Private C++17 `zoom_cpu.cpp` asserted identity at 1, center projection of a representative quad at 0.75, invalid/nonpositive fallback, and orthographic extents `1920×1080 -> 1440×810`; passed. This is a projection helper check, not a full caller camera-spy test. Caller phase timing is directly evidenced by the live scripted captures below.
- Private Docker build using `deathleak-linux-build:bookworm` compiled full Linux executable, exit 0, with existing tinyxml warnings only. Build log: `port/build/fresh-save-intro-world-zoom-2026-10-23/build.log`. Executable SHA-256: `0c433f75d6899839ab04c536a4a7dcce20073421dbc1ce004dea2d28a34dd46c`. It included the pre-existing **disabled** raft experiment in the working source, but the committed index excluded it.
- `git diff --cached --check` passed before commit. After commit `git diff -- port/linux/SDLRenderer.cpp` remains precisely the pre-existing 76-insertion/1-deletion (77-line) opt-in raft-mip experiment. Neither its branch nor methods appear in the intro commit. Other pre-existing dirty/untracked files were untouched.

## Strict physical GPU evidence

Docker capture used the real Xorg `DISPLAY=:0` socket and LightDM Xauthority, `/dev/dri`, `SDL_VIDEODRIVER=x11`, `SDL_RENDER_DRIVER=opengl`, strict accelerated mode (no test/software fallback), and GPU Sepia. Every run reported `video_driver=x11 name=opengl accelerated=1 software=0`, `OpenGL device vendor=nouveau renderer=NVE7`, 71 level sprites, zero missing assets, and the fresh forest-1 path. Frames are lossless 1280×720 PNGs and per-run logs in `port/build/fresh-save-intro-world-zoom-2026-10-23/`. New-session `--play` without a save was used each run; no continuation file or existing save was modified.

The script runs fixed `0.0166667` delta, capped by the game at 1/60 s. `I` is held Interact; `.` is release; `R` is rightward gameplay. Each checkpoint replays the same prefix from a new fresh session:

| Frame file | Script prefix | Expected state from code; observed live milestone |
| --- | --- | --- |
| `intro-000.png` | `.:1` | initial phase 1, zoom 1 |
| `intro-120.png` | `I:120` | intermediate held zoom about 0.90, controls gated |
| `intro-304.png` | `I:160,.:1,I:143` | second hold saturates at 0.75 without crossing the continuous 2.75 s threshold |
| `intro-334.png` | prior + `.:30` | released/recovering zoom about 0.80; hold reset |
| `intro-500.png` | prior + `I:166` | threshold crossed, pull logged at frame 499, zoom restored to 1 |
| `intro-600.png` | prior + `R:100` | pull completed at frame 539, active control moved player to `(408.985,3229.79)` and collected 2 orbs |

A separate same-state, same-script, frame-304 control binary forced `SetWorldZoom` to identity **only in `/tmp`**, with no repository change; it ran the same physical GPU and captured `control-304.png`. Control binary SHA-256: `2b3ce4e7ad53db85a8bd233628c54a26a77bc271eccad6113be3802434395bdb`. Against `intro-304.png`, 685,671 of 691,200 scene pixels in y=80..619 changed; top opaque bar y=0..32 and bottom opaque bar y=687..719 each changed **zero** pixels. This demonstrates a scene-only projection effect and stable screen bars, not exact original Windows image parity. The captures and logs are local ignored build artifacts, not remote release assets. The original Windows same-state frame was not available. No explicit per-frame `glGetError` instrumentation was added; logs show no emitted GL/SDL failure.

## Ownership/release and limits

Intro implementation files `port/linux/SDLRenderer.cpp` (only committed zoom hunks), `port/linux/main.cpp`, and `port/render/Renderer.h` are released for subsequent owners. The physical Xorg `:0` GPU/capture lane is released; no capture process remains. The unrelated unstaged raft experiment remains owned by its separate gate, uncommitted and unpushed. This report does not claim Windows visual parity, Jerome acceptance, complete intro audiovisual parity, or that a prior ordinary-gameplay clip used the fresh-intro route.

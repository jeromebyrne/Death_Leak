# Current Forest→Waterfall fade — physical GPU presentation gate (2026-10-23 22:58Z)

## Verdict

**PASS for the bounded Linux presentation boundary.** Exact zero-based, lossless GPU frame captures from active Forest gameplay show progressive fade-in, five contiguous fully opaque frames spanning destination installation and hold exit, then the first two revealing Waterfall frames with HUD present. No one-frame Forest/Waterfall exposure or wrong-scene flash is visible across captured frames 410–416. This is a physical-GPU/current-binary gate, not a matched-original-Windows parity claim or human delivery acceptance.

## Provenance and capture

- Repository HEAD: `64b2b09a2e5b34ab325df928a077a31e5893b97f` (`Correct loading fade pause and opaque hold boundary`); `port/linux/main.cpp` clean. Used the already-built production renderer/game executable `port/build/private-loading-fade-correction-20261023t2218z/deathleak-linux`, SHA-256 `f346a5db9941752b29aef5bbd64c0333e31b20693a43859b97e3eae56074e9e0`. Its existing handoff establishes this exact HEAD correction plus preexisting dirty `SDLRenderer.cpp`; this lane did not rebuild or alter source/build output. Renderer source mtime predates binary; `main.cpp` current clean content is HEAD (its filesystem mtime is later than binary). Accordingly this is **current HEAD fade logic with the established dirty-renderer binary**, not a clean-tree binary guarantee.
- Exclusive lane preflight: physical Xorg `:0` (PID 1360) was the only display process; no Death Leak game, FFmpeg, capture, or competing GPU container. Unrelated `html-mobile-games` container was present but not a Death Leak capture. All ten Docker runs were serial. Postflight showed no Death Leak game/capture process or container.
- Each run used the established `deathleak-linux-build:bookworm` image with `/dev/dri`, physical `DISPLAY=:0`, LightDM Xauthority, SDL X11/OpenGL strict-accelerated mode and dummy audio. Source tree was bind-mounted read-only, output only to `/tmp/deathleak-private-current-fade-gpu-20261023t2258z/`. Reproduction template from repository root (substitute F):

```sh
docker run --rm --network host --hostname jerome-Linux-MacBookPro --device /dev/dri \
  --mount type=bind,src="$PWD",dst=/src,readonly \
  --mount type=bind,src=/tmp/deathleak-private-current-fade-gpu-20261023t2258z,dst=/out \
  --mount type=bind,src=/tmp/.X11-unix,dst=/tmp/.X11-unix \
  --mount type=bind,src=/var/run/lightdm/root/:0,dst=/tmp/deathleak.Xauthority,readonly \
  -e DISPLAY=:0 -e XAUTHORITY=/tmp/deathleak.Xauthority \
  -e SDL_VIDEODRIVER=x11 -e SDL_RENDER_DRIVER=opengl -e SDL_AUDIODRIVER=dummy \
  -w /src deathleak-linux-build:bookworm \
  /src/port/build/private-loading-fade-correction-20261023t2218z/deathleak-linux \
  --play Game/XmlFiles/levels/forest_exploration_1.xml --script B:1800 \
  --frames $((F+1)) --fixed-dt 0.016666667 --capture /out/frame${F}.bmp
```

- Every corresponding `frameF.log` reports `video_driver=x11 name=opengl ... accelerated=1 software=0 ... mode=strict-accelerated`, `OpenGL device vendor=nouveau renderer=NVE7`, and `Sepia.fx GPU postprocess initialized (scene before HUD)`. Forest reports 71 original sprites, zero missing/unsupported assets. Trigger is frame 398, Forest-1→`waterfall_top.xml`; frame 410 still ends in Forest, frame 411 log reports `Entered .../waterfall_top.xml`, and frames 415 onward end in Waterfall. Thus this is **physical nouveau/NVE7**, not llvmpipe/dummy video or a CPU-only observer.

## Frame/pixel findings

All primary `frameF.bmp` images are 1280×720, 3,686,522 bytes. `contact.jpg` and `reveal.jpg` in the same evidence directory are review-only contact sheets made from those BMPs. RGB means are entire viewport, not HUD-only estimates.

| F | Scene/phase | Mean RGB | Pixel observation |
|---:|---|---|---|
| 397 | active Forest, pre-trigger | 56.39, 51.06, 41.42 | Forest trees/bridge; health/focus/orb HUD and FPS visible |
| 405 | Forest fade-in | 23.43, 21.22, 17.21 | Same Forest composition and HUD darken together |
| 410 | outgoing opaque | 0, 0, 0 | all RGB zero |
| 411 | Waterfall installed under opaque | 0, 0, 0 | all RGB zero |
| 412–413 | opaque hold | 0, 0, 0 each | all RGB zero |
| 414 | hold-exit frame | 0, 0, 0 | all RGB zero; no premature reveal |
| 415 | first gameplay FadeOut | 25.75, 23.00, 18.02 | Waterfall entry curtain/bridge, HUD/orb `7` and FPS visible under dark overlay |
| 416 | continuing FadeOut | 28.56, 25.63, 20.33 | same Waterfall family; HUD persists |
| 423 | revealed Waterfall gameplay | 43.54, 40.00, 33.58 | waterfall-top foreground, HUD/orb `7` and FPS visible |

Frames **410, 411, 412, 413, 414** are byte-identical BMPs (SHA-256 `e26ef58a562d45a8f8bf40aec887e913d93ac5b82bb5bfc3223ad44672d38998`); all 2,764,800 RGB channel bytes per image are zero, and each adjacent RGB difference bounding box is empty. Frame 415 is nonblack (2,493,536 nonzero channel bytes; BMP SHA-256 `d87e39bc71c42145dee1444faa2985f787a0ee3a6bd566144af28edc694fa457`) with visual Waterfall identity; frame 416 continues that identity. This contiguous boundary rules out a **captured one-frame wrong-scene flash at installation/hold/reveal**. The five black frames also cover HUD, proving the loading overlay is final over both scene and HUD rather than a black scene behind a visible HUD.

The scene-before-HUD conclusion is supported jointly by the live GPU initialization log, visible colored HUD over sepia scene at frames 397/405/415/416/423, and current draw order in `port/linux/main.cpp` (scene/postprocess, film and HUD, then loading overlay at approximately lines 1500–1550). Pixels alone cannot uniquely establish every internal render-pass ordering, but they show the intended presented composite and no HUD drawn on top of the opaque hold.

## Limits and next action

Exact-ordinal BMPs are fresh-process captures of the final frame of each run, not a single continuous scan-out recording; GPU shader/wall-clock animation can vary between runs, so nonblack frame means are not an invariant temporal interpolation test. The prior continuous active-gameplay MP4 in `port/build/private-corrected-fade-active-gameplay-20261023t2231z/` separately covers the same transition perceptually but at 30 fps, not every simulation ordinal. No original Windows matched-frame packet or user visual acceptance is established. Existing unrelated dirty files and the index were preserved; only this unique report was written inside the repository.

**Concrete next action:** coordinator can treat the Linux fade boundary as visually gated and proceed to private review/delivery of the already-captured corrected gameplay clip, while keeping matched-Windows visual parity and user acceptance explicitly separate. No fade code change or repeat GPU run is indicated by this evidence. Physical GPU/display lane released.

# Film-frame residency presentation seam — 2026-10-26

Base: clean detached remote `feature/linux-renderer-foundation` at `7fd76c7dd46a9462f4246fdb6c6ef4573abe580d`. Sole source/build/physical-NVE7 owner session `agent:main:subagent:d4b7ba19-2117-48ac-8e6e-23e3e25667b4`. Primary dirty source and unrelated untracked files were not build inputs.

## Bounded change

The game HUD's ordinary draw no longer calls `loadFilmFrames()` or any texture loader. `FilmFrameResidency::Prepare()` runs on the fixed-update side before the catch-up/presentation branch; the four authored paths and load order are unchanged. The existing `sea_2` startup preload and boat-transition preload remain on their pre-draw paths. `Frame() const` returns an already prepared texture ID. The original frame-zero success sentinel is retained: a failed first frame retries all four on the next update, while a successfully loaded first frame suppresses further loads even if another frame failed. This intentionally changes failed-load retry timing from draw to update and loads ordinary gameplay art earlier within its first active step; it is not a general rendering-purity change. `simulateOnly` does not touch the GPU loader. The 11-FPS film selection and authored opacity/order are untouched.

## Remaining draw/capture boundary

The ordinary draw still reads scene state and traverses `scene.Draw()`, appends combat painter items and damage burst, reads weather lightning alpha/sprite and foliage sway, invokes rain draw, samples live intro prompt input, and prepares/draws HUD from combat/vitals. These are not immutable full-scene packets; renderer/scene internals may mutate. No genuine update-free whole-frame path is claimed. Capture ordering is unchanged: `SaveFrame(...prehud.bmp)` remains immediately after scene postprocess and before the film/HUD; the normal/atomic frame readback remains after loading overlay and debug UI, before `EndFrame`, with packet JSON committed after image/hash creation.

## Verification

- C++17 fake-renderer test checks four exact paths/order, cached prepare, original first-frame retry sentinel, and 100,000 repeated const frame reads plus sprite submissions without additional loads.
- Initial and final restored-source native-clock isolated Docker production builds passed; final binary SHA-256 `50394a165d816d58ac8b669cfad568dfda3997619553df102c2e3507b71e17f2`. `git diff --check` passed. Strict serial physical X11/OpenGL Mesa nouveau NVE7 (`accelerated=1 software=0 vsync=1`) 1280×720 fixed-60 Forest-1 frame-100 A/B/A full post-HUD BMPs matched byte-for-byte, SHA-256 `86a0c99e07c824a9c9d36ef31d0a3a55fce651a0153228a70f3c6c724b804931`, zero changed pixels. Gameplay endpoints matched; elapsed wall times differ and were not used as a parity oracle. Evidence is retained under ignored primary `port/build/film-residency-20261026/`.

The physical test uses identical *test-only* frame-indexed film clocks in baseline and candidate, removed before final native-clock build/commit. Synchronized 60-Hz parity cannot prove arbitrary native-clock phase identity, high-Hz output, 8K, sustained FPS, or release acceptance.

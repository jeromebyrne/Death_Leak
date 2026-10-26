# Later-scene post-HUD sealed-replay acceptance — 2026-10-26

Base: authoritative live `feature/linux-renderer-foundation` at `e70a54e0760ad4142d124a5e26aeba127ba4e7c2`; isolated clean worktree. The primary divergent dirty checkout was not built or modified except append-only completion checkpoints.

## Scope

`port/tests/posthud_later_scene_replay.sh` starts from Forest-1 with the uninterrupted ordinary-control route from `port/tests/waterfall_cave_route.sh`: `R:1100,Q:40,R:640,E:1500,R:1300`. It captures the full 32-bit 1280×720 post-HUD frame at cave-6 frame 1780, Hot Springs frame 3037, underwater cave-4 frame 3190 and underwater cave-3 frame 3415 (zero-based frame labels; `--frames` is one greater). These are the first rendered frames after their authored transitions. No warp, save load, injected state, XML edit or collision bypass is used.

Each bounded case runs an ordinary arm and a repeat arm with fixed `0.0166667`-second gameplay steps and the same test-only `100`-ms-per-update film clock. The repeat arm captures its ordinary frame and a sealed-command replay. All three complete BMP byte streams must match. The gate also asserts the authored transition frame/scene chain, alive endpoint, exact baseline-vs-repeat gameplay/combat/event lines (excluding wall elapsed time), and presence of the replay marker only in the repeat arm. The renderer-only replay is not a second gameplay update.

## Results

Final built binary SHA-256: `2c620d4c186917eea4f6a924c8635c2477097f9d42c510871b0c3336d1a33b54` (Docker `deathleak-linux-build:bookworm`, `make -C port -j2`). The gate compares all three complete BMPs per case; outputs below are the shared SHA-256 values, with **0/921,600 changed pixels** in every case. Scene, authored transition chain, gameplay/combat endpoint and event lines match between arms.

| First scene frame | Full post-HUD BMP SHA-256 |
|---|---|
| cave 6 (1780) | `d1f9bec10002a3732dc517cc98541219aaaf3d13e5d5352801bb348de0c3b73f` |
| Hot Springs (3037) | `8f65f5260b5d9f705348e78effab455cf14cfdafc371a3d094841d3aa77441bf` |
| underwater cave 4 (3190) | `c80b2f58700d494c9bef3ea3720c4141c10495b209c3c1d32d0188ca8f75e9ec` |
| underwater cave 3 (3415) | `35584f63eb251728eff6451cbfb6e31ebed1251c4002fa508bd751247e50130d` |

Capture/log artifacts: ignored `port/build/posthud-later-scenes-final/`; reproduce with `port/tests/posthud_later_scene_replay.sh`, or run `DEATHLEAK_VERIFY_ONLY=1` against retained artifacts. `sh -n` and `git diff --check` passed.

## Limits

The physical Xorg `:0` authority cookie remains inaccessible to this user (`Invalid MIT-MAGIC-COOKIE-1 key`); this offscreen software OpenGL gate establishes functional pixel equality only. It does not establish selected high-Hz mode, physical nouveau/NVE7 execution, sustained FPS, hardware-input or release acceptance. No atlas/bind work was started.

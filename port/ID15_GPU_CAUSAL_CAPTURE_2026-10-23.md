# ID15 current-tree physical-GPU causal capture — 2026-10-23

## Result

The frozen current-tree executable produced baseline and draw-hidden-ID15 captures at the same scripted 540-frame forest-1 route. Both used physical Xorg `:0`/nouveau NVE7 OpenGL with strict acceleration and initialized GPU Sepia postprocessing. The images differ in the right-hand shaft region: **79,175 decoded RGB pixels changed**, bbox `[1011,316,1280,649)`.

**The protocol's strict diagnostic-acceptance gate is not met:** the complete `Simulation ended at ...` lines are not byte-identical. Every printed gameplay/state field is equal, but `elapsed=` is measured by `steady_clock` wall time (`port/linux/main.cpp:1283-1292`), not fixed simulation time: baseline `11.2972s`, hide-ID15 `11.2659s`. Thus the two runs are fixed-frame and equal in all printed simulation state, but cannot be described as having a byte-equal endpoint under the written gate. The spatial delta is strong evidence of ID15's Linux-renderer contribution, bounded by this endpoint caveat; it is **not** Windows parity, visual acceptance, or an art-edit instruction.

## Capture identity and route

- Repository HEAD at capture: see `port/build/id15-current-tree-gpu-2026-10-23/head.txt`; working-tree snapshot in adjacent `tree-status.txt`. Existing source/build edits by other owners were preserved; this lane made no build/source/test edit.
- Shared executable was copied **before** launching either variant to `port/build/id15-current-tree-gpu-2026-10-23/deathleak-linux.frozen`; source, frozen-before, frozen-after, and shared-after SHA-256 all equal `4011c2fc6de7a758b46331b9c8bffcf49da933743806f8572f334dcdb318e158` (8,974,616-byte executable). The frozen copy was used for both captures.
- Single sequential Docker container, `deathleak-video-capture:bookworm`, `--network host --hostname jerome-Linux-MacBookPro --device /dev/dri`, repository and X socket bind mounts, `DISPLAY=:0`, `XAUTHORITY=/xauthority`, LightDM authority read-only bind via `--mount type=bind,source=/var/run/lightdm/root/:0,target=/xauthority,readonly`. The gate document's `-v /var/run/lightdm/root/:0:/xauthority:ro` syntax failed Docker parsing because of the colon in the filename; `--mount` was used for the successful run. No offscreen/software fallback.
- Both: `SDL_VIDEODRIVER=x11 SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy`, `--play Game/XmlFiles/levels/forest_exploration_1.xml --script B:300,F:240 --frames 540 --fixed-dt 0.0166667 --capture ...`. Baseline unset both XML draw filters; hide variant set only `DEATHLEAK_RENDER_HIDE_XML_IDS=15`. No simultaneous game/recorder on Xorg `:0` in this lane.
- Both logs: `SDL renderer selected video_driver=x11 name=opengl flags=14 accelerated=1 software=0 vsync=1 mode=strict-accelerated`; `OpenGL device vendor=nouveau renderer=NVE7`; `Sepia.fx GPU postprocess initialized (scene before HUD)`; 71 original level sprites loaded, none missing.

## Endpoint and pixel measurements

Both final lines report forest-1 `player=(2360.52,3114.74) orbs=5 keys=0 health=25 focus=100 balance=5 spawned=0 enemies=0 hits=0 kills=0 fired=32 shots=2 grounded=1`, identical upgrade/velocity/pending-orb fields. The only printed discrepancy is wall-clock `elapsed` noted above; raw lines are retained in `baseline.log` and `hide-15.log`.

| Measurement | Current frozen-tree pair |
|---|---:|
| Decoded image size | 1280×720 RGB each |
| Pixels with any RGB-channel change | 79,175 |
| Changed bbox, half-open | `[1011,316,1280,649)` |
| ROI `[900,300,1200,550)` changed pixels | 38,540 |
| `(1100,350)` baseline → hide | `(111,96,68)` → `(97,83,58)` |
| `(1200,400)` baseline → hide | `(116,100,72)` → `(61,53,37)` |

Baseline BMP SHA-256: `7138a74cd719f6e6511e61c024b1b65eb67dcd206d28907ce0825d1ec9ea81ff`.
Hide-ID15 BMP SHA-256: `e632773982bb34204a01414dd13e0f6b6f49c726d6725b5e065a3dec96c4bcfd`.

Prior-tree reference (not an acceptance target) was 81,525 pixels, bbox `[985,323,1280,612)`, ROI 47,343. Current-tree geometry/count/samples differ, as expected for a changed visual tree. The current change remains confined to the anticipated right-side shaft band. No only-ID15 or hide-38/39 secondary diagnostic was run.

## Evidence and conclusion

Raw BMPs, logs, binary hashes, HEAD, and worktree snapshot are in `port/build/id15-current-tree-gpu-2026-10-23/`. RGB measurements use Pillow `ImageChops.difference` with a per-pixel any-channel mask and the gate's half-open ROI. The frozen binary stayed identical before and after, and the shared binary had the same digest after capture despite concurrent non-GPU work.

The fixed-script/fixed-frame, equal-gameplay-state hide delta localizes a current Linux-renderer brightness contribution to authored ID15. The protocol's full-line endpoint equality criterion fails solely because the executable prints non-deterministic wall duration, so this is a **qualified causal capture**, not strict gate acceptance. There is still no matched original-Windows frame at this camera/state; no claim about original parity, need to remove ID15, or release readiness follows.

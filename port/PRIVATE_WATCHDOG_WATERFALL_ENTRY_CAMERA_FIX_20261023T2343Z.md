# Waterfall entry-camera snap — 2026-10-23 23:43Z

## Result

**Fixed and physically verified.** On destination install, `port/linux/main.cpp` now initializes each camera axis from the just-placed LevelEntry player only when that axis is authored to follow. Non-follow axes retain the XML camera position. No XML, body, shadow, global culling, or renderer changes were made. The existing per-tick follow helper still applies authored offsets/lag and bounds; this does not permanently lock camera to player.

Source basis: `PRIVATE_WATCHDOG_FRAME424_PLAYER_CAMERA_VISIBILITY_SOURCE_GATE_20261023T2341Z.md`, original `GameObjectManager.cpp:183-202` `FollowObjectsOrigin(player)`, and Waterfall XML `followx=true`, `followy=false`, X lag 15, left bound -2300. The matched Forest entry places player at X=-2191.014648. Before fix, destination camera initialized from XML X=0 and the frame-424 packet reported draw camera X=-1082.38098, body fully left of viewport.

## Verification

- Focused source-anchored CPU fixture: `python3 port/tests/private_waterfall_entry_camera_cpu_20261023t2343z.py`, **PASS** locally and inside `deathleak-linux-build:bookworm`. It checks the destination branch wiring against source; parses authored Waterfall follow flags, bounds, lag, entry, and player dimensions; checks non-follow Y remains 3000; calculates the first bound/lag step and a positive-X body destination versus the previous offscreen body. It does not claim to reproduce all 424 ticks.
- Docker build: `make -C port TARGET=build/private-waterfall-entry-camera-fix-20261023t2343z/deathleak-linux -j2`, **exit 0** (existing TinyXML warnings only). Unique ignored output directory: `port/build/private-waterfall-entry-camera-fix-20261023t2343z/`; build log retained.
- Exclusive physical Xorg `:0`/nouveau **NVE7** capture: same fresh Forest-1 `--script B:1800 --fixed-dt 0.016666667 --frames 425` route, `--atomic-postfade-waterfall-packet /out/frame424.json`, **exit 0**. SDL log confirms x11/OpenGL strict accelerated, `accelerated=1 software=0`, GPU Sepia scene before HUD. Packet confirms first postfade render/update ordinal **424**, Waterfall active, no splash, gameplay/player visual updates true. Binary SHA-256 `f9f04d27e65f74040fae5a772c7e9a024d64130d6ce4f2730a3257695590645b`; BMP SHA-256 `22aa399ef0e552c463b395c85fdbce906bda332573eb4898bd72079f4cc7cd1f` matches packet/log.
- Physical packet camera X is **-1340** (authored left-bound clamp center), player X **-2139.01465**; Y camera remains **3000**. Ordered scene draw ordinal 16 submits actual body texture `Game/Media/characters/player/run_default/run_default_004.png` at destination **[46.859, 211.752, 120.929, 143.568]**, entirely inside the 1280×720 viewport. Ordinal 11 separately submits the shadow at X=40.657..173.990. This resolves the prior body-cull symptom (old frame-424 body right edge ≈-4 px). The actual BMP/PNG and JSON are retained in the ignored output directory.

This is a Linux source/capture visibility result, **not** a Windows matched-pixel parity claim or Jerome acceptance.

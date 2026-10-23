# Forest-1 jump takeoff material spray — 2026-10-22

## Result

Implemented the source's grounded solid-line jump burst as a separate scene-owned, nonlooping ten-particle emitter. `PlayerMotion` retains the winning ground-line segment from collision and calls the effect immediately after the existing +10 Y jump bump, before clearing contact. This preserves the source ordering and excludes airborne/coyote/wall/solid-box cases without changing jump physics. Material texture lookup is source-backed, including `water` if a caller has a retained line; the existing Running-footstep exact-water veto remains unchanged.

The burst uses `kPlayer` depth, `x + facing*10`, `postBumpCollisionBottom + 35`, (0,1) directed spread 0.8, 1200×720 culling bounds, speed 1.01–1.50 in source's 0.01 steps, lifetime 1.2 down to .84 seconds across ten particles, size 32, gravity .8, brightness .81–1, scale-to 5, ±50/±2 integer spawn spread, 0.15/0.9 fade, and the existing deterministic Linux emitter RNG stream. That seed is not a Windows global-rand match. Scene update retires the completed emitter; draw occurs adjacent to the player in the painter stream.

## Verification

- Docker image `deathleak-linux-build:bookworm`: `make -C port -j2` passed; only pre-existing TinyXML warnings.
- Focused CPU test `port/tests/jump_takeoff_material_spray_test.cpp` passed. It checks a real forest-1 ID3 grounded jump, exact count/texture/depth/origin/ranges, no duplicate while holding jump, airborne and accepted coyote-jump negatives, missing-material-texture negative, `water` material without footstep veto, and a submerged swim-input negative.
- Existing `footstep_material_spray_test.cpp` passed after the change, including its exact-water negative.
- Physical Xorg `:0` rendered eight scripted forest frames (`.:2,J:1,.:5`) with strict accelerated SDL OpenGL: `video_driver=x11 name=opengl ... accelerated=1 software=0 ... mode=strict-accelerated`, `vendor=nouveau renderer=NVE7`. Capture: ignored `port/build/jump_takeoff_gpu.bmp` (1280×720) and converted PNG. This is a post-change GPU execution/capture, **not** a pixel A/B or Windows parity proof. The source-frame baseline was not separately built/captured; image inspection tooling did not yield a reliable visual judgment. CPU event contract is the decisive gate.

## Remaining gaps

No matched Windows camera/contact/effect-age/RNG capture; no physical-GPU baseline A/B. Landing material burst, frame-marked roll/strafe footsteps, sound parity, and non-forest scenes remain outside this slice. No XML, art, footstep emitter logic, or physics parameters changed.

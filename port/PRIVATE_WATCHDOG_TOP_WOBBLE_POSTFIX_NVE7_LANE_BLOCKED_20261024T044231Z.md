# Waterfall-top IDs 13/14 post-fix GPU gate — occupied lane at 04:42Z

**Disposition:** Blocked before acquisition. No build, game launch, FFmpeg capture, source edit, or GPU experiment was started. Physical Xorg `:0`/NVE7 and the shared build route were occupied by a separate performance owner, so the requested post-fix ~20 s paired-raft gameplay capture and measured same-present pixel effect were not obtained. The GPU/build/capture lane was never acquired and is explicitly released.

## Preflight evidence

- Current repository HEAD: `a5bbd98` (source-proven unbounded waterfall wobble V-phase fix). `git status --short` showed existing shared dirty files including `CHECKPOINT_STATUS.md`, `port/linux/SDLRenderer.cpp`, tests, and untracked `PORT_WORK_STATUS.md`; all were preserved.
- At 2026-10-24 04:42:31 UTC, host `docker ps` showed container `43375b749570`, image `deathleak-capture:post-foliage`, **Up 9 seconds**.
- Host process PID `28877` was a live `docker run` mounting `/dev/dri`, `/tmp/.X11-unix`, LightDM Xauthority, read-only `/src`, and `port/build/performance-root-5880150d` as `/out`; environment selected `DISPLAY=:0`, X11, OpenGL, and GPU profiling. Container process PID `28930` was `/out/subpass-linux --play Game/XmlFiles/levels/forest_exploration_1.xml --script B:360 --frames 360 --fixed-dt 0.0166667 --profile`. Physical `Xorg` PID `1360` was serving `:0`.
- This is active physical-GPU use by another owner, not a stale reservation. We did not race it. No ownership claim or cleanup of its container/process was attempted.

## Result boundary and follow-up

The earlier `port/PRIVATE_WATCHDOG_TOP_WOBBLE_POSTFIX_NVE7_LANE_BLOCKED_20261024T0441Z.md` records CPU/source proof and a viable earlier synchronized route, but its clip predates HEAD `a5bbd98`. Neither report supplies post-fix ID13/14 submitted-V observations or a same-present RGB delta. Windows parity is unproven without a matched Windows command/phase/pixel packet. Once the performance owner releases the lane, run an isolated current-tree build and physical-GPU active Waterfall-top capture, record paired rafts (~20 s), ID13/14 V after integer crossing, and a same-state pixel comparison if practicable.

`CHECKPOINT_STATUS.md` was already shared-dirty, so no append was made. This report is the only file created by this worker. **GPU/build lane explicitly released (never acquired).**

# Waterfall-top IDs 13/14 post-fix NVE7 visual gate — CPU-only preflight

## Result

**Physical Xorg :0/NVE7 GPU lane was occupied at preflight, so no new GPU/build/capture was started.** The requested matched-current-tree active Waterfall-top ~20 s clip, same-state draw/pixel diagnostic, and post-fix RGB effect measurement remain outstanding. This is a lane-boundary result, not a failed gameplay or visual-parity test. The GPU/build lane was not acquired and is explicitly released by this worker.

## Exact preflight evidence

- Repository HEAD `a5bbd98` (`Preserve unbounded waterfall wobble V phase`). `port/game/LevelScene.cpp` is clean. Existing unrelated dirty `CHECKPOINT_STATUS.md` and `port/linux/SDLRenderer.cpp`, plus untracked `PORT_WORK_STATUS.md`, were observed and left untouched. No production source, Sea-2 work, binary, test, or build output was changed.
- At 2026-10-24 04:40:38 UTC, `ps` showed PID **26485**, started Fri Oct 23 21:40:30 PDT, actively running a `docker run` of `/out/subpass-linux --play Game/XmlFiles/levels/forest_exploration_1.xml --script B:360 --frames 360 --fixed-dt 0.0166667 --profile`. Its command explicitly mounts `/dev/dri`, `/tmp/.X11-unix`, LightDM Xauthority, uses `DISPLAY=:0`, `SDL_RENDER_DRIVER=opengl`, and writes to `port/build/performance-root-5880150d`. It remained live at the subsequent 04:40:50 UTC read. This is a separate performance owner on the physical display/GPU; the task's non-overlap condition prohibited starting our GPU/build route. Only the persistent LightDM Xorg `:0` process otherwise appeared in the scoped display/process check.

## CPU/source preflight and limits

- HEAD's scoped source conditional increments `autoScrollYPhase` on each draw, skips `phase -= floor(phase)` when `object.pixelWobble.enabled`, and forwards unbounded `draw.uvShiftY` for Waterfall-top XML IDs 13/14. Ordinary autoscrolling sheets retain bounded phase. The committed `port/tests/waterfall_wobble_unbounded_phase_cpu_test.cpp` exercises both wobble IDs over 480 draw submissions and a first integer crossing; its earlier worker report records PASS. This worker inspected that source/test and did **not** rerun a build or test under contested build ownership.
- The pre-fix first-input divergence was integer-V removal before the shader's second sine; unbounded V can change sampled U after phase crosses 1.0. The earlier CPU report demonstrates a nondegenerate formula witness, **not** a captured RGB delta at the active paired-raft state. The independent phase-scope audit finds IDs 13/14 are the only active-level pixel-wobble ParallaxLayers with Y autoscroll; it does not prove exact D3D/Linux effect-clock or accumulated-phase equivalence.
- `port/SYNCHRONIZED_ACTIVE_WATERFALL_VISUAL_PACKET_20261024T0250Z.md` and its retained 26.067 s clip establish a viable *route*: active Waterfall-top roughly video 9.924–21.876 s, paired rafts around 20 s, physical NVE7. That clip's frozen binary is based on earlier HEAD `0a69203b`, **not** `a5bbd98`, so it is a pre-fix route/provenance reference and cannot establish the post-fix visual effect or be treated as a matched same-state A/B. No matched Windows D3D10 command/phase/pixel packet exists here; no original parity or Jerome acceptance claim.

## Next eligible gate and release

After the performance owner explicitly releases physical Xorg :0/NVE7 and build, use an isolated, read-only-current-tree build/capture route targeting the synchronized Waterfall-top paired-raft interval (~20 s). Capture IDs 13/14 submitted V and `pixelWobbleTime` at a draw after the first integer crossing, plus same-present pre/post-ID pixels or a tightly controlled pre-fix A/B if available without dirty-source overwrite. Preserve the command/clock/camera packet, binary hash, frame samples and final RGB comparison. Without a same-state Windows packet, label this Linux post-fix effect only.

No checkpoint append: `CHECKPOINT_STATUS.md` is already shared-dirty and actively written by the watchdog; this unique report avoids racing its owner. **GPU/build/capture lane was never acquired; CPU-only preflight lane released.**

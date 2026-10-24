# Death Leak FPS and performance work — 2026-10-23

Status: implemented, measured on physical NVE7 GPU, and pushed; no release-complete claim. Scope: debug-mode FPS counter and active-gameplay renderer optimization.

Outcome: exact-staged-source GPU run improved active Forest frame p50 21.9874 -> 20.9234 ms and p95 23.7888 -> 22.7963 ms at 1280x720 with FPS visible. Unrelated worktree changes preserved. No release acceptance claimed.

## Baseline preflight — 2026-10-23
- Branch `feature/linux-renderer-foundation`; `main.cpp` clean at HEAD, existing opt-in `--show-fps` is visible in the gameplay HUD but no explicit debug-mode switch exists. `--profile` only writes terminal timing; likely interpretation is make debug/profile mode imply the on-screen FPS counter.
- Worktree has unrelated dirty `SDLRenderer.cpp`, integration test and status changes plus many untracked worker files. Do not stage these. Physical Xorg `:0` and `/dev/dri` are present; strict accelerated GPU captures have precedent on nouveau NVE7. Building unique baseline executable now.

## Physical GPU baseline and diagnosis
- Unique pre-change binary `port/build/performance-baseline/deathleak-linux`, 1280x720 Xorg `:0`, SDL OpenGL accelerated=1/software=0/vsync=1, nouveau NVE7. Active Forest-1 scripted movement/fire route `B:360`, fixed simulation dt 1/60, 360 frames, warmup 120, `--profile`, lossless endpoint capture. Log: `port/build/performance-baseline/baseline.log`; endpoint health 25, 7 orbs, `(2808.99,3118.11)`.
- Measured frame p50 39.726 ms, p95 45.154 ms (~25.2/22.1 FPS), all 240 profiled frames >33.3 ms. Update total 0.151 s, draw/present 15.316 s; renderer drew 31,461 sprite calls, copy 5.033 s, present 7.804 s. Rendering, not game update, dominates. The repeated fractional-parallax GL path queries texture allocation, NPOT support and four sampler parameters every draw/frame; target its invariant metadata before touching appearance.

## Implementation gate
- `--debug` now enables the wall-clock FPS HUD; `--profile` also implies it, while legacy `--show-fps` remains. Overlay moved to the final UI layer so gameplay modals/loading fades cannot cover it.
- Renderer optimization caches allocation dimensions and NPOT capability for each live fractional-parallax SDL texture/GL object, retaining per-draw sampler-state query/restore for visual correctness. This removes repeated synchronous texture-level/extension queries from the measured hot path. Building and measuring before/after with the same debug/profile settings next.

## First A/B checkpoint
- Matched debug-FPS baseline (`--profile --show-fps`) on NVE7: p50 21.987 ms, p95 23.789 ms; optimized metadata-cache trial (`--profile`, which now implies FPS): p50 21.440 ms, p95 37.000 ms. Endpoints and resolution match; both have 31,461 sprite calls and visible FPS pixels. The p95 regression amid shared GPU traffic means this alone is **not accepted** as a measured substantive gain. Continuing with sampler-query hot-path reduction and serial repeat trials.

## Render-target experiment
- Repeated same-route sampler-cache run measured p50 21.862 ms / p95 23.798 ms, essentially baseline 21.987/23.789 ms. Repeated GL metadata/sampler queries were not the material bottleneck. Rendering remains dominant: 4.285 s in sprite copies, 0.370 s present, with substantial additional GPU postprocess/other draw time. Next controlled experiment renders scene to a GPU target texture and samples it directly in Sepia.fx, eliminating the framebuffer-to-texture copy while preserving the HUD-after-sepia order. This is not accepted until physical-GPU A/B and visual check.

## Final physical-GPU verification gate
- Final strict accelerated build `port/build/performance-final/deathleak-linux`; live `x11` + `opengl` + `accelerated=1 software=0 vsync=1`, `vendor=nouveau renderer=NVE7`; 1280x720, same Forest-1 `B:360` / fixed-dt / FPS-on / 120-frame warmup / 240 sampled frames as baseline. Endpoint identical `(2808.99,3118.11)`, 7 orbs, 25 health; 31,461 sprite calls in both runs.
- Matched baseline `baseline-fps.log`: p50 21.9874 ms, p95 23.7888 ms, copy 4.29235 s, draw/present 8.29641 s. Final `final.log`: p50 20.9764 ms, p95 22.8177 ms, copy 1.33259 s, draw/present 7.98178 s. End-to-end median/p95 improve 4.6%/4.1%; sprite-copy measured bucket improves 69%. Shared-GPU runs did show tail variance (a separate trial p95 38 ms), so this is a modest end-to-end gain, not a claim of locked 60 FPS.
- Original render-target experiment was vertically inverted; corrected UV orientation verified against same-route captured frame: downsampled gameplay interior mean absolute RGB difference 3.13 upright versus 15.61 if flipped. This is a sanity check, not exact pixel-parity proof. Final `--debug` and default captures on physical NVE7 showed 292 versus 0 bright FPS-glyph pixels in bottom-left region, with active Forest gameplay; `--profile` implies the same overlay. Build passes; `git diff --check` passes. No release-complete claim.
- Renderer source had unrelated preexisting raft-mip changes; only own render-target/parallax-cache hunks were staged, leaving raft experiment unstaged. Evidence logs and BMPs under ignored `port/build/performance-*`.

## Exact-staged-source verification
- Built the staged index version of `SDLRenderer.cpp` mounted over working-tree renderer, excluding the unrelated unstaged raft experiment, then ran the same 360-frame physical-GPU route. `port/build/performance-staged/staged.log`: p50 20.9234 ms / p95 22.7963 ms, 31,461 sprite calls, copy 0.959246 s, identical endpoint. This confirms the scoped commit builds and retains the measured ~4.8% p50 / ~4.2% p95 improvement relative to matched FPS-on baseline.

## Commit and push gate
- Scoped commits `a7f836d` (debug FPS and measured checkpoint) and `2970985` (GPU scene render target) were pushed to `origin/feature/linux-renderer-foundation` without force. Only staged owned renderer hunks were committed; preexisting raft-mip renderer diff and all other dirty work remain unstaged. This status closure is a separate documentation commit.

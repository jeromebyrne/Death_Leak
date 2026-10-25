# Death Leak Linux performance goal

Owner: OpenClaw performance workstream. Started 2026-10-25. Status: active, not accepted.

## Outcome

Maximize sustained active-gameplay frame rate on the real NVE7 display at 1280×720 with hardware-accelerated rendering, without reducing visual fidelity, game logic, resolution, or required effects. First gate: <=16.667 ms median (>=60 FPS) and controlled tail latency across representative Forest, Waterfall, combat, and transition routes. Continue optimizing beyond 60 FPS where repeatable gains preserve parity. No software-rendering or headless-only acceptance.

## Current evidence

- Forest active route: 20.1275 ms median, about 50 FPS, on physical NVE7; the full-scene GPU path dominates CPU update.
- Far-background ID 1 is the largest individually removable scene layer (~1.5–1.6 ms), but removal changes authored imagery. Its texture has full nonzero alpha, so transparent-border culling is not useful.
- Previously shipped render-target and text-batching changes yielded measured gains. Backplane-copy cache was slower and changed pixels; rejected.
- The long scripted route stalls in Waterfall-bottom; its performance numbers are not full-campaign acceptance.

## Work program

1. Preserve and reconcile the dirty source tree. Use isolated diagnostic sources/builds; never overwrite unrelated changes or race another GPU owner.
2. Repeat an uncontended physical-GPU baseline with exact binary/route/settings, p50/p95/p99, slow-frame and pass timings; keep control runs interleaved with candidates.
3. Run the next ID-1 scissor/visible-region diagnostic to distinguish fragment/overdraw cost from fractional-GL state/driver overhead. The diagnostic may change pixels; it is not a candidate fix.
4. Exhaust measured, visually lossless avenues in priority order: occlusion-aware scene painter/geometry; batching and state-change reduction; texture sampling and formats; shader/pass fusion; framebuffer/target traffic; upload/readback avoidance; SDL/OpenGL presentation/compositor path; driver and GPU power-state controls. For each, record predicted mechanism, GPU A/B, exact-frame pixel checks, and keep/reject result. Do not optimize based on CPU wall-time phase labels alone.
5. Repair the Waterfall-bottom scripted traversal gate separately, then run sustained physical-display routes through all available scene types and transitions. Track median and tail latency, frame pacing, memory/VRAM, and thermal behavior.
6. Commit and push only verified, scoped source changes; leave failed prototypes as documented experiments. Update the checkpoint after each bounded step and proactively report meaningful gains or genuine blockers in the originating DM.

## Acceptance / stop condition

Performance work is complete only after representative sustained physical-GPU play is measured, visuals and gameplay remain acceptable, and the highest practicable stable frame rate is demonstrated with evidence. A 60-FPS median alone does not establish release readiness if p95/p99 or transition stalls remain bad. If a hardware/driver ceiling prevents the target, document measured alternatives and tradeoffs rather than claiming success.

## Next action

Run the isolated ID-1 scissor diagnostic on the physical NVE7 display, then prototype the best supported visual-preserving change and repeat A/B plus pixel checks. Current status: goal recorded; no new optimization accepted yet.

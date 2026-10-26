# Active-effect interpolation coherence gate — 2026-10-26

**Decision:** retain diagnostic mixed-command sampling, but conservatively hold a complete packet if either paired packet contains an unowned sprite or sprite batch. Reject a visible interpolation B for now: this conservative policy is coherent but produces no sub-tick motion in the active Forest case. No active A/B/A timing or high-refresh acceptance is claimed.

## Scope and source contract

Exact pushed base `8e541a1d994aa30cf3b404d4ceaeb8b133956cb9` was verified before a clean detached checkout. The primary dirty checkout and its unrelated tracked/untracked files were not used for build. In `AdmitMixedCommands`, an unowned sprite or sprite batch in either previous or current sealed packet returns a valid, all-held decision before any per-command sampling. The opt-in diagnostic replay still uses full painter order, Begin/Zoom/postprocess and the same renderer; default visible presentation is unchanged. The existing scene-epoch/discontinuity policy also holds scene cuts. CPU fixtures assert both previous/current unowned cases, unowned batch, ordinary tagged subset, cuts, animation changes, and alpha=1 current-packet behavior. This guard is deliberately broader than a world-only classification because the current command schema does not identify unowned draw purpose.

## Physical NVE7 evidence

Authenticated Xorg `:0`, SDL x11/OpenGL, nouveau/NVE7, `accelerated=1 software=0 vsync=1`, actual/logical 1280×720, fixed 1/60 simulation. No other game/apitrace/FFmpeg process overlapped. Captures and complete logs are under `/srv/openclaw/workspace/death_leak_analysis/active-effect-coherence/`.

- Active Forest frame 120 of `B:360`: 93 commands, 18 unowned sprites at ordinals 50–67 (texture IDs 243/293), sampled **0**, held **93**. Complete current, alpha=1 and alpha=.5 replays are byte-identical BMPs, SHA-256 `46295ad258fa4516c133c9264545b8a9268803d9ca4d3f8cc5951e0b0e0a3cf0`, hence exact RGB parity at all 921,600 pixels. Unlike the prior 43-sampled/50-held packet, no scenery phase moves against held player/effects. This is coherent hold-current composition, not proof of smooth interpolation.
- Forest→Waterfall-top cut frame 415 of `R:800,L:1200`: epoch 2, 40 commands, sampled **0**. Current, alpha=1, alpha=.5 byte-identical BMPs, SHA-256 `5369f94fb1facd4c89b2e0a9d96009effccc72e0735db13d0d63cf47e3299e3d`. No cross-scene blend or stale packet appears.

These captures prove the bounded exact-frame active/cut contract, not visual quality across all routes. The frame-120 result has no fractional visible displacement, so there is no sound opt-in visible interpolation B or meaningful serial A/B/A speed test. The only shipped software change is diagnostic admission safety; no 8K, high-Hz active gameplay, 60-FPS, VRAM/thermal or release acceptance.

## Next distinct gate

Trace emitted sprite/batch origin and tag stable player, projectile, spray and particle-generation identities plus phase for **every active world draw**. Recheck painter ordinal/visibility/cut topology across births and deaths; then capture moving/occluded fractional images on NVE7 that actually differ from current while alpha=1 remains exact. Only after that enable an opt-in visible B and conduct serial physical advertised-mode active A/B/A with timing and visual proof.

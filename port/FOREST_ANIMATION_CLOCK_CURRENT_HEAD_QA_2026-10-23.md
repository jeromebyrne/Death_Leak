# Forest animation clock current-HEAD QA — 2026-10-23

**Verdict: one bounded active-play phase mismatch remains under a slow rendered frame.** The new preceding-rendered-delta *slot and consumers* are correctly wired for ordinary matching deltas at HEAD `27ef41b`; the discrepancy is the value committed to that slot when a rendered iteration takes longer than 1/60 s. This is source arithmetic/command-path evidence, not a measured Windows/Linux pixel comparison.

## Exact source and port seam

- Original `Game/src/program.cpp:115-117,149-175` sets `TargetDelta=1/60`, waits until at least that much wall time has elapsed, caps every update delta **to 1/60** even if 50 ms elapsed, then calls `Update(modifiedDelta)`, `Render()`, and only afterward `SetLastUpdateDelta(modifiedDelta)`. `Game/src/Core/Timing.cpp:6-15` initializes that prior slot to zero. `Game/src/Animation/AnimationPart.cpp:169-196,201-237` adds `GetLastUpdateDelta()` for looped/finite frames, advancing at most one frame per call.
- Linux `port/linux/main.cpp:451,454-464,1106-1110,1222-1223` likewise initializes a process prior slot to zero and commits it after `EndFrame`, but derives current `dt` from elapsed wall time and caps only at **0.05 s**. That same `dt` is committed to `precedingRenderedModifiedDelta`; `port/game/LevelScene.cpp:936-943,1137-1144,1163-1177` consumes the prior slot for authored sprite, player body, and arm frames. This is not a failure to use the prior slot: the slot faithfully carries a different upstream timestep.
- Forest ID68 references `Game/XmlFiles/orb_anim.xml:3-15`, whose Still sequence is ten distinct `Game/Media/orb/{1,11,...,91}.png` frames at 10 fps (`Game/XmlFiles/levels/forest_exploration_1.xml:1108-1125`). Forest ID2 uses `player_anim_default.xml` (`forest_exploration_1.xml:33-51`) with Still body at 20 fps (also exercised by `port/tests/forest_preceding_delta_cpu_test.cpp`). Both are visible authored active-gameplay sprites when in the update/draw zone.

## Bounded discriminator

Assume gameplay remains active, ID68 stays in its update zone, ID2 remains grounded Still, no time modifier, no sequence reset, and three consecutive rendered iterations each take at least 50 ms wall time. After initial zero-slot draw, the next two animation updates consume the preceding rendered values:

| Render iteration | Windows prior delta | Linux prior delta | ID68 frame (Windows/Linux) | ID2 Still frame (Windows/Linux) |
| --- | ---: | ---: | ---: | ---: |
| first | 0 | 0 | 0 / 0 | 0 / 0 |
| second | 1/60 | .05 | 0 / 0 | 0 / 1 |
| third | 1/60 | .05 | 0 / 1 | 0 / 2 |

At the second draw the body texture is `idle_default_000.png` versus `idle_default_001.png`; at the third, the orb is `Game/Media/orb/1.png` versus `.../11.png`. The native ID2 cumulative time is still below 1/20 s after two prior-delta adds; Linux crosses once on each .05 add. The orb reaches its 1/10 s threshold only on Linux's second .05 add. Floating-point boundary behavior is not material for the Windows below-threshold cases. This schedule demonstrates potentially visible texture-frame selection during a stutter; it does not establish frequency or RGB severity in a real capture. Other gameplay simulation phases also differ under this schedule, so a matched Windows pixel oracle would need controlled timing/state.

## Action and limits

If closing this clock-phase gap, align the production Linux rendered update timestep with the Windows 1/60 cap before committing the prior slot, while assessing gameplay/physics effects of that broader timestep change. Do **not** change `LevelScene`'s prior-slot accumulators or the forest XML/assets for this finding. Existing 20-tick CPU spy uses at most .016 s deltas, so it correctly verifies prior-slot plumbing but cannot cover this >1/60 case. No new build, CPU fixture, physical GPU/display, capture, original Windows execution, source/test/shared-status edit, commit, or push was performed in this read-only audit; this report is the sole write. The exclusive GPU reviewer owns current-tree visual acceptance.

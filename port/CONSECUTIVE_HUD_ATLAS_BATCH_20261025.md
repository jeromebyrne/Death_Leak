# Consecutive gameplay-HUD atlas batch — physical NVE7 gate (2026-10-25)

## Scope and mechanism

Starting at `b2bd9ed`, seven unrelated tracked paths were dirty, including an unstaged raft-mip experiment in `port/linux/SDLRenderer.cpp`. The existing opt-in gameplay-HUD atlas (`DEATHLEAK_ATLAS_HUD=on`) is a single 234×132 page. HUD meter bar/cap/overlay draws are consecutive in painter order after orb count text; they alternate virtual source rectangles, flip and alpha (220/178). SDL copies emitted repeated real GL submissions even though those IDs share one page. An isolated candidate collected only that consecutive HUD segment and submitted ordered quads in one `SDL_RenderGeometry` call with per-vertex tint/alpha, source UVs and flip-relative trimmed destination offsets. Nonatlas, unsupported or disabled cases fall back to the previous per-sprite `DrawSprite` sequence. Scene sprite rendering, the film layer, render targets, private shaders and unrelated renderer code were not changed.

The initial isolated geometry candidate changed 1,188 HUD pixels because flipped trimmed rectangles were projected to the unflipped location. Correcting the flip-relative destination offset made the two-update physical full frame exact. That failed prototype was not committed.

## Reproducible physical evidence

Ignored sources, builds, logs, BMPs and apitrace files: `port/build/consecutive-batch-20261025/`. The baseline binary used clean-HEAD renderer source; the candidate used isolated clean-HEAD sources plus only this slice. Both used Xorg `:0`, SDL x11/OpenGL accelerated=1/software=0/vsync=1, nouveau NVE7, 1280×720, fixed-dt Forest route `B:180`; the only runtime arm difference was `DEATHLEAK_BATCH_HUD=on` in B, with `DEATHLEAK_ATLAS_HUD=on` in all arms.

- Serial A/B/A real apitrace, 180 frames: `glDrawArrays` **17,294 / 16,034 / 17,294** and `glBindTexture` **25,990 / 24,730 / 25,990**. The candidate removes 1,260 draws and 1,260 binds (7 per frame), with identical gameplay endpoint `(1368.99,3107.3)`. Traces are call evidence, not FPS evidence.
- Untraced serial A/B/A, 60 sampled frames after 120 warmup: p50 **22.2064 / 22.2361 / 22.2524 ms**, p95 **22.5213 / 22.4628 / 22.5146 ms**, p99 **22.5841 / 22.5892 / 22.5667 ms**. Candidate latency overlaps controls; **no sustained FPS gain is established**, but no measured regression in this route.
- Native-clock two-update A/B/A full physical captures matched byte-for-byte. Exact staged-source binary rebuilt with clean-HEAD renderer plus staged batch hunk also matched the control BMP SHA-256 `ad37f3a25e4249b13b6d9a929eff664d55c56b5a360d653532027bb8ebf4368a` on NVE7.
- Native-clock frame-179 full captures differed across arms because the existing full-screen film selects frames from process uptime; this is not a batch pixel gate. With a **test-only**, isolated deterministic film selector applied identically to one candidate binary and `DEATHLEAK_BATCH_HUD` toggled off/on/off, serial 180-frame full post-HUD BMP SHA-256 was identical in all arms: `c1a8741121c97f648346433a23d481807cbab7f2b0d2390a81d860f00dd7dfce`. That clock selector is not in the staged or shipped source.

## Disposition and remaining work

Accept the opt-in ordered HUD atlas submission batch as a **real GL call-count optimization only**. Do not infer 60 FPS or an unmeasured latency win. The six-asset HUD family is not broad atlas coverage; the atlas and batch still require explicit opt-in. Atlas-disabled calls use the original ordered sprite path. Same-ID/tint 7–10-draw runs observed in Forest may already be internally coalesced by SDL and were not rewritten without a demonstrated additional GL reduction. Tree-blur and rock atlas mappings remain rejected; repeat/UV-shift, foliage, fog/water, render targets, shaders and large scene painter intervals remain unbatched and need separate exact-pixel/GL/latency gates. No 8K, >60-Hz presentation, live VRAM/thermal, full-campaign or release acceptance is claimed.

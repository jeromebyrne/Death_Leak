# Waterfall-top PixelWobble uniform-cache gate — rejected

Starting clean HEAD `a5e39acb55ad02eff3dc5baf6cd2f04da719d2e8`; isolated worktree `/tmp/deathleak-waterfall-shader-20261026`. The primary tree's seven unrelated dirty tracked paths, including raft-mip renderer edits, were not modified by this experiment.

## Candidate and physical route

Test-only renderer instrumentation counted PixelWobble draws and five uniform-location lookups per draw, plus scene-target begin/unbind, scene-texture bind, framebuffer-copy fallback, and Sepia draws. Candidate cached five linked-program PixelWobble uniform locations at shader initialization and used them at draw time. It changed no shader source, uniform values, draw order, or render-target traffic. The active Waterfall-top route was 180 fixed-dt updates at 1280×720 on physical Xorg :0, SDL x11/OpenGL accelerated=1/software=0/vsync=1, nouveau/NVE7. The route reached the same player `(818.985,2933.64)`, one orb, 25 health, zero shots in all arms.

Instrumentation over 180 updates: control/candidate/control PixelWobble draws **851/851/851**, per-draw uniform queries **4255/0/4255**, target begin **180/180/180**, target unbind **180/180/180**, scene-texture bind **180/180/180**, framebuffer-copy fallback **0/0/0**, Sepia draws **180/180/180**. This is a CPU/driver uniform-query reduction, not a GPU draw, bind, copy, or traffic reduction. Fog/ground-front-specific GPU time or VRAM bytes were not measured; therefore none is claimed.

The first native-clock captures were unsuitable for parity: one control differed while the candidate and other control matched, because gameplay film uses monotonic wall time. A corrected test-only ordinal gameplay-film clock, identical on all arms, produced byte-identical **full post-HUD 1280×720 BMPs** for true serial A/B/A, SHA-256 `a4aab99ad985c1a752ae25499049aef53df336c302cdb2eae55c92995bf6748e`. This proves the synchronized endpoint pixels for this route, not every intermediate frame. Test-only clock and counters were not integrated.

## Physical latency and decision

Separate serial native-clock profile A/B/A, 120 warmup plus 60 measured frames per arm, gave p50 **21.7556 / 21.8212 / 21.7539 ms**, p95 **21.9597 / 21.9718 / 21.9111 ms**, p99 **21.9777 / 22.0250 / 21.9698 ms** (control/candidate/control). Candidate is slightly slower at each percentile than both controls, and no pass/target traffic changed. **Reject**; no production renderer change, no FPS or 8K/high-Hz acceptance. The previous covered-Sepia-clear optimization remains in HEAD.

Evidence remains in ignored isolated `port/build/wf-sync-{a,b,c}.bmp`, `/tmp/wf-profile-{a,b,c}.log`, `/tmp/wf-sync-{a,b,c}.log`; the test-only source was confined to the detached worktree. Next slice should measure a genuinely nonzero fog/ground-front GPU span or avoidable target bind/copy on an active route before proposing a traffic optimization; a uniform-location micro-optimization is below this gate.

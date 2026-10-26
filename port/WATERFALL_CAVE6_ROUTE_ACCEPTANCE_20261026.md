# Forest → Waterfall → cave 6 acceptance slice — 2026-10-26

Clean detached worktree: `a5e39ac`. Docker image: `deathleak-linux-build:bookworm`. Build: `make -C port -j2` passed, with existing compiler warnings only. Route gate: `sh port/tests/waterfall_cave_route.sh` passed in the same clean worktree. It uses offscreen llvmpipe with the explicit test-only override, so it is CPU progression evidence, not GPU/display evidence.

Single uninterrupted script: `R:1100,Q:40,R:900`, `--fixed-dt 0.0166667 --simulate-only`, starting at authored Forest-1 spawn. Transitions: Forest→Waterfall-top frame 398; Waterfall-top→bottom 762; bottom→underwater cave 1378; cave→cave 6 1780. Endpoint: cave 6 `(461.134,-1409.1)`, 15 orbs/balance, health 25, grounded. No save load, warp, injected currency, debug damage, or XML edit.

Additional bounded continuation probes: `R:400` and `R:1000` from the gate endpoint both stop at x≈1319.33; a held right-jump before the wall (`E:40,R:700`) and after approaching it (`R:200,E:40,R:700`) also stop there. `L:500` returns to underwater cave at frame 2396. In cave-6 XML, solid id 3 center x=1712.933594, width=658.244568, height=1600; left face x≈1383.81 minus player half collision width ≈64.48 gives x≈1319.33. This identifies the immediate barrier without establishing the intended route around it.

Concurrent remote commits through `c14a0fa` added read-only clock and presentation-preparation seams, but the live loop still needs update/present separation and actual selected high-refresh display proof. No FPS optimization or release claim follows from this route gate.

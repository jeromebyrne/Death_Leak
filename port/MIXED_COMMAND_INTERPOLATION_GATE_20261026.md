# Mixed per-command active-game interpolation gate — 2026-10-26

**Decision:** Accept a diagnostic CPU command policy; reject visible runtime B for now. Base is exact pushed `c406ae70d9ec49acf6b713462c0cfe96beea32aa` in clean isolated `/tmp/deathleak-mixed-command-20261026`. Primary dirty checkout was preserved except performance-checkpoint append.

## Contract implemented

`MixedCommandInterpolationGate.h` keeps the **current** sealed command stream and ordinal painter order. It samples only unique same-ordinal stable sprite identities with matching texture/source/size/tint/opacity/rotation/flip/repeat/shader parameters, admitted parallax or shader phase and bounded camera/world displacement. Every other command copies current, including film/animated textures, changed HUD batches, births/deaths, generation changes, order changes and untagged draws. Changed Begin/Zoom/postprocess/resolve ordinal state blocks all sampling; scene epoch, camera cut or explicit discontinuity also blocks all sampling. Alpha=1 returns the exact current packet directly. No new runtime render path or default draw behavior is enabled.

The policy is intentionally strict: a birth before a stable sprite shifts its ordinal, so that sprite holds current too. It does not claim coherent sub-frame physical composition simply because some commands are numerically eligible.

## Evidence

- Focused fixture passed stable position sampling with changing film/animation textures, birth/death, painter reorder, blend/opacity change, scene/camera cut, invalid alpha and alpha=1 exact current-packet return. Existing whole-frame fixture and Docker production build passed in the isolated checkout; binary SHA-256 `f8ae260a7181743f3065e5302cf0785463ca3a97a69842e5b66ac1275ef34496`. The focused fixture initially ran on the host, then was rebuilt and passed in Docker after a host/Docker libstdc++ version mismatch. The production build has no visible interpolation switch.
- Actual emitted active Forest 12-frame fixed-dt route under `SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1`, `--play Game/XmlFiles/levels/forest_exploration_1.xml --frames 12 --fixed-dt 0.0166666667 --full-frame-topology-check`: the 11 consecutive pair decisions sampled/current command counts were `44/11, 46/9, 0/53, 44/9, 45/8, 37/16, 45/8, 0/68, 45/23, 0/67, 0/68`. Whole-frame admission held all 11. Texture changes occurred in film and scene animation, including ordinals 49 and 29 in early frames. These are command-admission diagnostics, not pixels or latency.
- The source path copies the current packet at alpha=1; the fixture asserts command identity for dynamic current fields. There is **no exact physical alpha=1 pixel comparison**. Because visible ordinary/repeat interpolation has not been enabled, serial NVE7 A/B/A pixels/calls/timing would be a fictitious candidate test. No physical B was run.

## Next distinct gate

Add an offscreen same-alpha replay comparator that renders the complete current packet and mixed sample under the **same** captured GPU state, including render-target, shader/blend state and scene-cut packets. Require exact alpha=1 physical RGB equality and inspect 0<alpha<1 gameplay frames for detached effects/occlusion before enabling an opt-in runtime B. Then serial physical NVE7 active-play A/B/A with matching route and timing/call evidence. Current 15/896 atlas coverage is unchanged. No 8K, high-Hz, 60-FPS, VRAM, thermal or release acceptance follows from this CPU diagnostic.

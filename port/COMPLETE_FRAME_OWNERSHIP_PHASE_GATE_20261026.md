# Complete-frame ownership/phase gate — 2026-10-26

**Decision:** Accept diagnostic semantic ownership and conservative CPU admission; reject visible interpolation B. Exact fetched base `f731fdc72aa3135b5325eceda4953ccfa291a176` was used in clean detached `/tmp/deathleak-fullpacket-ownership-20261026`; primary dirty checkout was not edited.

## Actual active Forest packet

The prepatch five-frame test-only dummy/software route (`forest_exploration_1.xml`, fixed 1/60 s, `--full-frame-topology-check`) emitted 55 commands and 50 sprites on frames 1–2. Four unowned sprites were ordinals 49–52: gameplay film (1280×720), authored top/bottom HUD bands (1280×33.333), and the first HUD meter sprite (67.333×42.667). Ordinals 53–54 were 19-rect FillBatch and 8-sprite SpriteBatch. This is game-level HUD/postprocess output, not untagged scene sprites. Game-level film texture advanced 394→395, so identifying it cannot make it interpolable.

The opt-in diagnostic now tags film and bars with explicit game-level keys; HUD command slots get distinct authored-HUD namespace keys after the immutable HUD packet draws. The oracle admits batches only when key, cardinality, order, texture/geometry/UV/effect fields and colors are unchanged; variable meter width or batch size holds the whole frame. Auto-scroll-Y now stamps the unwrapped phase step actually committed by `PreparePresentation`; admission checks camera-parallax X and the observed Y delta against the stamp, rejects wrap seams and shader-bearing auto-scroll. No default draw/output behavior was changed.

## Executable evidence

- Focused C++ whole-frame fixture passed prior scene/camera/birth/death/generation/clock cases plus unchanged HUD FillBatch admission, changed element rejection, source-stamped nonwrapping auto-scroll admission, wrong-stamp rejection, wrap-seam rejection, and alpha=1 exact **packet** copy. Synthetic 30/60/75/120/144/165/240-Hz fixed-step counts remain 601 updates/10 s. `presentation-command-packet-test` passed.
- Isolated Docker production `make -C port -j2 all` passed; binary SHA-256 `3f4d26e884df4471f65799150cf1ef6f1ce5947082fe618cdda3442beab124ee`. `git diff --check` passed.
- Final test-only dummy/software 12-frame active Forest route had 50/50 tagged sprites at 55 commands on frames 1–2, later 48/48, 63/63 and 62/62 as actor/effect visibility/cardinality changed. All 11 pairs were `HoldCurrent`; HUD batch counts included 19→10 rects. Final five-frame diagnostic identified film texture 393→394→395 at ordinal 49 and an additional scene animation texture 223→224 at ordinal 29. Thus stable IDs and phase stamps do not prove coherent whole-frame topology. These CPU/software results are not physical NVE7 pixels or frame timing.

**Next smallest integration gate:** add per-command hold-current semantics for changing film/animated HUD/scene texture and born/dying effects while preserving a single ordered painter packet, then prove alpha=1 current-frame pixel identity and mixed-command sampling under camera cuts and active combat. Whole-frame `Interpolate` must remain disabled until every command has an explicit sample/hold policy; only a sound opt-in B merits serial physical advertised-mode NVE7 A/B/A pixels, calls and timing. No 8K, high-Hz, 60-FPS, VRAM, thermal or release acceptance is claimed.

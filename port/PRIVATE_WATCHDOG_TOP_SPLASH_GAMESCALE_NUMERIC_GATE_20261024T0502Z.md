# Top splash ID50/51 GameScale numeric gate (read-only)

## Verdict

At an **original 1920×1080 backbuffer**, the source's `GameScale.X=1` and the Linux port's 2/3 world-to-1280 projection give the **same 1280-normalized particle size, speed, gravity displacement, and lifetime-growth formula for an identical particle/random draw**. Adding a further 2/3 GameScale multiplier to the port would double-scale these effects and is **not a warranted correction**. The 1920×1080 Steam JPEG establishes image dimensions, not the original process backbuffer or a matched runtime. If the original instead ran at an actual **1280×720 backbuffer**, the source's extra `GameScale` multipliers produce a real conditional numerical mismatch, most strongly its 8/3 rather than 4 growth multiplier. This cannot identify an active-frame visible defect without that runtime fact and matched particle state.

The earliest *source-operation* difference, before size or speed is chosen, is the port's private RNG stream and omission of the original's discarded `rand()%100` at each particle's `spread=1` evaluation. This changes which subsequent random values select offsets/sizes and warm ages; it is not an isolatable visible first divergence in the unmatched paired-raft composite. No unequivocal ID50/51 visual bug warrants a production change from these sources alone.

## Exact arithmetic, both IDs

Both XML entries have `numparticles=10`, `speed min=5 max=-32`, `size min=45 max=90 scaleto=true scaletovalue=4`, `gravity=22`, `fade in=out=.25`, and direction Y=-1. ID50 direction X=0, ID51 X=.01. The inverted speed branch selects **-32**, not a random speed. Let `N=(rand()%45)+46`, so N is one of 46…90; let `u=t/L` for each particle lifetime L=1.5−0.1i seconds (i=0…9, hence 1.5…0.6). Source vertices are made from the pre-advance size; the newly grown size is seen at the next update.

| Quantity | Original, actual 1920×1080 (g=1) | Original 1920 output normalized to 1280 (×2/3) | Current port 1280×720 | Original, *if actual backbuffer is 1280×720* (g=2/3) |
|---|---:|---:|---:|---:|
| Initial diameter | N = 46…90 px | (2/3)N = 30.667…60 px | world N; drawn (2/3)N = 30.667…60 px | `int(N×2/3)` = 30…60 px |
| Speed scalar | −32 px/tick | −64/3 = −21.333 px/tick | world −32; projected −64/3 px/tick | −64/3 px/tick |
| Gravity scalar | 22 px/tick | 44/3 = 14.667 px/tick | world 22; projected 44/3 px/tick | 22 px/tick |
| Y advance before air drag, direction Y=−1 | +32−22 = +10 px/tick | +20/3 = +6.667 px/tick | world +10; projected +20/3 px/tick | +64/3−22 = −2/3 px/tick |
| Scale-to multiplier | 4 | 4 | 4 | 4×2/3 = 8/3 |
| Diameter at normalized age u | N(1+3u) | (2/3)N(1+3u) | drawn (2/3)N(1+3u) | `int(2N/3)`(1+5u/3) |
| Nominal final diameter at u=1 | 4N = 184…360 px | (8/3)N = 122.667…240 px | (8/3)N = 122.667…240 px | (8/3)`int(2N/3)` = 80…160 px |

These are **per-particle quads**, not the XML 300×300 emitter/cull box. The source multiplies speed by .99 before every positive-gravity advance, so the listed Y values are the pre-drag first-tick algebra, not a measured trajectory. At matched native-1920 state, the same drag and 2/3 projection preserve that equivalence. ID51's first X advance is −.32 original px/tick or −.2133 normalized/port px/tick; ID50's is zero. Spawn offsets, RNG ages, alpha and phase prevent interpreting a composite pixel width as one of these nominal diameters.

## First divergence and implementable next gate

1. `Game/src/Game.cpp:142-143` sets `GameScale` from actual backbuffer, not JPEG dimensions. `Game/src/GameObjects/GameObjectManager.cpp:838-891` reads raw XML position/size/speed and calls `CreateDirectedSprayLoadTime`. `Game/src/Particles/ParticleEmitterManager.cpp:598-705` first consumes a discarded `rand()%100` for positive spread, then global-rand spawn offsets, integer-truncated size `N*g`, speed `-32*g`, and `scaleTo=4*g`. `Game/src/Particles/ParticleSpray.cpp:160-342` grows `StartSize×[1+(scaleTo−1)u]` after constructing current-frame vertices; `:347-365` consumes more global RNG to warm each particle.
2. `port/game/LevelScene.cpp:315-374` seeds an independent per-emitter generator and begins directly with spawn-offset draws (no discarded spread draw), then stores N, −32 and scaleTo=4. `port/game/ParticleSpray.h:43-100` applies the same broad update/growth and `:102-125` projects world size/movement by 2/3. Thus the **first procedural divergence is RNG provenance/consumption at load**, before any hypothetical scale-to divergence. A nominal coefficient mismatch occurs only under the original-1280 backbuffer hypothesis. Even identical RNG draws would make original-1280 initial diameter integer-truncated versus fractional port diameter, followed by gravity/growth divergence. No matched original-runtime pixel or per-particle vertex record exists to call one the active visible first difference.
3. **Bounded next gate:** get one original paired-raft ID50/51 run's actual backbuffer dimensions and, for each emitter, log first two particles' ordered `rand` draws (including discarded spread draw), initial offset/size/speed/lifetime, first pre-advance and next-frame vertices, and warm age. Capture matching camera/frame/load order and the corresponding port per-particle fields. First compare original at its actual resolution after 1280 normalization; only if the backbuffer is 1280 and the coefficient mismatch survives aligned state should a separate scoped scale-to/size/gravity correction be designed. If original is 1920, focus on RNG and update alignment, **not** adding GameScale to the port. This is a finite numerical instrument/compare gate, not a request for another qualitative still-image review.

The earlier `port/PRIVATE_WATCHDOG_TOP_ID50_51_SPLASH_VISUAL_GATE_20261024T0501Z.md` establishes the current 20-second packet's active paired-raft state and why its unmatched JPEG/composite cannot isolate ID50/51 pixels. This follow-up changes no source, status, tests, build, GPU/Xorg state, or other reports. Existing unrelated dirty files were preserved. No matched pixel parity or Jerome acceptance is claimed. Ownership of this read-only gate is released.

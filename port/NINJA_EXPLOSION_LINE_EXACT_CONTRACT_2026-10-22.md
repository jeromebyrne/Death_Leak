# Ninja explosion-line exact source contract (read-only, 2026-10-22)

Scope: the five `explosion_lines.png` particles only. Source is `Game/src/GameObjects/NPC.cpp:541-565`, `Game/src/Particles/ParticleEmitterManager.cpp:380-535`, `Game/src/Particles/ParticleSpray.cpp:160-310`, and `Game/src/program.cpp:114-117,147-175`; current port is `port/game/Combat.cpp:175-182,219-231,571-582`. No build or image comparison was performed.

## Spawn values, in emitter insertion order

`CreateDirectedSpray(5, position, kGroundFront, direction=(0,0), spread=.1, ..., speed=3.5..5, lifetime=.3..7 seconds, size=32..128, gravity=0, loop=false, brightness=1, scalesByLiveTime=true, scaleTo=16, spawnSpreadX=spawnSpreadY=0, fadeIn=.05, fadeOut=.1)`.

- Lifetime is **not sampled randomly**: `L_i = .7 - ((.7-.3)/5)*i`, `i=0..4`, hence **.70, .62, .54, .46, .38 seconds**. The minimum `.30` is not attained. Current port uses `.30, .40, .50, .60, .70` (reverse ordering and different middle values).
- Per particle, `r_i = (rand()%10)*.01` in `{0,.01,...,.09}`; `Direction=(r_i,-r_i)`. Thus zero motion is possible; otherwise source-world motion is **right and down** (world Y decreases). No negative X or positive Y component. Source direction is not normalized.
- `Speed_i = (3.5 + .01 + (rand()%150)*.01)*gameScaleX`, i.e. discrete `3.51..5.00` source distance units per target tick at game scale 1. With `targetDelta=1/60 s`, update displacement is `(r_i*Speed_i, -r_i*Speed_i)*(dt/targetDelta)`, or velocity `(60*r_i*Speed_i,-60*r_i*Speed_i)` world units/s at scale 1. **Do not multiply the scalar speed by 60 before accounting for the small direction component.** Axis speed is `0..27` world units/s; for nonzero spread it is `~2.106..27` depending on the random draws. Port instead uses `((i-2)*.025+.04)*240` and `-(i-2)*.025*240` units/s, introducing left/up directions and different component ranges.
- `StartSize_i = (32+1+rand()%96)*gameScaleX`: integer `33..128` at 1920×1080 (`gameScaleX=1`). Each particle's quad is square, with initial full width/height `StartSize_i`; the vertices use `Size/2` as half-extent. The texture's native pixel dimensions do not determine this size. Source stores `m_scaleTo=16*gameScaleX`, so its final-size target is `StartSize_i*(16*gameScaleX)` (the extra scale multiplication matters off the reference resolution). Port's fixed samples are `40,60,80,100,120` and are currently shrunk in the last `.1 s`.

## Update/render formulas and timing

Let `a` be `CurrentTime` after this update, `u=a/L_i`, `q=1-u`. `ParticleSpray::Update` first writes this frame's quad vertices from **pre-update position and Size**, then advances position with `dt/(1/60)`, increments `CurrentTime` by `dt`, writes vertex alpha, and updates stored Size for the *next* update. With zero gravity and no resistance, the position recurrence is exact as above. There is therefore a one-update geometry lag relative to the new age/motion; a clean continuous-time port can use the formulas below, but frame-exact comparison should account for that lag.

- Stored size after update: `Size = StartSize + (StartSize*(16*gameScaleX)-StartSize)*u` (no clamp in source). The visible quad this update uses the previous `Size`. This is growth over the full lifetime, **not a late shrink**.
- Alpha is initialized to 1 for every update. Code tests fade-out **first**: if `q < 1-.1 = .9` (equivalently `u>.1`), `alpha=q/.9`. Otherwise, if `u<.05`, `alpha=u/.05`; else `alpha=1`. Thus the actual source curve is **5% fade-in, 5% plateau, then 90% fade-out**, not a fade confined to the final 10%. At `u=0,.025,.05,.1,.55,1`, the formula gives `0,.5,1,1,.5,0`. Source brightness is 1, and the spray object's alpha defaults to 1; the vertex normal X carries this particle alpha.
- Source increments `CurrentTime` in seconds, marks a non-looping particle dead when `L_i-a<=0`, and removes the spray when no particles remain. Port increments `effect.age` in seconds and erases at `age>=lifetime`; its current `fade=clamp((L-age)/.1,0,1)` is both an incorrect opacity envelope and an incorrect size multiplier.

## Integration checks for the owning worker

1. Verify five particles in insertion order with lifetime sequence `.70,.62,.54,.46,.38` (tolerance for float arithmetic); none has `.30`.
2. At reference game scale, verify every direction has `vx>=0`, `vy<=0`, `vx=-vy`, and absolute axis velocity in `0..27` units/s; test zero-spread and upper-spread endpoints plus non-unit `dt` for the `60*dt` tick conversion.
3. Given a chosen start size `S` and lifetime `L`, verify full size `S` initially, `S*8.5` at half life, `S*16` at expiry as the continuous target; verify size never contracts. If matching source frames rather than continuous intent, verify the one-update lag explicitly.
4. Verify alpha samples at normalized ages `0,.025,.05,.10,.55,1` are `0,.5,1,1,.5,0`, with `u=.11` already in fade-out. Do not interpret `.1` as the last-10%-of-life fade interval.
5. Preserve per-particle texture, square geometry, brightness and ground-front painter layer; validate source-to-port world-Y inversion at rendering (negative source-world Y velocity moves downward on screen).

Uncertainty/limits: `rand()` sequence is global and cannot be reproduced from this local call alone; deterministic test samples should check the discrete admissible ranges and formulas, not claim exact gameplay random draws. `ParticleSpray` writes vertices before advancing state and may handle its death frame differently from a port that erases before rendering. No GPU or captured-frame evidence establishes the precise final-frame visibility or pixel-level blending. This report corrects the earlier QA shorthand “3.5–5 per tick” by distinguishing scalar speed from its much smaller directional component, and clarifies that the source's `.1` fade-out parameter causes a 90%-of-life fade due to the implemented condition.

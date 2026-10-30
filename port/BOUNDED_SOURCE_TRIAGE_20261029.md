> Pre-repair read-only baseline at22b33d4. Current repair dispositions, final earned evidence and residual limits are in [Linux completion checkpoint](LINUX_EARNED_COMPLETION_CHECKPOINT_20261029.md). This preserves the original evidence rather than relabeling source inspection as runtime proof.

# Bounded original-versus-Linux fidelity triage — 2026-10-29

## Scope and evidence standard

Read-only independent review of `/srv/openclaw/workspace/death_leak_analysis/keyboard_mouse_wt`, HEAD `22b33d4`, and its retained original `Game/src` and authored `Game/XmlFiles/levels`. Baseline: `port/ALL_LEVEL_REFERENCE_AUDIT_20261029.md`. This report is the only file written by this reviewer; no checkout, tracked/source/build mutation, build, gameplay launch, save load, or Windows mutation was performed. Initial worktree status was clean. Line numbers below refer to the inspected source, not future parent edits.

These are source-demonstrable behavioral differences, not synchronized original/runtime reproductions. Priority reflects campaign exposure and likely impact, not proof of the cause of any recorded replay death. P1 = core encounter behavior; P2 = narrower collision/NG+/animal interactions; P3 = presentation. Deterministic RNG replacement alone is distinguished from missing state-machine behavior.

## Highest-value actionable discrepancies

### T01 — P1: Ground NPCs can wind up/fire while retreating or following; original explicitly stops during windup

- **Original:** `Game/src/AI/AIStateRangeAttack.cpp:60`–75 stops acceleration and X velocity, decrements the shot countdown, fires, then returns. Lines 112–150 enter the firing phase through a 0.5-second hold after entering the desired/follow band; decisions use full squared XY distance (118,127) and camera visibility (135).
- **Port:** `port/game/Combat.cpp:684`–708 moves by a horizontal-only 200/500 band every tick, before the windup branch at 715–721. A new windup only checks cooldown and the broad `abs(dx)<=700 && abs(dy)<=500` rectangle. Thus a ninja at horizontal range 100 retreats while preparing its shot; a ninja at 650 follows while preparing its shot. Original sets X velocity to zero throughout preparation and does not initiate through that broad rectangle.
- **Campaign exposure:** every ninja encounter, including `Game/XmlFiles/levels/village_central.xml:2443` trigger151 and `:2451` trigger152, and `grass_exploration_2.xml:1671` trigger101.
- **Minimal fix:** process active windup before movement; add the original hold/band-entry phase and full XY distance gating, retaining fixed RNG values initially if needed. Do not just tune fire cooldown to compensate.
- **Focused verification:** trace x, acceleration, hold state, windup and shot emission for stationary players at 100/350/650 horizontal distance and a large vertical separation. Assert no X movement during active windup and no initiation from the wrong state.

### T02 — P1: Ghost height/follow behavior removes original hold, camera clamp and inertia

- **Original:** `Game/src/AI/AIStateFlying.cpp:25`–31 initializes a target; :48–62 retargets height only after two seconds and only while the player is grounded, then clamps to camera top minus170. :70–84 uses a +/-10 height deadband, acceleration and velocity damping; :87–99 uses an X band and 0.99 velocity damping. :42–45 checks the fire gate before those movements.
- **Port:** `port/game/Combat.cpp:663`–670 assigns `targetY=playerY+300` every tick and directly changes position, with no grounded retarget gate, timer, camera-top cap, acceleration or inertia; X motion clamps at +/-250. Fire is checked after movement at :711–713. Fixed offsets are not the only discrepancy: even with a matched random draw, the state evolution differs.
- **Campaign exposure:** `Game/XmlFiles/levels/grass_exploration_2.xml:1679` trigger102 spawns two ghosts; other ghost triggers share this implementation.
- **Minimal fix:** add ghost target-age/current-target state, grounded-only retarget and camera cap; evolve velocities with source acceleration/damping instead of snapping positions. Keep original fire-before-movement order. Supply camera-top/grounded data explicitly through the gameplay boundary.
- **Focused verification:** hold player grounded, then jump/fall for >2s under a low camera ceiling; compare target retention, upper limit and alignment/fire timing. Do not claim ghosts categorically never fire: player motion can still bring the port inside its45-unit fire gate.

### T03 — P2: Ninja teleport conditions omit vertical recovery and out-of-view boundary recovery

- **Original:** `Game/src/AI/AIStateRangeAttack.cpp:80`–90 teleports a noncrouching NPC when it cannot accelerate past an offscreen level boundary. :97–108 also permits teleport when the player is >500 above and has remained grounded >1s. The exact source expression has asymmetric precedence: `(npcInView && abs(dx)>600) || (dy>500 && grounded && groundTime>1)`; it is not a symmetric `abs(dy)` test. :211 onward chooses front/behind placement using player velocity, or facing only when stationary.
- **Port:** `port/game/Combat.cpp:675` only checks `abs(dx)>600` plus timer; :676 positions using facing, with no view, vertical or boundary condition. This can teleport an invisible far-horizontal ninja unnecessarily and fail to recover a ninja directly below the player.
- **Minimal fix:** port the exact guards and player-velocity placement, preserving skeleton `CanTeleport=false`. Add grounded-duration and camera/bounds inputs. Preserve/explicitly document the original asymmetric expression rather than silently modernizing it.
- **Focused verification:** same-X player +550Y grounded >1s; offscreen left/right NPC boundary; horizontal separation >600 in/out of view; facing opposite actual movement.

### T04 — P2: Ground enemy solver bypasses original hard-edge stops

- **Original:** `Game/src/GameObjects/MovingSprite.cpp:115`–141 applies the current supporting strip's hard left/right edge plus offset to **all Characters**, not only Player, stopping velocity/acceleration. NPC inherits that movement. The supporting-strip identity is essential.
- **Port:** `port/game/Combat.cpp:98`–140 handles vertical walls, solid boxes and floors but never tests line hard-edge flags/strip bounds; the player hard-edge repair does not automatically cover this independent solver.
- **Campaign exposure:** village-central contains ninja triggers151/152 and authored hard-right-edge strip27 (20-unit offset). Exact encounter routing onto that strip is not proven here; the shared solver omission is certain. Rabbits in the two campaign rabbit scenes do not have authored hard-edge strips, so do not sell rabbit edge reaction as a demonstrated campaign blocker.
- **Minimal fix:** retain supporting-strip identity in Enemy and reuse the source-derived strip-wide hard-edge bound calculation before applying desired X. Do not treat each tessellated segment endpoint as a hard edge.
- **Focused verification:** grounded ninja crossing a flagged strip edge with20 offset versus unflagged internal segment junction.

### T05 — P2: Bomb terrain contact only detects downward floor crossings, not original forward/previous-ray or vertical-strip contacts

- **Original:** `Game/src/GameObjects/SolidLineStrip.cpp:459`–507 checks forward projectile ray, previous-frame ray fallback, then top-to-bottom collider ray for each candidate segment. `Game/src/GameObjects/BombProjectile.cpp:122`–126 delegates to that query. A successful contact zeros velocity/gravity and starts the inactive fuse.
- **Port:** `port/linux/main.cpp:1487`–1494 skips segments with `abs(x2-x1)<1` and only accepts bottom crossing a sampled top. A bomb shot horizontally into an authored vertical solid-line wall can pass through rather than starting its fuse; upward/oblique segment contacts are also not represented by the floor test. Solid-box overlap at :1496 is a different representation and does not rescue vertical line strips.
- **Campaign exposure:** bomb unlock is NG+ gated; this is a real campaign-mechanic difference after unlock, not a first-run replay explanation.
- **Minimal fix:** use a dedicated renderer-neutral bomb/strip ray-contact query matching the three original rays. Supply old X as well as old Y. Preserve water exclusion and the contact-only five-second fuse; do not add a launch TTL or speculative bounce.
- **Focused verification:** vertical strip, sloped segment approached from above/below, previous-frame ray crossing, submerged crossing. Assert first inactive time and strict `now>inactiveAt+5` explosion.

### T06 — P2: Explosion omits source projectile/debris impulse

- **Original:** `Game/src/GameObjects/Explosion.cpp:70`–98 applies radial force to visible debris/projectiles within radius: add normalized direction times10 to X velocity and times15 to Y, in source units/tick. :102–110 runs display/damage/force on the explosion's second update.
- **Port:** `port/game/Combat.cpp:368`–373 only damages enemies; dispatcher `port/linux/main.cpp:1505`–1513 damages breakables/emits art/shakes. No projectile, bomb, blood-flight or debris impulse is applied.
- **Minimal fix:** expose one typed explosion-force pass for eligible dynamic bodies. Convert +10/+15 tick velocity to +600/+900 for port shot collections stored in units/second, but do not apply that conversion to tick-unit bomb bodies. Preserve visibility/radius/type filters and exclude Player. Schedule using explicit explosion event age if matching the source two-update timing.
- **Focused verification:** nearby knife/enemy projectile and debris get outward component deltas; outside radius/offscreen bodies and Player do not. Capture object-order semantics before adding chain interactions.

### T07 — P2: Bomb damage and camera shake lack original visibility gates

- **Original:** `Game/src/GameObjects/Explosion.cpp:30` obtains `GetSolidSpritesOnScreen`; `Game/src/GameObjects/GameObjectManager.cpp:1079`–1093 explicitly filters `Camera2D::IsObjectInView`. Bomb shake is conditional on the bomb being in view (`Game/src/GameObjects/BombProjectile.cpp:62`–64).
- **Port:** `port/game/Combat.cpp:369`–371 damages every retained enemy in radius; `port/linux/main.cpp:1507`–1509 similarly damages every scene breakable in radius, and :1512 always starts camera shake.
- **Minimal fix:** pass source-equivalent drawable visibility into blast damage dispatch; separately gate shake on bomb visibility. Radius is still required and is not a substitute for view gating. Use actual camera zoom/viewport, not a fixed distance heuristic.
- **Focused verification:** same-radius visible and offscreen targets, offscreen bomb with visible nearby object, multiple camera zooms. This addresses unsupported offscreen damage, not player self-damage: the port correctly does not damage Player.

### T08 — P2: Campaign rabbits and ground butterflies have no damage/death collision dispatch

- **Original:** `Game/src/GameObjects/Rabbit.cpp:13` sets0.01 health; :21–23 calls Character damage. `Game/src/GameObjects/butterfly.cpp:24`–40 removes ground butterflies and spawns two original wing debris assets; non-ground butterflies are explicitly immune. `Game/src/GameObjects/Projectile.cpp:108`–114 damages a contacted butterfly out of water and returns without ordinary projectile impact. General NPC projectile ownership filtering remains relevant (:162–166).
- **Port:** `port/game/AuthoredAmbientActor.h:17`–22 explicitly excludes damage/death/debris; `port/game/LevelScene.cpp:1150`–1181 only updates pose/animation. Combat shot targeting and `ApplyExplosion` use `enemies_`, not scene ambient actor bodies. There is no corresponding damage bridge for these actors.
- **Campaign exposure:** four Rabbit objects: `Game/XmlFiles/levels/catacombs_entrance_1.xml:409` (id25), :437(id26), `inside_hanging_shadow.xml:891`(id58), :919(id59); two ground Butterflies: `hot_springs.xml:549`(id32), :577(id33). These are actual authored campaign actors, not just prototype evidence.
- **Minimal fix:** add a scene-owned damageable-ambient view/dispatch, preserving actor identity, collider/offset, ownership, depth and water exclusions. Retire/deactivate on damage before rendering; use original dead-wing assets. Keep motion helper renderer-neutral. Add corpse/debris behavior separately rather than relabeling debris as collectible rewards.
- **Focused verification:** knife through ground butterfly removes it but preserves projectile continuation; background butterfly remains; player projectile kills rabbit; enemy projectile does not hit NPC animals; blast uses same visibility gate as T07.

### T09 — P3: Bomb feedback uses one-shot nonpositional fuse and fixed bang; no blast rumble event

- **Original:** `Game/src/GameObjects/BombProjectile.cpp:45`–49 owns looping positional `fuse_burning.wav` (600 dimensions,3000 fade dimensions). `Game/src/GameObjects/Explosion.cpp:110` requests vibration0.8/1.0 for0.2s; :128 onward chooses bang/bang2/bang3. Bomb destruction owns fuse lifetime.
- **Port:** `port/linux/main.cpp:1473` plays fuse once via `PlayFile`; there is no bomb-owned positional voice to follow/stop. `port/game/LevelScene.cpp:1395` always queues bang.wav; blast dispatch at main:1505–1513 has no dedicated vibration call.
- **Minimal fix:** reuse existing positional-audio owner infrastructure with per-bomb identity and cleanup on explosion/scene reset; randomized bang variant can use local RNG without claiming identical global stream. Emit the explosion vibration through the established renderer/input interface if supported; otherwise document device-feedback capability as unavailable.
- **Verification:** mixer/voice lifetime and positional attenuation diagnostics first, then coordinated audible/physical feedback checks. A source call alone is not speaker/controller acceptance.

## Explicit non-findings / bounded evidence

- **Do not reopen repaired core bomb physics:** `port/game/BombProjectileMotion.h` already preserves source water drag/gravity, five-second fuse beginning on first contact, strict expiry, blade explosion and nonblade deactivation. Source sets bounce flags but zeros velocity before ordinary collision response; that flag alone is not evidence bombs should visibly bounce.
- **RNG is not full behavioral proof:** fixed spawn offsets, range offsets, jump/reload/teleport times and ghost speed are documented approximations (`Combat.cpp:594`, :673, :692, :748). They cannot certify source RNG/frame parity, but replacing them alone will not fix T01–T03's missing states/order/gates. No exact original `rand()` trace was recovered or replayed here.
- **ActiveBird death is real source functionality but not current campaign exposure:** `Game/src/GameObjects/ActiveBird.cpp:15`–56 emits dead bird, blood and feathers; the only ActiveBird in the39 current definitions is unlinked `Game/XmlFiles/levels/test.xml:793`, id38. Keep that outside campaign-critical counts. AmbientBird is not interchangeable with ActiveBird.
- **Animal sidewall/water/overlap response remains uncertified:** helper explicitly excludes these. This review demonstrates absent damage dispatch, but did not establish a specific authored rabbit-wall/water trajectory; do not inflate the boundary into a reproduced campaign collision failure. Rabbit hard-edge callback is unwired (`AuthoredAmbientActor.h:63`, `LevelScene.cpp:1152`–1174), but campaign rabbit levels have no enabled authored hard edges.
- **No contact body damage invented:** original `Character.cpp:595`–602 skips ordinary physical displacement for another Character; enemy/player overlap alone is not grounds to add damage or knockback.
- **Swept projectile queries differ from original strict discrete AABB:** original `Core/CollisionManager.cpp:100`–103 is strict AABB; the port intentionally sweeps several projectile paths. This is not automatically a campaign defect; no concrete adverse authored encounter was demonstrated in this review. Do not undo anti-tunneling wholesale.
- **Sprint bomb speed not escalated:** helper supports15/20 launch speed while caller defaults15; port sprint availability was not established as campaign content here. It is not included as a demonstrated campaign bug.
- **No replay causal claim:** T01–T04 can change earned-route difficulty, but source inspection does not prove any one caused the historical trigger151 death. No tests, full earned ending, synchronized frame comparison, physical display, audible sound or remote-input acceptance were executed by this reviewer.

## Recommended bounded parent implementation order

1. T01 with state traces, then T03; these are the most direct core ninja encounter fixes. Preserve earned progression/damage rules.
2. T02 ghost state integration separately, then T04 supporting-strip edge logic.
3. T05–T07 as one typed bomb-contact/explosion boundary change with unit-explicit tests; maintain NG+ purchase gating and persistence snapshot timing.
4. T08 ambient damage bridge; do not conflate unlinked ActiveBird with campaign animals.
5. T09 feedback and a precise limitation ledger for device/audio proof.

No request to change Windows originals, rewrite the renderer, globally alter RNG, weaken replay survival checks, or certify complete engine parity is implied.

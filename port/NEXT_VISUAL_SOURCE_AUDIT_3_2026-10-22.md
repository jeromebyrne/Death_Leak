# Next source-proven forest gameplay visual gap: projectile spawn fade/scale (2026-10-22)

## Verdict

The port draws newly fired player knives immediately at **full opacity and full 84.5 × 25 world-unit size**. Windows `Projectile` instead starts at zero alpha and, while active, grows **both alpha and dimensions linearly over 0.3 s**. This is a distinct, repeated moving visual in the held-fire forest route, separate from the firing arm, ground shadow, and aim line. It is a source-semantic priority, **not** a claim of measured pixel difference in unmatched frames.

## Exact evidence

- Forest Player ID2 is authored in `Game/XmlFiles/levels/forest_exploration_1.xml:33-47`. `Player::FireWeapon` constructs the knife with `kPlayerProjectile`, dimensions `(84.50,25.0)`, and texture `Media/knife.png` or upgraded `Media/knife_2.png` (`Game/src/GameObjects/Player.cpp:128,425-463`).
- The Windows `Projectile` constructor calls `SetAlpha(0.0f)` (`Game/src/GameObjects/Projectile.cpp:25-67`), and `mTimeActive` starts at zero (`Game/src/GameObjects/Projectile.h:130`). `GameObject::LoadContent` retains its initial dimensions as `mOriginalDimensions` (`Game/src/GameObjects/GameObject.cpp:129`). On each active update, `Projectile::Update` increments the age and, for age below `kFadeInTime = 0.3f`, applies `alpha = age/0.3` **and** `dimensions = originalDimensions * age/0.3`; afterward both reach full values (`Game/src/GameObjects/Projectile.cpp:20,425-445`). The Windows draw manager skips zero-alpha drawables (`Game/src/GameObjects/GameObjectManager.cpp:302-329`). The `0.25f` passed at creation is the *inactive* expiry interval, not the active flight lifetime (`Game/src/GameObjects/Projectile.cpp:25-40,506-543`), so it must not be used to cap this fade.
- The port creates a `Shot` on eligible fire with `life=0.25f` and a knife texture (`port/game/Combat.cpp:219-228`); `Shot` has fixed `visualWidth=84.5f`, `visualHeight=25.f` and no visual-age field (`port/game/Combat.h:36-50`). `Combat::Draw` sends every surviving shot to a helper at those fixed dimensions with default sprite tint/full alpha (`port/game/Combat.cpp:488-505`). `Combat::Update` removes shots when their port lifetime ends (`port/game/Combat.cpp:369-441`). The port draws combat after the scene (`port/linux/main.cpp:1103-1112`), so this proposal addresses only each projectile's own fade/scale, not overall depth-order parity.

## Bounded implementation

Track age from spawn independently of collision/lifetime. For each active player knife, use `t = clamp(age / 0.3f, 0, 1)` for alpha and *both* width and height, centered at the shot's existing projected position, while preserving its selected knife texture and current horizontal flip. Apply the same source behavior to other `Projectile`-backed shots only after checking their construction/update path; do not add the commented-out projectile-trail emitter (`Game/src/GameObjects/Projectile.cpp:544-579`). Keep the port's current 0.25 s gameplay lifetime as a separate issue rather than silently changing combat in a visual patch. Verify at deterministic shot ages (spawn, ~0.15 s, >=0.3 s if a shot survives) and during the existing held-fire forest route; matched Windows timing/camera are needed for pixel parity.

## Limit

This audit uses source/port code only. No game execution, GPU/build/capture, source edit outside this report, shared checkpoint update, commit, or push was performed.

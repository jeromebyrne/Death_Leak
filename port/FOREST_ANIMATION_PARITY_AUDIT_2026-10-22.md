# Forest 1 animated-frame parity audit — 2026-10-22

Scope: read-only comparison of authored `forest_exploration_1` animated sprites and their Windows/Linux frame selection and timing. No game run or visual capture. Excludes parallax, repeat/flip, fog, rain particles, ID37, and raft/exit-shaft work.

## Authored animation population

`Game/XmlFiles/levels/forest_exploration_1.xml` has animated Player **ID 2** (`:33-46`) and CurrencyOrb IDs **66, 68, 70, 72, 80, 84–86, 88, 90, 92, 94, 96, 100, 102, 104–105, 107, 112–115** (e.g. ID 68 at `:1110-1123`). No other authored `isanimated=true` objects were found in this level. The orbs use `XmlFiles/orb_anim.xml` body/Still, ten ordered frames at 10 fps (`Game/XmlFiles/orb_anim.xml:3-15`).

## Confirmed divergence 1 — orb animations keep advancing outside the Windows update zone

Windows `GameObjectManager::Update` skips non-updateable objects and only updates non-`always_update` objects while intersecting the camera-centered update rectangle (`Game/src/GameObjects/GameObjectManager.cpp:221-247`); the rectangle starts at 4000×4000 world units (`:397`). The authored CurrencyOrbs are `updateable=true`, `always_update=false` (ID 68 example: level XML `:1110`; likewise all orb IDs above). `Sprite::Update` invokes `UpdateAnimations` (`Game/src/GameObjects/Sprite.cpp:358-366`), whose default body/Still path advances and selects the texture (`:869-885`); `AnimationPart::AnimateLooped` accumulates update delta and advances at most one frame per update (`Game/src/Animation/AnimationPart.cpp:160-196`). Thus Windows orb frame clocks stop outside that zone and resume where they stopped.

Linux correctly reads the Still frame list/rate (`port/game/LevelScene.cpp:86-103,420-436`) but advances each non-collected sprite's frame every `UpdateVisuals` call, without the `updateable`/`alwaysUpdate`/camera-zone test applied to its scrolling and occlusion logic (`port/game/LevelScene.cpp:655-731`, especially `:668-729`). Draw uses the resulting indexed frame (`:862-867`). Therefore an orb that has spent time outside the update zone re-enters on a later frame in Linux than in Windows. This is a source-confirmed frame-timing and texture-selection divergence; its in-view impact depends on camera path and elapsed time. The at-most-one-frame update behavior *within* the zone matches.

## Confirmed divergence 2 — Player falling-frame transition omits source delay

The authored Player ID 2 uses `XmlFiles/animation/player_anim_default.xml` (level XML `:33-46`), with separate `JumpingUp` and `JumpingDown` body sequences (`Game/XmlFiles/animation/player_anim_default.xml:524,603`). Windows records the time when downward movement first begins (`Game/src/GameObjects/Character.cpp:302-310`) and, in `Character::UpdateAnimations`, retains the current body frame until **0.05 s** has elapsed, unless time off solid surface exceeds 0.25 s or the player is in water (`Game/src/GameObjects/Character.cpp:22,922-959`). After that it selects/advances `JumpingDown`. Linux chooses `JumpingDown` immediately whenever airborne and no longer ascending (`port/game/LevelScene.cpp:733-765`, specifically `:757-761`), with no equivalent descent-start timer. At an ordinary jump apex, Linux can therefore show the first falling frame up to about 50 ms earlier than Windows. This conclusion is about code-path selection, not a measured screenshot mismatch.

No code change proposed in this audit. The two differences above are bounded animation-frame parity candidates for the port owner; source inspection alone does not rank their visual prominence.

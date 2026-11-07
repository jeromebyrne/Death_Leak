# DrawDamageBurst rotated-flip corner contract — 2026-10-22

## Verdict and scope

Read-only CPU/source audit; no source/build/GPU/shared-status changes. `DrawDamageBurst` is a custom GL quad, but its UV-labeled geometry has **the same angle requirement as an ordinary SDL copy**: for authored X/Y flips, `phi = -(sx * sy) * theta`, with `sx = flipX ? -1 : +1`, `sy = authoredFlipY ? -1 : +1`, and `theta` the Windows authored radians. It rotates by `+sprite.rotationDegrees`; there is **no foliage-style extra minus to remove**. The shared all-four-flip helper now visible in the concurrently edited working tree supplies this required angle, and `LevelScene::Draw` now calls it with `object.flipY`. The earlier X-only helper was correct for no flip and X-only, but wrong for Y-only and both-flipped nonzero angles. This is a source-coordinate conclusion, not a GPU or original-Windows pixel match.

## Exact UV-labeled derivation

Windows `Game/src/GameObjects/Sprite.cpp:19-22,89-96,194-223` binds `U,V = (0,0),(1,0),(1,1),(0,1)` to local BL, BR, TR, TL, then gives authored flips negative X/Y scale. `Game/src/GameObjects/GameObject.cpp:202-210` constructs row-vector World with rotation before that scale; `Game/ShaderFiles/BasicLightTexture.fx:42-55` transforms each vertex with World and retains its UV. In screen-Y-down coordinates, for a source UV label `(u,v)`, define

`q(u,v) = ((u - 1/2) * w, (1/2 - v) * h)`, `F = diag(sx,sy)`, and `A(a)(x,y) = (cos(a)x - sin(a)y, sin(a)x + cos(a)y)`.

Relative to the destination center, the Windows screen position is `W(u,v) = F A(-theta) q(u,v)`. Width/height here are destination dimensions (the common 2/3 world-to-logical scale cancels from the angle equation). The port's `LevelScene.cpp:997-1012` uses `draw.flipX` (player facing for the player, otherwise authored X) and, for an ordinary actor, `draw.flipY = !object.flipY`, converting D3D's V=0-at-screen-bottom to SDL texture orientation. `DrawDamageBurst` in `port/linux/SDLRenderer.cpp:253-306` swaps U endpoints according to `sprite.flipX` and V endpoints according to this **inverse-authored** `sprite.flipY`, then rotates the four screen corners by `+phi`. Therefore the corner carrying each source UV label is `D(u,v) = A(phi) F q(u,v)`. The V inversion is already performed in `draw.flipY`; passing it as if it were authored Y to the angle helper would be wrong. Algebra gives `D=W` exactly when `phi = -sx*sy*theta`.

| Authored X | Authored Y | `draw.flipY` for ordinary burst actor | Required `phi` | Old X-only `phi` |
|---|---|---|---|---|
| false | false | true | `-theta` | `-theta` (right) |
| true | false | true | `+theta` | `+theta` (right) |
| false | true | false | `+theta` | `-theta` (wrong) |
| true | true | false | `-theta` | `+theta` (wrong) |

The actual GL loop emits screen corners in TL, TR, BR, BL order; it obtains UVs from `left/right/top/bottom` source-rectangle endpoints (`SDLRenderer.cpp:265-270,293-305`). Its `textureSpan` uniform scales the sample coordinate in the fragment shader (`SDLRenderer.cpp:563-575`), not the vertex position. For a cropped source, use the four endpoint UV labels instead of literal 0/1; the same geometry equation holds. RGB gain/tint/opacity are fragment/color concerns, not rotation corrections. `DrawSprite` selects burst first when `rgbGain > 1` and the shader exists (`SDLRenderer.cpp:207-214`), even ahead of wobble/foliage; otherwise it falls through to those/ordinary paths.

Concrete synthetic nonzero-angle discriminator: for `w=120`, `h=80`, `theta=+0.3 rad`, and label `UV(0,0)` (`q=(-60,+40)`), the Windows-relative corner coordinates are approximately `(-45.499,+55.945)`, `(+45.499,+55.945)`, `(-45.499,-55.945)`, `(+45.499,-55.945)` in the table's parity order. The old X-only angle instead yields `(-69.141,-20.482)` for Y-only and `(+69.141,-20.482)` for both, an approximately `42.621` logical-unit UV-corner error for either (using those chosen logical dimensions). The new all-flip angle yields the Windows coordinates for all four. This is a corner test vector, not a predicted visible-pixel displacement.

## Authored example / reachability

No authored **burst-eligible nonzero-angle actor** was found: an XML parse of all `Game/XmlFiles/**/*.xml` found 51 `<Player>` elements and zero with nonzero `<position rotation>`. `port/tests/fixtures/damage_burst.xml` likewise has a zero-angle player. `port/game/Combat.cpp:499-515` creates the transient enemy burst sprite without assigning `rotationDegrees`, so that path defaults to zero. Thus the four-parity rotated case is a **latent branch contract**, not evidence of a currently visible authored player-hit rotation bug. A synthetic nonzero-angle control is required; do not cite a rotated fog/foliage sprite as a damage-burst instance. The player source X flip can change with facing, but that alone does not expose an angle-sign error at theta=0.

## Minimal owner test/fix recommendation

1. Keep the all-four-flip `OrdinarySpriteRotationDegrees(theta, draw.flipX, object.flipY)` now present in `LevelScene.cpp:1011-1012`; the helper currently returns the table above. Do not negate `sprite.rotationDegrees` inside `DrawDamageBurst`, and do not replace its `sprite.flipY` UV swap with authored Y. If these concurrent edits are revised, preserve this angle/UV combination.
2. Add one **CPU-only UV-labeled corner test** of the *actual burst quad construction*, ideally by extracting only the tiny pure corner/UV generation loop from `DrawDamageBurst` (or by a line-for-line independent probe if extraction is undesirable). For each of four authored X/Y parities, use asymmetric 120x80 dimensions, theta=0.3, center offset, and optionally a non-full source rectangle; set `sprite.flipY = !authoredFlipY`. Assert every source endpoint UV label's position against `F A(-theta) q` with a small float epsilon. Include a wrong-sign control for Y-only and both so the test would fail under the old X-only helper. Assert that existing no-flip/X-only cases remain correct.
3. `port/tests/damage_burst_test.cpp` currently verifies gain lifecycle/alpha/neighbors, not rotation or corner identity. Extend that focused test or add a narrow new CPU test; no GPU required for this coordinate contract. If an integration frame is later captured, use a synthetic rotated actor because current authored actors do not exercise the disputed parity.

The concurrently edited working tree was observed with the new three-argument angle helper and corrected foliage sign; this audit did not edit or validate those changes by building/running tests. Preserve the renderer owner's in-progress work and verify the final merged state independently.

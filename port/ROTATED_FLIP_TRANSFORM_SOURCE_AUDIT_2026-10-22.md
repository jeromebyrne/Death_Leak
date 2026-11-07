# Rotated/flip transform-order source audit — 2026-10-22

## Verdict

The targeted **X-flipped, rotated** sprites do **not** demonstrate a transform-order discrepancy: waterfall-top raft IDs 32/33 and forest-1 shaft ID14 have the same textured-corner positions under the Windows `R*S*T` row-vector world matrix and SDL's local X flip followed by its positive screen rotation. The earlier `RAFT_EXIT_PARALLAX_SOURCE_AUDIT_2026-10-22.md` claim that Windows rotates/translates the center is corrected by `RIGHT_SHAFT_PARITY_2026-10-22.md`; the center stays at its authored position.

A narrower source-backed discrepancy **does** exist for forest-1 shaft ID15, which is rotated but **not** horizontally flipped. With the conventional D3DX row-vector Z-rotation and Y-up-to-screen-Y-down projection, its Windows textured quad rotates in the opposite screen direction from the current `SDL_RenderCopyExF` call, which receives the same signed angle. This is a mathematical/source-path prediction, **not** a measured original-Windows screenshot difference or a reason to edit the XML/art. The existing qualitative Steam still and Linux-only captures do not establish pixel parity.

## Sources and authored targets

- `Game/src/GameObjects/Sprite.cpp:19-22,215-223` assigns U/V 0 at local world bottom-left and centered quad vertices; `:86-97` turns authored horizontal flip into negative `m_matScaleX`. `Game/src/GameObjects/GameObject.cpp:160-164,202-210` resets `m_world` to identity and left-multiplies translation, then scale, then D3DX Z rotation, yielding `R*S*T`. `Game/ShaderFiles/BasicLightTexture.fx:37-46` multiplies row positions by World, View, Projection. `Game/src/Core/Camera2D.cpp:25-30,122` uses a translation-only view and orthographic projection. Thus rotation/reflection affect local corners, not the translated center.
- `port/game/LevelScene.cpp:995-1011` forwards authored X flip, inverts the ordinary sprite's vertical texture mapping to account for D3D V=0 at world bottom, and sends authored radians as same-signed SDL degrees. `port/linux/SDLRenderer.cpp:190-193,244-250` uses `SDL_RenderCopyExF` with a null (center) pivot and SDL flips. `port/render/ViewportCull.h:10-24` uses an angle-absolute AABB, so it does not detect rotation-sign error for a centered quad.
- `Game/XmlFiles/levels/waterfall_top.xml:587-616`: raft ID32 `(−92.014648,2790)`, 674.042114×184.308380, +0.180110 rad, X flip true; ID33 `(803.985352,2813)`, same size, −0.279275 rad, X flip true. Both use `raft.png`, no Y flip.
- `Game/XmlFiles/levels/forest_exploration_1.xml:222-251`: shaft ID14 `(−2212.014648,3210)`, 512², −1.519999 rad, X flip true; shaft ID15 `(2796.985352,3207)`, 512², −1.469999 rad, X flip **false**. Both use `sun_shaft_2.png`, no Y flip.

## Explicit corner calculation

Use local **screen** coordinates `q=(x,y)`, with `y` downward, for a texture landmark before geometric transforms. Because D3D UV top-left is world-bottom-left, the source texture's top-left landmark has `q=(−w/2,+h/2)` before authored X flip. Let `C=cos(theta)`, `S=sin(theta)`, and `F=diag(−1,1)` for X flip. D3DX's row-vector Z rotation gives an unflipped Windows screen mapping `A(−theta)=[[C,S],[−S,C]]`. Since Windows `World=R*S*T`, X reflection acts **after** rotation: `F*A(−theta)=[[-C,−S],[−S,C]]`. SDL's positive angle rotates clockwise in screen coordinates, `A(theta)=[[C,−S],[S,C]]`, and its horizontal image flip maps the source landmark into the opposite local X position **before** rotation: `A(theta)*F=[[-C,−S],[−S,C]]`. They are exactly equal for every local point, not just the four corners. This also explains why a bare comparison of operation order is insufficient: X reflection conjugates and reverses the rotation sign.

For **no** authored X flip, Windows is `A(−theta)` but the current SDL path is `A(theta)`. They agree only at degenerate angles (`sin(theta)=0`) or at the center. Source texture top-left coordinates relative to the object center (world units, before the port's 2/3 output scale and camera translation) are:

| Target | Windows `(dx,dy)` | Current SDL `(dx,dy)` | SDL − Windows |
| --- | ---: | ---: | ---: |
| Waterfall-top raft 32 | `(315.061,151.037)` | `(315.061,151.037)` | `(0,0)` |
| Waterfall-top raft 33 | `(349.366,−4.319)` | `(349.366,−4.319)` | `(0,0)` |
| Forest-1 shaft 14 | `(268.668,−242.671)` | `(268.668,−242.671)` | `(0,0)` |
| Forest-1 shaft 15 | `(−280.461,−228.940)` | `(228.940,280.461)` | `(509.401,509.401)` |

These values follow directly from the cited XML angles/sizes and matrices; they are **texture-landmark positions**, not claims that opaque pixels occupy those coordinates. In the port's 2/3 viewport scale, ID15's predicted landmark displacement is about `(339.6,339.6)` screen pixels at a matched camera. Its square quad AABB and center can nevertheless look deceptively similar.

## Existing test/report coverage and bounded next action

`port/tests/forest_flip_test.sh` checks that the loader forwards center, flip flag and positive converted angle, including IDs 14/15; it does not compare to Windows transformed texture corners. `port/tests/sdl_rotated_flip_test.cpp` tests **both** horizontal and vertical SDL flips at 45°, not a lone horizontal flip or an unflipped rotation, so it cannot adjudicate this Windows-versus-SDL question. `FOREST14_COMPOSITION_ROTATION_AUDIT.md` verified that converting radians to degrees restored qualitative scene slopes, but its blanket sign-preservation explanation does not resolve the X-flip parity distinction. `RIGHT_SHAFT_PARITY_2026-10-22.md` correctly repaired the center-order claim but left flip/UV parity open.

**Next action:** make a tiny CPU-only transform-contract test for these four authored IDs comparing the four UV-labeled Windows `R*S*T` corners against SDL's mapped UV-labeled corners, with one unflipped and one X-flipped nonzero-angle control. If it confirms the equations, a renderer owner can consider an isolated unflipped-ordinary-sprite angle-sign correction, then use a matched original-Windows forest frame/ID15 isolation before accepting a visual fix. Do not invert IDs 32/33/14, move sprite centers, or alter authored XML on this evidence. No build, GPU/display run, source edit, shared status edit, or image-parity claim was made in this audit.

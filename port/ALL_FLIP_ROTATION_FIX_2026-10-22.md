# All-four-flip rotation parity — implementation and bounded verification (2026-10-22)

## Source change

Based on `ALL_FLIP_ROTATION_IMPLEMENTATION_PLAN_2026-10-22.md` and `SHADER_ROTATION_VISUAL_AUDIT_2026-10-22.md`, `OrdinarySpriteRotationDegrees` now accepts **authored** X and Y flip flags and gives the SDL/custom-quad angle `-sx*sy*theta`: neither/both flipped negate authored radians; exactly one flipped preserves the sign. `LevelScene::Draw` passes `object.flipY` rather than its intentionally inverted ordinary SDL copy flag. Ordinary `SDL_RenderCopyExF` tint, source, alpha, repeat and UV paths are otherwise untouched. Pixel wobble still uses the same UV, scrolling, fragment-stage wobble and alpha; its live vertex/UV construction was factored into `EffectQuadGeometry.h`. Foliage now rotates by the shared positive `sprite.rotationDegrees` (removing its extra negation); its BL/BR/TR/TL source intensities, X-before-Y world sway, V orientation and repeat tiling were retained, with its live vertex calculation also factored for a CPU test. The Makefile tracks the new geometry/rotation headers.

`DrawDamageBurst` is intentionally untouched pending its separate rotation-contract audit handoff; it also consumes the shared angle. No claim about every shader branch follows from this change.

## CPU/build evidence

- `ordinary_sprite_rotation_contract_test`: passed eight cases, four UV-labeled corners each, including authored forest-1 IDs 14/15, waterfall-top 32/33, underwater-cave-4 15 (both flips)/16 (Y flip), plus asymmetric synthetic Y/both controls. Independently computed Windows row-vector source-corner positions match the modeled ordinary SDL inverse-V copy within 0.001 screen unit; a deliberately opposite angle differs by >1 pixel.
- `effect_quad_rotation_contract_test`: passed all four wobble flip parities (forest-1 ID119 remains negative angle, ID122 becomes positive angle) and all four foliage parities, including waterfall-top 66/67 and an unflipped rotated control. It tests all four source vertex/UV identities; foliage zero-sway base points and distinct nonzero source-corner intensities agree with an independent Windows transform and X-then-Y displacement oracle. Opposite-angle counterfactuals fail.
- Existing `foliage_sway_test`, `forest_flip_test.sh` (including SDL software primitive), `rotated_fog_cull_test`, and `waterfall_wobble_uv_test` passed. The cull test's stale +0.11 expectation for unflipped ID117 was corrected to -0.11; its visibility checks remain unchanged.
- Reproducible Docker `deathleak-linux-build:bookworm make -C port -j2` passed twice. The host lacks SDL2 development headers, so the SDL primitive and integration checks ran inside that Docker image. Build emitted pre-existing TinyXML warnings. `git diff --check` passed.
- The full `port/tests/integration.sh` **did not pass**: it stopped at the existing arena-combat expectation `spawned=2 enemies=2 hits=6 kills=0`; the actual result was `spawned=2 enemies=1 hits=14 kills=1`. Re-running the exact arena command with the isolated unchanged HEAD baseline binary produced the **same** actual values, establishing this failure predates the rotation patch. Trace: ignored `port/build/all_flip_integration_trace.log`. This is a gate limitation, not a claim of full integration pass.

## Strict physical GPU A/B (diagnostic, not visual acceptance)

After the CPU tests/build and with no competing game, encoder or compiler process, an isolated unchanged HEAD `fcb713b` worktree and the patched tree were each built in Docker. Four sequential strict-accelerated captures used physical Xorg `:0`, LightDM read-only Xauthority, `/dev/dri`, SDL `x11`/OpenGL, `nouveau / NVE7`, dummy audio, the same original XML/input/fixed timestep, and `DEATHLEAK_RENDER_ONLY_XML_IDS` to isolate the subject. All four logs report `accelerated=1 software=0 mode=strict-accelerated`; logs/BMPs are in ignored `port/build/all-flip-rotation-gpu-2026-10-22/`.

- Forest-1 fog ID122: `--script .:60 --frames 60 --fixed-dt 0.0166667`, baseline versus fixed 1280×720 BMPs differ at **134,659 RGB pixels**, bbox `(36,440)-(1121,720)`. Baseline SHA-256 `e4abdf0895d9b2fc1de832734df5e64500b1425d5f23c212a8098bcaf068ba4d`; fixed `2e1d63cb97ca489408badeb2170a77938169a4b13b391d078792961e3c17367b`.
- Waterfall-top leaf shadow ID66: `--script R:120 --frames 120 --fixed-dt 0.0166667`, baseline versus fixed differ at **125,478 RGB pixels**, bbox `(342,313)-(1280,532)`. Baseline SHA-256 `cee1014bcd559d3b3b8df8b56fae233e9c5b07b329a692598f6df9eb9fd9aef7`; fixed `33373f59b91070eb99a3578ead913a4bcbabd825324300030530218ae6cf2ded`.

These are Linux-only, ID-isolated geometry/appearance discriminators. They do not prove Windows pixel parity, opacity/wrap correctness, full composition, or human visual acceptance. GPU lane was released after capture. **Next:** recapture an active forest-1/waterfall gameplay clip from the new binary, obtain state-matched original-Windows frames for ID122 and rotated foliage, then review the whole composite including opacity, wrap, sway phase and player/HUD before any visual-acceptance claim.

## Delivery state

Verified source/tests were committed as `c1725ee` and pushed to `origin/feature/linux-renderer-foundation` (`fcb713b..c1725ee`). This unique report and the ignored capture/log artifacts remain local and were deliberately excluded from the scoped software commit. Other workers' checkpoint/status/reports were not staged or edited.

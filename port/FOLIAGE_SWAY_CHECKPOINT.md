# Foliage-sway visual slice — 2026-10-22

Scope: one source-attributable renderer effect, `effectfoliagesway`. No Game/ source, XML, texture/art, ID37, or particle-emitter changes. The Linux port remains partial; Jerome's visual review and a matched Windows gameplay reference are pending.

## Source basis

- `Game/ShaderFiles/FoliageSway.fx` moves each world-space vertex by `sin(worldY*0.1 + Time*TimeMultiplier*GlobalTimeMultiplier) * Normal.x` in X, then computes Y from the *updated* X and `Normal.y`. Texture alpha is multiplied by the sprite alpha.
- `Game/src/GameObjects/Sprite.cpp` maps the XML `foliage_sway_props` corner intensities to quad vertices; `Game/src/Game.cpp` supplies total shader time and WeatherManager's global multiplier. `WeatherManager.cpp` normal-weather multiplier is 2.5. Forest-1 XML ID46 supplies a horizontally flipped, repeated, half-alpha leaf-shadow with top-corner intensities (7,6)/(8,9) and zero bottom intensities.

## Implementation and limits

- The scene now parses the effect and its authored four corner intensities/time multiplier, advances a scene visual clock, and passes those fields through the renderer-neutral sprite.
- SDL renders deformed textured geometry on the accelerated path. It preserves flip, alpha modulation, and existing whole-quad repeat-X phase for ID46. The pure vertex displacement is isolated in `port/linux/FoliageSway.h`.
- This is a bounded approximation: repeated texture tiles linearly interpolate the displaced quad edges, whereas the original D3D vertex shader rasterizes its two triangles; exact interior interpolation and total process-time phase may differ. The normal-weather 2.5 multiplier is fixed because rain weather is not ported. Other effects, notably ParticleSpray and pixel wobble, remain unported.

## Verification

- Established `deathleak-linux-build:bookworm` Docker `make -C port -j2` passed; only pre-existing TinyXML warnings. `port/tests/foliage_sway_test.cpp` passed the ordered X-then-Y shader formula and zero-intensity control. `git diff --check` passed.
- On physical Xorg `:0`, changed binary selected strict accelerated SDL `x11` OpenGL, `software=0`, direct `nouveau / NVE7`. Active forest-1 `R:180`, fixed-dt 0.0166667, frame 180 ended alive at `(1368.99,3107.3)` with five earned orbs. Ignored evidence: `port/build/foliage-sway-2026-10-22/{run.log,forest.bmp}`.
- An ID46-only physical-GPU capture with the same script and frame ended in the same gameplay state. Compared with the pre-sway ID46-only physical capture in `port/build/repeat-flip-phase-2026-10-22/only-46.bmp`, changed pixels are bounded to `(0,571,1280,701)` (117,062 pixels). This establishes a localized rendered effect, not exact visual parity. Ignored evidence: `port/build/foliage-sway-2026-10-22/{only-46.log,only-46.bmp}`.
- Complete integration was not rerun for this isolated visual path; there is no human display, controller, speaker, matched-Windows, or release acceptance claim.

`CHECKPOINT_STATUS.md` was already concurrently modified and was deliberately not appended here. `port/FOREST_REFERENCE_AUDIT.md`, `PORT_WORK_STATUS.md`, and `port/COMPOSITION_DIAGNOSTIC_NOTES.md` were left to their owners.

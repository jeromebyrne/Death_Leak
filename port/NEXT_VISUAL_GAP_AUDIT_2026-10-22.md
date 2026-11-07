# Next bounded forest-1 visual-gap audit — particle brightness alpha (2026-10-22)

## Verdict

A **source-demonstrable, still-unported particle-shader semantic** exists for forest-1 oak leaves (XML ID47): source brightness scales *both* sampled RGB and sampled alpha; the Linux port scales RGB only. A one-line, particle-local correction is justified by the shader contract, independent of an unmatched Windows screenshot. Its effect is confined to four leaves, so this is **not evidence of a high-impact whole-screen composition problem** and cannot explain the near-black ID37 foreground or pale ID15 shaft. Prioritize accordingly. No code/build/GPU change was made here.

## Verified source-to-port differential

- Forest XML `Game/XmlFiles/levels/forest_exploration_1.xml:780-799` authors ID47, an `oak_leaf.png` spray at `kGroundFront`, four particles, brightness range 0.85–1.0, and fade-in/out parameters. Port `port/game/LevelScene.cpp:267-323` explicitly loads only this forest emitter, reads the brightness range, and assigns per-particle brightness. This is an active ported visual path, not an unsupported object type.
- Original `Game/src/Particles/ParticleSpray.cpp:237-267` puts fade alpha in vertex `Normal.x` and brightness in `Normal.y`. `Game/ShaderFiles/Particle.fx:52-59` calculates `sample * Normal.y`, multiplying **RGBA** by brightness, then multiplies alpha by `Normal.x`. The intended source fragment is `RGB = sampled.rgb * brightness; A = sampled.a * brightness * fadeAlpha` (before blending).
- Linux `port/game/ParticleSpray.h:81-95` sets the leaf tint to `(brightness, brightness, brightness, fadeAlpha)` and calls the generic sprite renderer. `port/linux/SDLRenderer.cpp:185-188` independently applies RGB color modulation and alpha modulation. Thus its fragment is `RGB = sampled.rgb * brightness; A = sampled.a * fadeAlpha`: brightness is missing from alpha. This is a direct algebraic mismatch even if random leaf positions/timing differ between platforms.
- For a particle at brightness 0.85 with fade alpha 1 and opaque texel, the source alpha is 0.85 while port alpha is 1.0; source-alpha-over therefore composites a visibly stronger leaf in the port, up to 0.15 absolute alpha / 17.6% relative opacity in this authored range. At brightness 1 the paths coincide. The exact frame-average visual size is unmeasured because no matched Windows frame or new GPU capture was taken.

## Bounded correction for the code owner

In `port/game/ParticleSpray.h::Draw`, set only the emitted leaf's alpha modulator to `clamp(p.alpha * p.brightness, 0, 1)` (rounded to byte), retaining RGB brightness modulation. Do **not** change the general `SDLRenderer::DrawSprite` alpha semantics: ordinary forest sprites and authored ID15/ID37 must remain untouched. The fix affects only ID47's particle instances. A follow-up should verify on a source-formula unit/example or isolated same-frame ID47 capture that leaf alpha varies with brightness, then record a short active-gameplay clip; a Windows matched-frame comparison is still needed for acceptance of overall visual parity.

## Evidence limits / non-findings

This audit is read-only source inspection. It does not establish how often ID47 appears onscreen, whether its four leaves dominate any particular captured frame, or exact Windows-vs-Linux pixels. It does not justify modifications to ID37 darkness, ID15 shaft texture/placement, global blend modes, sepia, or particle UV/simulation based on speculation. The correction above is justified as source-semantic fidelity but is **small in scene coverage**, not the requested high-impact global explanation.

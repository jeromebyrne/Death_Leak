# ID47 particle brightness–alpha parity (2026-10-22)

## Bounded result

Corrected `port/game/ParticleSpray.h::Draw` so each leaf's alpha modulator is `round(255 * clamp(fadeAlpha * brightness, 0, 1))`. RGB remains brightness-modulated as before. The change is local to particles, not the generic SDL sprite renderer or other forest layers.

## Source contract and reason

`Game/ShaderFiles/Particle.fx:55-59` first multiplies the sampled RGBA vector by `input.Norm.y` (brightness), then its alpha by `input.Norm.x` (fade). `Game/src/Particles/ParticleSpray.cpp:237-267` supplies those two vertex components. Previously the port's leaf tint alpha contained fade only; `port/linux/SDLRenderer.cpp:185-188` applies the independent RGB and alpha tint channels. The corrected tint follows source `RGB = sample.rgb * brightness`, `A = sample.a * brightness * fade`, to the resolution of the 8-bit SDL modulation. The forest XML ID47 brightness range is 0.85–1.0 for four `oak_leaf.png` particles (`Game/XmlFiles/levels/forest_exploration_1.xml:780-799`).

## Verification

`port/tests/particle_alpha_parity_test.cpp` uses a recording `Renderer` to inspect emitted sprite tint without a GPU. It checks 0.85 brightness with full fade (RGB/A 217), independent 0.5 brightness and fade (RGB 128/A 64), full brightness with half fade (RGB 255/A 128), clamping, and zero fade. Run in the established `deathleak-linux-build:bookworm` image with `g++ -std=c++17 -Wall -Wextra -Wpedantic`; result: **PASS**. The same Docker invocation runs `make -C port -j2` for the Linux game build. `git diff --check` is also required before committing.

## Scope and limits

This is source-semantic parity, not a matched Windows/GPU pixel comparison or overall forest visual acceptance. It can affect only emitted particles and does not address ID37 foreground, ID15 shaft, global blending, or any larger composition difference. No physical-GPU capture, FFmpeg, or shared status file was used.

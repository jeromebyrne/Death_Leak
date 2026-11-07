# ID37 fractional-parallax float alpha single-apply fix — 2026-10-23

## Scope and result

Read-only source/arithmetic gate confirmed Forest-1 ID37 (`shrubbery.png`, `effectlighttexture`) is authored at alpha `0.95`; the Linux loader sets both `tint.a=round(.95*255)=242` and `alpha=.95`, and the draw copies these to `draw.tint` and `draw.opacity`. Its nonzero X UV shift enters `DrawFractionalParallax`; the accelerated `FloatQuad` branch previously sent `242/255 * .95 = 0.901568627` as GL alpha. The ordinary SDL fallback sets texture alpha from `tint.a` alone (`242/255 = 0.949019608`). Windows `Sprite::Draw_effectLightTexture` sends one `m_alpha` and `BasicLightTexture.fx` multiplies sampled alpha by that value once. This confirms the double application in the float branch.

**Only renderer change made** in `port/linux/SDLRenderer.cpp`, in `DrawFractionalParallax`'s `glColor4f`:

```diff
-                      sprite.tint.b / 255.f, sprite.tint.a / 255.f * sprite.opacity);
+                      sprite.tint.b / 255.f, sprite.tint.a / 255.f);
```

The byte tint representation agrees exactly with the existing SDL fallback, and differs from Windows' unquantized `0.95` by `0.000980392` (one byte-rounding step or less). Other GL effects' alpha expressions, dirty raft mip experiment code, XML, UVs, blend/sampler/depth state, and draw order are untouched.

## CPU/source gate

Added standalone `port/tests/id37_float_alpha_cpu_gate.py` (no integration script edits). It checks the live loader/draw/FloatQuad/SDL-fallback and Windows shader source chain, then exercises unity alpha, ID37 `.95`, and waterfall-bottom ID10 `.5`. Command and result:

```text
python3 port/tests/id37_float_alpha_cpu_gate.py
opaque: authored=0.950000, tint=242, float=legacy=0.949019608
lower alpha: authored=0.500000, tint=128, float=legacy=0.501960784
unity: authored=1.000000, tint=255, float=legacy=1.000000000
```

`git diff --check -- port/linux/SDLRenderer.cpp` passes. This is a static/CPU gate, not a renderer build or image comparison.

## Limitations and next gate

No build, GPU/display run, capture, or matched Windows measurement was performed in this lane. The existing physical-GPU log identifies a 2048×512 float parallax texture but not the object filename; source mapping plus size supports ID37 without proving per-ID runtime tracing. The byte-rounding choice preserves Linux fallback consistency; exact Windows alpha and mip/light-color/composition parity remain open. When the GPU worker releases the current binary/display, coordinator should build the merged tree and run same-frame baseline versus hide-ID37 capture, checking predicted darker opaque lower band while the irregular onset stays fixed; a lossless matched original-Windows frame is still required for exact parity.

No commit or push; coordinator owns integration with concurrent capture gate.

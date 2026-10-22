# Forest fog float-alpha correction (2026-10-22)

## Change

Preserved `SceneSprite::alpha` as `render::Sprite::opacity` at draw submission, and uploaded that float to the per-sprite PixelWobble `spriteAlpha` uniform. The existing 8-bit `tint.a` remains for SDL copy/geometry paths. No UV, motion, shader sampling, or global postprocess changes.

The renderer-neutral `port/render/Renderer.h` required a one-field addition because `render::Sprite` is the value passed across the scene/backend boundary; there was no existing float opacity slot. This is the only necessary file outside the initially listed implementation pair.

## Verification

- `port/tests/scrolling_sprite_test.cpp` now asserts both authored fog opacities (.5, .6) and fade-at-one-second opacities (.5/6, .6/6) survive as floats, while byte tint remains unchanged. It explicitly checks float vs byte discrepancy.
- Docker `deathleak-linux-build:bookworm`: focused scrolling-sprite test **PASS**; `make -C port -j2` **PASS**. Existing TinyXML compiler warnings only.
- No physical GPU/Xorg run or capture was performed, to avoid overlap with other workers.

## Remaining visual gate

A deterministic fog-only fade-frame GPU comparison is still needed to measure whether the corrected input changes actual composited pixels and to compare to Windows if a matched reference is available. This code/test/build verification establishes data-path parity, not screenshot parity.

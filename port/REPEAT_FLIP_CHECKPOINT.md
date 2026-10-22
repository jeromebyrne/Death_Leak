# Repeated + horizontally flipped Sprite UV phase — 2026-10-22

Scope: SDL repeat-X tessellation for ordinary `Sprite` objects; no asset, level XML, foliage-sway, physical GPU/Xorg, or visual-acceptance changes.

The original Sprite assigns UV X from 0 to `objectWidth / textureWidth` and then flips the whole quad. SDL's old loop instead flipped each left-anchored texture tile. For forest-1 ID46 (`leaves_shadow.png`, 10000-world-unit quad, 2048-pixel texture), the source's left-edge phase is 1808/2048, whereas the old port began with a full local tile. `ForEachRepeatXTile` now anchors flipped copies at the quad's right edge, leaving the 1808-source-pixel partial repeat at its left; non-flipped copies remain left-anchored. Culling still uses the full authored quad, not the visible viewport, as the phase origin.

Verification: Docker `deathleak-linux-build:bookworm` compiled and ran `port/tests/repeat_x_test.cpp` (PASS: flipped whole-quad repeat phase and viewport culling), then `make -C port -j2` passed. Compiler warnings were limited to existing TinyXML warnings. This is deterministic source/code-path verification only; foliage sway and matched Windows/physical-display visual parity remain unverified.

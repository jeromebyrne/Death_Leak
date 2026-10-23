# Fog PixelWobble mip-linear CPU implementation — 2026-10-22

## Source contract and implementation

`Game/ShaderFiles/BasicLightTexture_PixelWobble.fx` declares `MIN_LINEAR_MAG_POINT_MIP_LINEAR`, U/V wrap, ordered V-then-U wobble, and sampled-alpha multiplication by the sprite's float alpha. The port had an SDL-owned texture bound in `DrawPixelWobble` with `GL_LINEAR` minification and no mip chain. `Game/src/Core/TextureManager.cpp` loads via default `D3DX10CreateShaderResourceViewFromFile` settings; exact D3D mip texels, gamma treatment and LOD rounding remain unknown.

`port/linux/SDLRenderer.cpp` now lazily decodes a file-backed texture on its first PixelWobble draw, reverses PNG rows for the shader's existing `1-V` orientation, uploads a **private** full-size GL RGBA8 texture, generates/checks its complete mip chain, and selects `GL_LINEAR_MIPMAP_LINEAR` minification, `GL_NEAREST` magnification and repeat wrapping. It caches one private name or one failure per `TextureId`; `LoadTexture` already deduplicates identical paths to one ID. The private names are destroyed before SDL renderer/window teardown. No ordinary sprite/HUD/damage/foliage sampler, shader wobble math, source UV, rotation or opacity was changed.

The path requires desktop GL 2.0 NPOT support plus a current-context core or EXT mip-generation entry point. Capability, PNG decode, allocation, generation, level-size or sampler error causes a once-per-ID factual log and the prior SDL-bound `GL_LINEAR` wobble draw. Failure is cached to avoid a per-frame decode or log. The SDL fallback preserves its previous wrap/min/mag values. Both paths flush queued SDL draws first, capture/restore unit-0 binding and prior active unit, program, matrix mode, matrix stacks and pushed attributes; upload also restores unpack alignment. SDL unbind runs only after successful SDL bind. A later renderer/context recreation would need to invalidate this cache; current class creates one renderer/context.

## CPU gate performed

- `docker run --rm -v "$PWD":/src -w /src deathleak-linux-build:bookworm make -C port fog-mip-cpu-test` — PASS. The no-window/no-context test exercises production row reversal, exact 2x2 and NPOT 3x5 CPU box mip expectations with odd-edge handling and alpha, wrapped bilinear, both flips, nearest magnification, and trilinear positive-LOD reference. The audited 1988x1290 to 1325x860 ratio yields lambda approximately 0.585 (levels 0/1 blend), not base-only bilinear. A pure fake state/cache ledger covers reuse, replacement/context invalidation policy, failed generation, successful SDL fallback and failed SDL bind; it is a contract simulation, not a GL interoperability test.
- `docker run --rm -v "$PWD":/src -w /src deathleak-linux-build:bookworm make -C port -B all` — PASS. Only pre-existing tinyxml compiler warnings appeared.
- `git diff --check` for scoped implementation files — PASS.

No Xorg or physical GPU was used. Actual extension resolution, completeness on a live driver, SDL-after-raw-GL state, fog A/B, curtain magnification and matched Windows visual parity **remain unverified**. This is source/CPU/build evidence only, not visual acceptance. The CPU ratio oracle applies to an unrotated, unwarped full-UV quad; the live wobble/rotation derivatives are not byte-exactly predicted by it.

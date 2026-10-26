# Atlas single-draw UV/filter discriminator — 2026-10-26

Base: clean detached remote `e32738d599290fb9a95c3df2b383e8a252ba231a`; sole source/build/physical-NVE7 owner. **Result: reject cart atlas default and any UV-bias patch.** This report is the only accepted repository change. Primary seven unrelated tracked edits and untracked artifacts were preserved.

## Matched physical GL draw

The prior serial physical NVE7 `apitrace` pair for the same Forest-1 B:180 route contains a directly matched cart-track draw at call 312370, immediately after `glBindTexture(GL_TEXTURE_2D,267)`. `apitrace dump --blobs --calls=312340-312374` exposed the actual `glVertexPointer` and `glTexCoordPointer` arrays; all six GL_FLOAT screen-position vertices are identical between standalone and atlas. `glTexParameteri` sets both MIN and MAG to `GL_LINEAR` in both arms. The source is the complete 492×61 cart-track rectangle, with Forest IDs 50 (horizontal flip) and 51 (near-zero rotation, no flip), alpha 0.6, and 2/3 destination minification. The atlas track rect is `(1034,2,492,61)` within the verified full-RGBA 1528×516 page, with two-pixel extrusion.

| Vertex group | Standalone normalized UV | Atlas normalized UV | GL position example |
|---|---|---|---|
| top left/right | (0,0), (1,0) | (0.676701545715,0.003875968978), (0.998691082001,0.003875968978) | (892.332336,638.998779), (1220.332397,639.001343) |
| bottom left/right | (0,1), (1,1) | (0.676701545715,0.122093021870), (0.998691082001,0.122093021870) | (892.332703,598.332031), (1220.332642,598.334717) |

The other flipped draw at call 312356 likewise has identical screen vertices and reversed horizontal endpoint assignment. At the two failed **pixel centers**, barycentric interpolation of call 312370's captured triangles followed by `texel = UV * texture_dimension - 0.5` (subtracting atlas origin for source-local coordinates) yields:

| Screen pixel | Standalone local texel (x,y) | Atlas local texel (x,y) | Standalone horizontal bilinear fraction | Atlas fraction |
|---|---|---|---|---|
| (1172,609) | (419.7511248,43.7514710) | (419.7510964,43.7514705) | 0.7511248 | 0.7510964 |
| (1180,610) | (431.7511360,42.2515677) | (431.7511079,42.2515672) | 0.7511360 | 0.7511079 |

For (1172,609), bilinear (top-left, top-right, bottom-left, bottom-right) weights are standalone `(0.06185271,0.18667631,0.18702249,0.56444849)` versus atlas `(0.06185989,0.18666965,0.18704369,0.56442678)`. For (1180,610), they are standalone `(0.18625782,0.56217444,0.06260614,0.18896159)` versus atlas `(0.18627897,0.56215380,0.06261309,0.18895414)`. These are **CPU calculations from captured GL vertices**, not a claim to observe the GPU's internal interpolator precision. They establish a real page-normalization difference at exactly the two failing samples while leaving the last GPU rounding/blend stage unisolated.

The atlas's captured left/right GL_FLOAT UVs map to source endpoint errors `-0.000038147` and `-0.000026703` texels. Their next representable higher float values would map to `+0.000052929` and `+0.000064373`; no one-ULP endpoint step reproduces both exact standalone endpoints. The prior low-X-origin layout left the physical candidate BMP identical. An arbitrary endpoint bias, nearest filtering, changed source bytes, or per-pixel patch is therefore **not a justified general correction**: it could move many samples and regress transformed/source-rect cases. The measured normalized-float precision shift is a sufficient plausible mechanism for the one-LSB result, but precise driver sampler/rounding attribution remains open. No speculative renderer variant was attempted.

## Fresh isolated build and serial physical gate

Regenerated the three cart/prop assets with `atlas_pack.py --full-rgba` into one 1528×516 page; source SHA, reconstructed full RGBA including invisible RGB, and two-pixel extrusion passed. An isolated Docker production build passed. A native-clock candidate capture showed unrelated broad film-clock variance and was **not** used for pixel attribution. With the **same temporary frame-indexed film clock in all arms**, physical X11/OpenGL Mesa nouveau NVE7, accelerated=1/software=0/vsync=1, 1280×720, fixed 1/60 dt, Forest-1 B:180 A(`DEATHLEAK_ATLAS_CART_PROPS=off`)/B(`on`)/A(`off`) was strict serial:

- Full BMP payload (excluding 54-byte header) controls: SHA-256 `06145c2fccbb75152d47aa5132cb6cbe6a09ee9d00d68786c568c3cec90794d6`, byte-equal.
- Candidate: `a4026a01368976432a33d21b6f760dcd0575c9f662f1b4c8b3322b234dbe2814`, differing in exactly two RGB bytes: (1172,609) green 31→30; (1180,610) red 35→34. No other BMP payload difference.
- All arms had 15,990 measured SDL sprite submissions and identical gameplay endpoint (player 1368.99,3107.3; five orbs; 25 health). A separate temporary transparent standalone cart-track control changed 5,651 RGB pixels in x416..1209, y607..633, proving the visible branch. The original PNG was restored, SHA-256 `9260867645ad75cf9513257d6729f7f8ebb005ceffc5adc39d1efe9b7a085ae1`.

The prior same-route **actual GL** traces (used above, no inferred batching) counted steady frames 121–180: `glDrawArrays` 6,235 versus 6,235; private `glBegin` 360 versus 360; `glBindTexture` 8,970 versus 8,970, standalone/candidate. This slice does not claim a new trace or timing gain. Painter, blend, shader, source rectangle, and filter state were not changed. Source/asset/film fixtures were restored before this report was committed. Default atlas coverage remains 15/896; menu, branches and cart remain rejected for exact-pixel default coverage.

## Next concrete discriminator

A general sampling correction requires a bounded alternative with a mathematical source-local sampling contract (for example, a programmable sampler that reconstructs source texel positions at sufficient precision while preserving SDL's GL_LINEAR and edge behavior), then multiple transformed/source-rect pixel cases, strict serial physical full-BMP A/B/A and **new** actual GL bind/draw measurements. Do not enable this cart page by default on the present evidence. No FPS, 8K/high-Hz, VRAM or release acceptance is claimed.

Ignored evidence: `port/build/atlas-uvfilter-20261026/` in the primary workspace (GL pointer blobs and calls, isolated build logs, all BMP/log arms, restored-source checksum material); preceding `port/build/atlas-third-family-20261026/` holds the full GL traces and counted call dump.

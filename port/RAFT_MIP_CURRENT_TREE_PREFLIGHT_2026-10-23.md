# Raft mip current-tree preflight — 2026-10-23

## Verdict

**Keep the ID11/57/58 raft mip question as a narrow, test-worthy candidate; no visible defect or Windows parity improvement is proven.** At current tree commit `63855f2`, the XML and PNG are tracked and unmodified; `port/game/LevelScene.cpp` has another worker's uncommitted changes, so the draw-path findings below describe the inspected **working tree**, not a clean-HEAD build. No build or runtime was used.

## Identity and projected footprint

- `Game/XmlFiles/levels/forest_exploration_1.xml` IDs **11, 57, 58** are `Sprite` elements, each with `Media/objects/raft.png`, `isanimated=false`, `effectlighttexture`, `alpha=1`, dimensions **458.298584 × 125.316017**, `native=false`, and `repeatX=false`/`repeatY=false` (lines 191–205, 928–957). ID11 has rotation 0; IDs57/58 have rotation 0.219974; authored horizontal flips differ. Those transforms do not change the shared asset identity or nominal axis scale.
- The single referenced file `Game/Media/objects/raft.png` is **1024 × 280, 8-bit RGBA PNG**, SHA-256 `d319f5d1cc292565df20111a67529feb8b4ff3434ff2176fa3d7ddf5f420d031`. This is one file/hash shared by all three IDs, not three inferred copies.
- Current `LevelScene::Draw` sets `scale = 2.f / 3.f` (`port/game/LevelScene.cpp:1188`), producing an unrotated destination **305.5324 × 83.5440** logical output pixels (`:1254–1263`), or **3.35153 source texels per output pixel on each axis**. Rotation and fractional placement can change actual sample footprints locally; this is the nominal full-quad ratio, not a measured GPU LOD.

## Current render branch

The XML loader obtains each PNG via `renderer.LoadTexture`, records the authored effect, and enables private foliage/pixel paths only for `effectfoliagesway`/`effectpixelwobble` (`port/game/LevelScene.cpp:385–392, 525–545`). For these three raft entries, `repeatX` stays false (`:552–555`); `LevelScene::Draw` submits an ordinary `render::Sprite` through `renderer.DrawSprite` (`:1254–1307`). `SDLRenderer::LoadTexture` decodes one RGBA PNG and creates one `SDL_Texture` (`port/linux/SDLRenderer.cpp:153–188`). `DrawSprite` selects `SDL_ScaleModeLinear` during minification, then reaches `SDL_RenderCopyF` or `SDL_RenderCopyExF` for the raft (`:197–225, 263–267`); no ordinary mip chain or LOD selection is present in this inspected branch. The existing private special-effect GL paths do not establish raft mips.

The original *source* requests `MIN_LINEAR_MAG_POINT_MIP_LINEAR` for `BasicLightTexture.fx` (`Game/ShaderFiles/BasicLightTexture.fx:10–16, 53–59`), and its texture-load call passes null D3DX10 load info (`Game/src/Core/TextureManager.cpp:50–79`). Prior `RAFT_MIP_SOURCE_GATE_2026-10-23.md` explains why that API default predicts mips. The actual shipped Windows raft SRV/resource descriptor, generated mip contents and frame LOD are **unmeasured**; this is a source-contract comparison only.

## CPU-only sensitivity probe

To bound whether the nominal 3.35:1 reduction is worth an isolated test, I decoded the unchanged PNG with Pillow and compared (1) a single-sample affine bilinear reduction at 306 × 84 to (2) a BOX area reduction at the same size. BOX is only an **approximate multiscale/area proxy**, not D3DX mip generation, trilinear sampling, SDL output, or a Windows pixel reference. The two proxies differ by >8/255 in at least one channel at **7,308/25,704 (28.4%)** RGBA output pixels and by >24 at **2,603 (10.1%)**. After alpha compositing over black, the corresponding counts are **6,186 (24.1%)** and **1,886 (7.3%)**, with mean absolute RGB channel differences **5.74, 4.47, 2.60**. The source has nonzero alpha at **194,419/286,720** texels. This suggests that sampling treatment can materially alter raft colors/edges at this footprint, but magnitude and sign in an actual renderer remain unknown; the proxy's coordinate alignment, filtering, alpha treatment and compositing are not authoritative.

## Decision boundary

Proceed only to the previously specified Windows bound-SRV descriptor and matched, lossless raft-local crop gate when that separate capability exists (`POST_ID15_VISUAL_DISCRIMINATOR_2026-10-23.md`). A bound view exposing only mip 0 would close this candidate for that runtime. Multiple viewable levels would keep it live, not establish a defect. A later Linux ID11/57/58-only off/on crop can establish local impact; only a registered same-state Windows comparison can support parity. No source/test/shared-status edit, build, game/GPU run, or Windows parity claim was made here.

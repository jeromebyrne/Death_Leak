# Ordered opaque coverage / fragment-invocation gate — no safe geometry B arm

Clean isolated detached worktree `/tmp/deathleak-opaque-20261026` from pushed `2444b77a1dd6ed631312157fd802b6d3236e4b7c`. Primary unrelated dirty work was preserved (append-only performance checkpoint only). Exact-ref Docker production build passed, SHA-256 `2b61ba8cf7875e7aa51f1796e6c6bb957ffbe6a6ada4f3cce9c09547bb9c4cd2`. The diagnostic pipeline-query build was test-only and reverted before final production verification. Local raw logs/BMPs: `/tmp/deathleak-opaque-evidence/`; alpha inventory scratch `/tmp/deathleak-opaque-inventory.py`. One offscreen/Intel and one X11/Intel selector probe were discarded, not counted as NVE7 evidence.

## Exact source and painter conditions

`LevelScene::LoadLevel` stable-sorts sprites descending by depth; `Draw` preserves authored equal-depth order and interleaves dynamic/spray draws. Forest ID1 far background precedes ID21 middle background and ID22 near background. Source assets were opened as RGBA; 45 Forest and 53 Waterfall authored Sprite/ParallaxLayer rows with resolvable PNGs were counted. **Zero** rows in either scene combine globally alpha255 source with authored alpha=1. The few globally alpha255 sources have reduced authored alpha. This rules out whole-quad opaque coverage from those rows, not every possible partial opaque texel region or dynamic draw.

| Source/layer | Alpha255 texels | Other source alpha | Authored alpha | Consequence |
| --- | ---: | ---: | ---: | --- |
| Forest ID1 `tree_strip_bg.png`, 1985×1090 | 2,163,650 (100%) | 0 | 0.9 | Expensive predecessor, **not** an opaque successor; hiding it changes pixels. |
| Forest ID21 `tree_strip_dark.png`, 1985×1238 | 628,832 (25.59%) | 1,828,598 | 1.0 | Repeat-X, partial opaque source only. Largest all-alpha255 axis rectangle 99×808 = 3.26% source area. |
| Forest ID22 `tree_strip.png`, 1985×1238 | 1,001,208 (40.74%) | 1,456,222 | 1.0 | Repeat-X, partial opaque source only. Largest all-alpha255 axis rectangle 151×1161 = 7.13%. |
| Waterfall ID13/14 `waterfall_4.png`, 512×1024 each | 0 | 524,288 | 0.3 | Private wobble, no opaque texel. |
| Waterfall ID58 `blank_white.png`, 2×2 | 4 (100%) | 0 | 0.2 | Authored blend, not opaque cover. |

The SDL ordinary path selects nearest except for minified scene copies, where it selects linear; `DrawSprite` also supports source rectangles, trimmed atlas regions, tint alpha, flips/rotation, repeated X, UV shifts, world projection/zoom and private wobble/sway paths. For nearest, an opaque mask requires the actual source texel alpha255 and effective tint alpha255 at every covered sample. For linear, every contributing tap must be opaque; source-alpha rectangles above are only an **upper bound** before filter-footprint erosion. Repeat/wrap edges, atlas extrusion and source-rect boundaries need their actual UV mapping. A later alpha1 texture's bounding rectangle is not an occluder where its source alpha is zero/mixed. An earlier draw may be suppressed only for fragments that all *later ordered* draws replace with exactly opaque output; depth alone, same texture, or a later full-screen bounding box is insufficient. Private quads and Sepia do not rescue an invalid ordinary-painter skip.

## Physical accelerated fragment discriminator and camera states

The isolated diagnostic compiled `GL_FRAGMENT_SHADER_INVOCATIONS_ARB` around the scene-target draw interval after clear and before Sepia; an SDL flush closed the queued ordinary draws before reading. It counted all scene fragment shader invocations, including ordinary, private and overlapping draws; it did **not** count Sepia/HUD and does not label individual painter IDs or prove any invocation removable. Four serial 100-update fixed-dt baseline runs were authenticated X11 `:0`, OpenGL nouveau **NVE7**, accelerated=1/software=0/vsync=1, 1280×720. The static `.:100` and rightward `R:100` routes changed physical camera/player states: Forest player x −43.015→728.985; Waterfall 818.985→1590.985. Full baseline BMPs at all four endpoints are retained. Query-on runs share gameplay endpoints with query-off captures; their wall-clock film phase may differ, so the BMPs are not an A/B parity test.

| Route, 100th scene frame | Fragment invocations | / 921,600 output pixels | Frames 30–100 median |
| --- | ---: | ---: | ---: |
| Forest static | 8,665,541 | 9.40× | 8,668,843 |
| Forest right | 8,338,821 | 9.05× | 8,471,028 |
| Waterfall static | 10,215,134 | 11.08× | 10,139,453 |
| Waterfall right | 8,064,698 | 8.75× | 9,129,041 |

These high counts establish material overdraw/fragment workload and camera sensitivity, **not** opaque overlap or a candidate saving. Prior physical ID1 scissor/visible-region diagnostics measured ~1.5–1.6 ms cost for its full visible layer, but hiding it changed authored pixels. Prior per-painter query-on Waterfall sheet/fog timings are also visible blended work, not a removable budget. The diagnostic query alters synchronization and is not query-off frame pacing. There was no candidate B, so no new serial A/B/A, no pixel or timing gain, no call-count reduction and no VRAM/thermal claim. Default atlas mapping remains 15/896 (881 not default-mapped).

## Disposition and next distinct gate

**Reject whole-quad / bounding-box occlusion; ship no runtime geometry change.** The only source-proven Forest successor coverage is partial alpha255 regions. Its largest rectangular source islands are small before scaling/filter/wrap, and no screen-space, source-alpha/sampler-aware *ordered* coverage mask was proven at both camera states or across dynamic painter content. Waterfall's large measured sheet/fog work is intentionally translucent. An invented B arm would risk false skips and fail the exact-pixel contract. Reverted the diagnostic source and rebuilt exact production code.

Next gate, if pursued: capture per-painter projected vertices, source rect/UV and effective tint/blend at multiple camera states; construct a conservative raster/sampler oracle for later alpha255 texels (linear four-tap footprint, repeats/atlas edges, rotations and source rectangles) and intersect *later ordered* opaque coverage with an identified expensive predecessor's actual fragments. Cross-check candidate savings with per-ID NVE7 pipeline-statistics queries or controlled replay, then prototype a tiny geometry clip only where that intersection is proven. Require synchronized-film full post-HUD BMP A/B/A at multiple cameras plus query-off physical p50/p95/p99, steady calls, gameplay, and memory/thermal gates. No 8K/high-Hz/60-FPS or release acceptance is established.

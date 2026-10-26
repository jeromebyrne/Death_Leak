# Death overlay gameplay-HUD atlas gate — 2026-10-26

## Source and eligibility

Exact pushed base `434d8a1352b0cf80998f423ab83d1592003c2c7f`; isolated clean detached `/tmp/deathleak-death-overlay-20261026`. Primary unrelated dirty source was not used for builds or changed except append-only performance checkpoints. `port/linux/main.cpp` loads `Game/Media/UI/you_died.png` in gameplay and draws it on `combat.Health() <= 0`, after scene/HUD, at full authored viewport `{0,0,1280,720}`. Original PNG is 1920×1080 RGBA, alpha 0..223 (87,027 nonzero partially transparent source pixels). This ordinary draw has no crop, repeat, UV shift, private shader, rotation or tint override. `SDLRenderer::DrawSprite` selects nearest for post-scene HUD even though the 1920×1080 source is minified to 1280×720. The existing virtual region preserves source dimensions, painter order, source-over alpha and dedicated fallback for shader/wrap paths.

Deterministic `atlas_pack.py --full-rgba` repacked the twelve prior HUD assets with `you_died.png`, all thirteen full source rectangles and nearest-extruded two-pixel edges byte-verified. One trimmed **2504×1312** page (<4096 each dimension), with death art at `(2,2,1920,1080)` and **86.67% source-rectangle occupancy**. No second page. Default-on `DEATHLEAK_ATLAS_HUD=on` maps the actual draw through its manifest/TSV virtual region; `off` retains dedicated source. Coverage rises **21→22/896**. Isolated Docker production `make -C port -j2` passed; binary SHA-256 `57825a51da3692c4bdd7cba52f0411891660ac6ab88b46fb90976ffcbf78378f` (unchanged executable because this slice changes runtime assets only).

## Physical NVE7 A/B/A

A deterministic DL2 zero-health Forest saved fixture makes the game log `Player died at frame 0` and repeatedly draws the death overlay. Serial physical Xorg `:0`, SDL x11/OpenGL accelerated=1 software=0 vsync=1, nouveau/NVE7, 1280×720, fixed 1/60-dt A(off)/B(on)/A(off), three presented frames: all captured full RGB frames byte-identical, **0/921,600 changed pixels**, SHA-256 `36bffcae28f267a630987cb300333a47d70f2ef3518e31ee4c8fc5ec46e53416`. Separate serial traced A/B/A captures also byte-identical. Authored partial-alpha death art is composited over the visible scene; this is an actual death route, not a no-op loading route.

Three-present apitrace: total GL `glDrawArrays` **206/206/206**; total `glBindTexture` **1107/1054/1107**. Each warm present (frames 1–2) has draws **69/69/69**, binds **107/97/107**, texture uploads **0/0/0**. Bind reduction includes the existing HUD atlas advantage and added death overlay sharing; no draw-submission reduction is claimed.

Serial untraced 360-frame zero-health Forest profiles (120 warmup, 240 measured) gave frame-time p50 **22.7180/22.6978/22.7508 ms**, p95 **25.9966/25.9821/26.0035 ms**, p99 **26.0635/26.0175/30.7511 ms**. Candidate p50/p95 are within control spread; no FPS or tail improvement is claimed. This is a stationary death route, not active gameplay acceptance.

Uncompressed RGBA page proxy is **13,140,992 bytes** versus prior HUD page **3,364,632** plus dedicated death texture **8,294,400**, net **+1,481,960 bytes**. This is a real packing-memory cost, not a saving; direct driver VRAM allocation/residency and thermals were not measured. PNG disk page grows 684,915→899,597 bytes while the dedicated source remains on disk for off/fallback.

## Disposition

Accept the bounded, default-on point-sampled death-overlay atlas route: exact physical death-frame pixels, preserved alpha/order, fewer binds, unchanged draws, no timing gain and higher raw texture-area proxy. Do not infer 8K/high-Hz/60-FPS, sustained gameplay, thermal or release acceptance. Next distinct in-scope gate: an actually drawn **post-scene point-sampled modal/pause art family** with page-fit and recurring-call/memory proof; audit actual draw/filter before repacking, avoiding inactive XML references and linear-minified world sprites.

Raw physical BMPs/logs, GL traces and profile logs: `/srv/openclaw/workspace/death_leak_analysis/death-overlay-evidence/`.

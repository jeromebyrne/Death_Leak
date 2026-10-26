# Four-frame gameplay film atlas gate — 2026-10-26

## Scope and source contract

Exact pushed recovery base `dc63ba88b034256e8a803c1ece3800e6dc620acc`; isolated clean detached `/tmp/deathleak-next-point-atlas-20261026`. Primary dirty source was not used for builds or edits. The next recurrent ordinary family is `Game/Media/UI/film_frame.png`, `_2`, `_3`, `_4`: four 576×324 frames loaded by `FilmFrameResidency::Prepare()` and selected by the 11-FPS gameplay film clock. `main.cpp` draws one as a full-viewport post-scene HUD sprite at alpha 51; the SDL renderer's post-scene copy uses point/nearest sampling. No source crop, repeat, UV shift, private shader, or alternate painter slot is introduced. This is distinct from the rejected linear-minified orb family.

The existing gameplay-HUD virtual atlas lookup is default-on and remains controlled by `DEATHLEAK_ATLAS_HUD=off`. Added only four manifest/page/TSV entries; the renderer already routes exact paths through virtual regions and preserves tint/alpha/order. Deterministic `atlas_pack.py --full-rgba` packed all prior eight HUD entries and four film frames into **one 2526×333 trimmed page**, below 4096 in both dimensions. Packer verification byte-compared all twelve full source RGBA rectangles and nearest-extruded two-pixel edges. Source-rectangle occupancy is 91.97%; no second sparse page. Default coverage rises **17→21/896**. Isolated Docker production `make -C port -j2` passed; binary SHA-256 `57825a51da3692c4bdd7cba52f0411891660ac6ab88b46fb90976ffcbf78378f` (same binary in all arms).

## Serial physical NVE7 evidence

Authenticated Xorg `:0`, SDL x11/OpenGL accelerated=1 software=0 vsync=1, nouveau/NVE7 at physical 1280×720. Docker was given the live LightDM Xauthority mount without exposing cookie bytes. Runs were strictly serial, with no other Death Leak GPU job. Fixed-dt 1/60 active Forest `B:N` and test-only 100-ms film clock forced four successive film phases at N=121,122,123,124. For **each** N, A(off)/B(on)/A(off) full RGB BMPs were byte-identical: 0/921,600 changed pixels. Distinct phase SHA-256 prefixes are `293693360579`, `cdf98344913e`, `4a317e77bb4a`, `745adb06ea82`. This closes the four source-frame visible pixel gate for this route, not every game scene or time phase.

Serial 124-present physical apitrace on the same active route also produced byte-exact A/B/A frame-124 captures. Total `glDrawArrays` **11,464/11,464/11,464**; `glBindTexture` **18,361/17,347/18,361**; `glTexImage2D`+`glTexSubImage2D` **786/764/786**. Presented frame 124: draws **100/100/100**, binds **152/144/152**, uploads **0/0/0**. The 8-bind/frame delta includes the pre-existing seven-bind HUD atlas advantage plus the one recurrent film source sharing the page; the atlas does not reduce draw submissions. Trace overhead is not FPS evidence.

Serial **untraced** 360-frame Forest profiles (120 warmup, 240 measured) gave p50 **19.8005/19.7675/19.8190 ms**, p95 **24.3923/24.7481/24.5471 ms**, p99 **25.2982/25.3162/25.2699 ms**. Candidate p50 lies inside control variation; candidate p95 is worse than both controls. No frame-rate or tail-latency improvement is claimed. The last-frame capture outlier was 49.63/45.91/50.70 ms, not a repeatable optimization signal.

Raw uncompressed page proxy rises from 594,032 to **3,364,632 bytes**, while four dedicated 576×324 RGBA textures (2,985,984 bytes) are avoided: **215,384 bytes less** than old page plus dedicated film rectangles. Actual driver VRAM allocation/residency was not measured; no VRAM saving claim. PNG disk size rises from 19,270 to 684,915 bytes.

## Disposition and next distinct gate

Accept this bounded, default-on, point-sampled recurrent four-film virtual-atlas family: exact four-phase physical pixels, unchanged draw count, fewer binds, neutral/worse-within-noise timing, and modest favorable raw page-area proxy. Do not infer 60 FPS, 8K/high-Hz, all-scene, sustained, thermal, or release acceptance. Next distinct source gate: audit recurring loading `player_silhouette_2.png` draw/crop/filter/tint in transition frames and page fit; only route it after exact partial-alpha A/B/A and direct call/memory evidence. Do not revisit orb linear-filter routing under this point-sampled gate.

Raw evidence: `/srv/openclaw/workspace/death_leak_analysis/film-atlas-evidence/` (four-phase BMP/log A/B/A, 124-frame GL traces and captures, 360-frame profile logs).

# Transition loading silhouette atlas source gate — 2026-10-26

## Provenance and candidate

Exact pushed feature base `5fd4d1228f0bab97ac6ff231ed1768c70870b4c2`; clean detached isolated checkout `/tmp/deathleak-silhouette-20261026`. The primary dirty source was not used for build or edited except append-only performance checkpoints. No competing local Death Leak game/build/apitrace process was present at preflight.

The prior film atlas report proposed `Game/Media/UI/player_silhouette_2.png` as a recurrent transition/loading sprite. Source audit disproves that premise: the only reference in active game data, `Game/XmlFiles/UI/gameloading.xml`, is within an XML comment (`2_icon`). The sole active `1_load_splash` sprite uses `Media\\UI\\blank_dark.png`. Linux `port/linux/main.cpp` explicitly loads `Game/Media/UI/blank_dark.png` as `modalBackground` and submits it through `LoadingOverlayPacket::Draw` after scene/HUD with fade tint; no active Linux load/draw of `player_silhouette_2.png` exists. Repository-wide textual search finds only the commented Game XML, a duplicate commented Windows-content XML, and the prior report (plus opaque VS database). The transition sprite actually drawn, `blank_dark.png`, was already accepted into the default gameplay-HUD atlas in the preceding slice.

## Page-fit and source-pixel evidence

The inactive silhouette asset is 256×256 RGBA with variable color and alpha. A read-only-to-source, full-RGBA deterministic packer dry run added it to the current twelve-source gameplay-HUD manifest and verified all thirteen source rectangles and 2-pixel nearest-extruded edges. The single trimmed page would be **2786×333**, versus current **2526×333**, under the 4096 ceiling. The silhouette source occupies `(2322,2,256,256)`. Thus page fit/source reconstruction alone are feasible, but are not a draw/filter/alpha/pixel-parity proof for an actual active transition route. Dry-run artifacts are under `/tmp/deathleak-silhouette-pagefit-20261026`, not tracked runtime assets.

## Disposition

**Reject runtime atlas routing of `player_silhouette_2.png` in this slice.** It is inactive, so a physical loading A/B/A would submit the same blank-dark sprite in both arms and could not establish candidate pixel parity, bind reduction, timing benefit, or even that the silhouette path was exercised. Adding this unused source would enlarge the resident HUD page from 2526×333 to 2786×333: **+346,320 raw RGBA bytes**, without avoiding an active dedicated texture or bind. No source or runtime asset change, production build, or physical NVE7 A/B/A was warranted after the source eligibility rejection. Default atlas coverage remains **21/896**. No performance, 8K/high-Hz/60-FPS/sustained/thermal/release acceptance claim.

Next distinct gate: inventory an **actually loaded, recurrent, point-sampled** transition/HUD texture not already in the default page, then prove draw path, filtering/alpha/page residency and physical A/B/A before routing. Do not reactivate the commented silhouette in authored XML solely to improve atlas counts.

# Next active post-scene point-sampled atlas source gate — 2026-10-26

## Provenance and audit

Exact remote `feature/linux-renderer-foundation` base: `19f28f4b25c34cbe17cdac868960b3bf61595967`. Clean detached isolated checkout `/tmp/deathleak-next-active-point-20261026`; primary dirty checkout never built or changed. Prior modal owner had explicitly released source/build/NVE7. No other Death Leak game/build/apitrace process at preflight. This is a source eligibility rejection, not a GPU or FPS experiment.

Inspected Linux `main.cpp` load sites and both draw branches, `Hud.cpp`, `FilmFrameResidency.h`, gamepad icon loads, gameplay-HUD manifest, and `SDLRenderer.cpp` sampling and atlas lookup. After `ApplyScenePostprocess()` flips `sceneSamplingPhase_` false (`SDLRenderer.cpp:1033-34`), gameplay submits four film frames, `frame_top`, six HUD meter/orb assets, `blank_dark`, `orb`, `you_died`, and modal/intro gamepad icons. All are already covered by the 14-source gameplay-HUD page or the existing gamepad-icon atlas; the full source list is in `port/assets/gameplay_hud/atlas_manifest.json`. The HUD page is 2504×1343 and default atlas coverage is 23/896. Bitmap text and HUD bands are not unatlased PNGs.

| Residual source | Actual draw/phase | Decision |
| --- | --- | --- |
| `Game/Media/objects/health_upgrade.png` | An actual but episodic 50×50 pickup sprite inserted at painter depth 3000 before scene postprocess (`main.cpp:1809-16`); destination ~33×47, so `SDLRenderer.cpp:485-490` selects **linear** scene-phase minification. | Outside this nearest/point-sampled post-scene gate; one sprite during a conditional pickup is not a recurrent bind/draw family. Do not silently mix its linear filter with HUD page's nearest path. |
| `Game/Media/UI/player_silhouette.png` | Drawn only when no level and no menu loop (`main.cpp:618,1777`), not in active gameplay. `player_silhouette_2.png` loading XML is inactive. | No gameplay bind saving; prior loading rejection remains valid. |
| `Game/Media/UI/pause_backing_2.png` | XML-only; Linux modal draws already-atlased `blank_dark.png` instead. | Prior inactive rejection remains valid. |
| `Game/Media/UI/orb.png` | Active repeated upgrade-modal draw. | Already mapped by preceding 23rd-source commit; do not revisit. |
| Menu backgrounds/title/frame/buttons/music/sfx art | Recurs in separate `levelPath.empty()` menu branch (`main.cpp:1738-77`), not the active gameplay post-scene branch. Some film assets overlap existing HUD page. | Valid **next distinct menu-route atlas** audit, not an active-gameplay candidate. Keep menu-only residency separate; test a real menu selection/options route and source/filter/page-fit before any integration. |
| Scene/combat/weather animation and particles | Submitted before postprocess; scene minification may be linear and animation uses changing source textures, shader/wrap or identity contracts. | Different scene-family gate, not a new ordinary point-sampled post-scene family. Orb animation remains excluded by instruction. |

## Disposition

No remaining actually drawn recurrent active-gameplay **post-scene nearest-sampled** PNG family outside the already routed 23 sources was identified. No new dense page is justified: adding an inactive or scene-linear source would spend page area without a demonstrated eligible repeated state saving and could violate filtering/phase contracts. Therefore no runtime patch, manifest regeneration, production build or physical NVE7 B was performed; an A/off–B/on–A/off comparison cannot prove a nonexistent new active draw. No GL draw/bind, timing, memory/VRAM or FPS improvement is claimed from this gate. This is an evidence-backed rejection, not atlas/release acceptance.

Next distinct gate: own **separate actual main-menu point-sampled family** (especially three repeatedly drawn button states plus options music/sfx states), establish source/filter/alpha and dense <=4096 page fit without forcing menu-only textures resident during gameplay, then isolated production build and serial physical NVE7 menu-route A/B/A with full-frame pixels, GL draw/bind, timing and memory proxies. If the workstream must stay gameplay-only, move instead to a specifically scoped scene-linear family with its own filter/edge proof; do not treat it as this point-sampled gate.

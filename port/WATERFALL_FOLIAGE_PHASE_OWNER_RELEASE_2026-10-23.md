# Waterfall foliage phase: dirty-owner release watchdog — 2026-10-23

## Decision

**Do not delegate a second `LevelScene.cpp` writer yet.** At HEAD `ef69c5b` the ID64/65 draw-centre correction is absent. The dirty `LevelScene.cpp/.h` and `main.cpp` diff is the right-stick aim-line integration, including an adjacent `Draw` flip/rotation hunk. Its implementation and neutral-state correction have named owners, but the neutral-state source owner's completion/release is **not authenticated**. Focused CPU passes validate bytes, not ownership transfer. The main.cpp scope repair reports its own completion/verification, but explicitly left that dependent patch uncommitted while LevelScene remained separately owned; it does not release LevelScene. The waterfall execution and later patch-gate workers completed **read-only/CPU-spy handoffs**, not production source integration. No foliage implementation or GPU result may be inferred from those handoffs.

## Source and ownership evidence

- Current `git diff -- port/game/LevelScene.cpp port/game/LevelScene.h port/linux/main.cpp` shows: load-time aim-line reset, `UpdatePlayerVisual` right-stick arguments/hold/direction, `Draw` aim-line flip/rotation, matching `.h` state/signature, and per-frame stick defaults plus forwarding in `main.cpp`. It shows no `foliageSway.worldX/Y` draw assignment. Current SHA-256: `LevelScene.cpp` `634dbc6e407b902075e851e5b8ae150076a16af9c5f1e8ce5374a083d9714ac5`; `.h` `0160f7de603cf5a3aceafded49ab0df78f53e80652610bc97d8efdcaa5512951`; `main.cpp` `ffcae0623ddc1440693913d0b56561a293cd51ad93734ab87c043bb7bd8924e1`.
- `PORT_WORK_STATUS.md:3916,3923,3929,3935,3942,3949` names neutral-state correction owner `4a087854-27e0-4264-8913-138df0556e92`, run `f4d3456e-bd96-4bb4-b935-02113ff9244d`, for LevelScene/right-stick fixture and repeatedly says completion handoff unverified. `port/AIM_LINE_OWNER_RELEASE_RESOLUTION_2026-10-23.md` investigated the plausible `RIGHT_STICK_NEUTRAL_PARITY_FIX` report and found neither explicit owner attribution nor completion/release; `port/AIM_LINE_DIRTY_OWNERSHIP_GATE_2026-10-23.md` independently found the same, despite passing aim-line and strafe CPU fixtures. The untracked `port/tests/right_stick_aim_line_cpu_test.cpp` remains part of the unresolved patch.
- `port/AIM_LINE_MAIN_SCOPE_FIX_2026-10-23.md` owns the per-frame `main.cpp` declaration/scope repair and reports production build and focused fixture passes. It deliberately did not commit/push its coupled work; full integration failed a village-agri expectation then. The current `main.cpp` diff is unchanged from its reported hash. `port/RIGHT_STICK_AIM_LINE_SOURCE_IMPLEMENTATION_2026-10-22.md` is an implementation checkpoint, not release.
- `PORT_WORK_STATUS.md:4555-4556` consumed/completed the waterfall execution worker `5eb1b2d9-aceb-476e-a398-fabc23474e82` and released **that worker's** source/GPU slot, not the earlier aim-line owner's dirty file. `:4581` consumed the negative owner-resolution handoff; `:4659` consumed the completed foliage patch-gate report. `port/WATERFALL_FOLIAGE_PHASE_EXECUTION_2026-10-23.md` establishes actual CPU draw-spy mismatch for 64/65 and ID60 control, while withholding source/GPU work for this ownership reason. `port/WATERFALL_FOLIAGE_PHASE_PATCH_GATE_2026-10-23.md` specifies the fixed-mode test and explicitly forbids a second writer pending release.

## Conflict-free production edit, **after** release

One exclusive LevelScene integration owner may insert the following immediately after `port/game/LevelScene.cpp:1283` (`draw.foliageSway = object.foliageSway;`) and before current `:1284` (`draw.foliageSway.time = shaderClock.totalTime;`). This is textually separate from the dirty aim-line `Draw` lines 1288-1294; no `.h`, `main.cpp`, XML, renderer, shader clock, weather, destination, UV or fixture edit is needed for the production delta:

```diff
         draw.foliageSway = object.foliageSway;
+        if (draw.foliageSway.enabled && !object.player && !object.playerShadow &&
+            !object.playerAimLine && !object.parallaxLayer) {
+            draw.foliageSway.worldX = worldX - (cameraX - worldX) * (layerX - 1.f);
+            draw.foliageSway.worldY = worldY - (cameraY - worldY) * (layerY - 1.f);
+        }
         draw.foliageSway.time = shaderClock.totalTime;
```

The already selected `worldX/Y` and depth multipliers at `LevelScene.cpp:1236-1250` drive the destination at `:1259-1261`; this makes the ordinary foliage shader centre follow the same translated space. Windows `GameObject.cpp:173-205,916-924` computes the update-time parallax-translated World centre and `FoliageSway.fx:40-51` samples the World-transformed vertex before sine. Waterfall XML IDs 64/65 are ordinary depth-1500 foliage sprites with multipliers `(1.10,1.05)`; ID60 at depth 1750 has `(1,1)` and must remain at its XML centre. The source derivation, visible per-ID cameras, and independent sine expectations are in `port/WATERFALL_FOLIAGE_PARALLAX_PHASE_CPU_GATE_2026-10-23.md`. The committed fixed-mode draw spy described in the patch gate is the post-edit CPU discriminator; its prepatch baseline and intentional 64/65 mismatch are **not** postpatch passes.

## Release and risk gate

Coordinator must first obtain the original neutral-state owner's explicit completion/release (or its authenticated final run record) identifying the accepted `LevelScene`/fixture patch. If inaccessible, coordinator must explicitly assume and reconcile that ownership after independent scoped review; inactivity, a plausible report, CPU pass, or another worker's slot release is insufficient. Re-read status, current hashes and scoped diff at that point; assign exactly one integration owner for LevelScene/main-adjacent work, coordinate any forest animation/arm proposals with that owner, and preserve aim-line behavior. Stage/review only intended hunks; never blanket-stage or commit the mixed dirty file. Then run committed foliage spy in `fixed` mode for IDs 64, 65 and 60, focused aim-line/strafe regressions, and scoped diff review before any separately owned build/GPU acceptance.

Residual risks: the single-site formula follows the current draw camera, while Windows can retain the last successful object-update camera when paused or outside update zone; this is an active-draw correction, not full temporal parity. ID65/60 are culled at initial camera `(0,3000)`, so use the per-ID visible cameras in the CPU gate. Bottom-vertex phase includes half the 677-unit sprite height; ID60's authored X flip changes source-corner labels. CPU command agreement does not establish image severity, matched-Windows pixel parity, or Jerome visual acceptance.

This watchdog edited only this new report. It did not modify source, tests, shared checkpoint/status, or use build/GPU lane; no commit/push.

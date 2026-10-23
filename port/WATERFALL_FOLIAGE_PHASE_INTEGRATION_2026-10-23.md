# Waterfall foliage phase integration — 2026-10-23

## Ownership reconciliation

The original neutral-state owner's completion could not be authenticated from the available cross-run records. Under this assignment the coordinator explicitly assumed exclusive integration ownership of `port/game/LevelScene.cpp/.h`, `port/linux/main.cpp`, and the right-stick aim-line fixture after independent scoped review. This is an assumption/reconciliation, not a claim that owner `4a087854-27e0-4264-8913-138df0556e92` sent a release. The preexisting aim-line patch matched the documented neutral-first behavior: load initializes +5/unflipped/zero angle, active nonzero stick sets quadrant direction, neutral preserves the previous transform, strafe holds the line, and per-frame main scope forwards neutral defaults or live stick values. Pre-edit hashes for `.h`, `main.cpp`, and aim-line fixture matched `AIM_LINE_DIRTY_OWNERSHIP_GATE_2026-10-23.md`; the unrelated dirty strafe fixture remained untouched. No status/checkpoint, SDLRenderer, integration fixture, GPU code, or capture was edited.

## Change

Added only the source-backed draw-centre calculation immediately after `draw.foliageSway = object.foliageSway` in `LevelScene::Draw`. For enabled ordinary foliage it uses the existing world position, camera, and depth multipliers to make shader `worldX/Y` follow the same translated centre as the destination. The existing aim-line `Draw` hunk was preserved. This covers the active-draw phase-centre mismatch for waterfall-top IDs 64/65; it does not claim paused/out-of-update-zone temporal parity with Windows.

## Verification

- Fixed-mode actual-XML CPU draw spy: **PASS** for IDs 64, 65, 60. At both visible cameras and times 0/0.5, all four rows per ID report `centre=MATCH bottom_phase=MATCH`. ID60 is the invariant control.
- Focused right-stick aim-line CPU fixture: **PASS**, including neutral-first, active quadrants, launch direction, strafe hold, and neutral fade.
- Focused strafe-body CPU fixture: **PASS**. Its existing dirty file was not staged.
- Forced Docker production `make -C port -B -j1 all`: **PASS**. Only TinyXML warnings. No competing GPU build was running; a separately owned GPU baseline render used its already-built diagnostic binary. No GPU/display capture or matched-Windows comparison was performed in this lane.
- `git diff --check` on scoped source and fixtures: **PASS**.

The committed scope is the reconciled aim-line source/main/fixture, the narrow foliage `LevelScene.cpp` delta, and this handoff only. CPU draw-state verification is not visual or Windows pixel parity. Existing shared-tree unrelated dirt and Jerome visual acceptance remain outside this commit.

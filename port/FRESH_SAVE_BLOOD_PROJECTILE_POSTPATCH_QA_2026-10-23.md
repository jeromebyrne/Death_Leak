# Fresh-save directed blood projectile — bounded postpatch QA (2026-10-23)

## Verdict

**Pass for the high-impact fresh-save gate, with one bounded collision-position parity defect to fix next.** Commit `6cb190c` adds one projectile at the held-Interact Looped→Pull threshold, retains intro-only blood ticking through Pull/completion, and delegates ordinary play to the existing single `Combat::Update` call. No evidence of duplicate spawning, frozen intro flight, ordinary-hit launch regression, or controls/AI being enabled in the intro. This is source/CPU QA, not an independent GPU or Windows visual acceptance.

## Evidence

- `port/linux/main.cpp:640-681`: spawn occurs only under `!hasPulledSword`, Looped, held input and strict `introHeldTime > 2.75f`; phase immediately becomes Pull. `UpdateBloodFlights` runs once in the active intro branch, once on completion, and `Combat::Update` calls it once in ordinary play (`port/game/Combat.cpp:476-477`). This matches the original one-shot call at `Game/src/GameObjects/Player.cpp:1088` without enabling normal combat during the cutscene.
- `port/game/Combat.cpp:346-363,386-395,439-461`: intro launch is `(-1.6,+1.4)*3.5 = (-5.6,+4.9)` without normalization; the immutable direction supplies the 64-unit-width extended collision ray and reverse material-spray direction. Ordinary player-hit launch stays `(0,0)`. Current painter selects flight/impact art and depth 3000/4000 (`:877-891`). The implementation report's physical-NVE7 A/B evidence at frames 180/200 is consistent with a moving localized projectile, but was not independently recaptured here.
- Focused CPU-only regression, compiled to a private `/tmp/blood-postpatch-qa-*` directory with `-UNDEBUG` from current source: `port/tests/player_blood_impact_cpu_test.cpp` **PASS** (first-line splat, material/no-material, inactive envelope, nonpierceable rebound). No shared port build, test file, physical GPU, or `:0` was used.

## Concrete parity finding and next bounded fix

**Low/moderate visual impact at collision, not a fresh-save gate blocker:** the new directed-ray sweep returns a fraction along `old center → (new center + direction*32)`, but `Combat.cpp:407-408` uses that fraction to interpolate `old center → new center` and overwrites the blood sprite's position. Original `Projectile::Update` first moves to the full new center (`Game/src/GameObjects/Projectile.cpp:410-491`); `HandleSolidLineStripCollision` then stops it without moving the position to the geometric contact (`:615-638`). Thus a directed line hit places the Linux impact/rebound sprite short of its source center. For a nominal first tick from `(0,0)`, the new center is `(-5.6,+4.4)` and the extended ray end is `(-56.8,+49.2)`; a contact fraction of `0.5` would place Linux at `(-2.8,+2.2)` instead of source `(-5.6,+4.4)`. The material spray correctly remains at the ray end, so the mismatch can separate the sprite and spray at impact. The existing ordinary-hit CPU fixture does not distinguish this directed case.

**Next owner-sized fix:** retain nearest-hit selection and ray-end spray origin, but on directed collision leave the projectile at `nextX,nextY` before splat jitter/rebound, matching the source's already-advanced center. Add one isolated CPU fixture with a directed ray-only line contact asserting impact position, spray origin, and ordinary zero-direction behavior; then only if that gate passes consider a fresh-save physical visual check when the GPU lane is explicitly free. Do not fold broad-phase/Windows RNG parity into this fix. Existing known approximations (private RNG, segment sweep, port-only orphan bound) remain disclosed by the implementation report.

## Scope and ownership

Only this report was written. Source, tests, shared build, checkpoint, `PORT_WORK_STATUS.md`, and physical GPU were untouched. **Report ownership is released.**

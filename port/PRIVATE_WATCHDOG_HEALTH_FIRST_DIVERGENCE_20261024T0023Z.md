# Health first-divergence gate — CPU-only same-assets route (2026-10-24 00:23Z)

## Verdict

**The current-tree versus pre-`ee289a5` health-hit comparison is blocked before the first encounter.** The first observed state divergence is the forest-1 → waterfall-top loading transition at **frame 398**: after executing frames 0–398 (`--frames 399`), pre-change `a4bfb61` is already in `waterfall_top.xml` at `(-2191.01,3131)`, while current `f7d04b4` plus its existing dirty renderer remains in `forest_exploration_1.xml` at `(3120.99,3120.45)`. Both full traces print the same trigger at frame 398, but only the old simulation advances the level immediately. The current tree's later loading-fade/pause changes therefore alter the scripted movement schedule well before any 3-HP ninja-star contact. This is **not** evidence that knife lifetime caused or cured a hit.

With the identical fixed-clock `R:710,E:40,B:4500` script and same current-tree `Game/` assets, the pre-change run enters forest 2 at frame 1058 and finishes village-central with **1 HP**, eight 3-HP projectile hits. The current run instead crosses from waterfall-top to waterfall-bottom at **frame 788**, finishes waterfall-bottom at **25 HP**, and never spawns an enemy or takes a hit. There is no ordered pair of damage events to compare. The first divergent *observed state* is level/position at frame 398; the exact internal loading-state mutation within that frame was not instrumented.

## Reproducibility and provenance

- Old source: detached worktree `/tmp/deathleak-health-pre-ee289a5-0023`, `ee289a5^ = a4bfb61`. Current source: `f7d04b4` plus pre-existing dirty files; none were modified. `git diff --name-only a4bfb61 HEAD -- Game` was empty. Both binaries were run from the current source root, so both loaded the **same physical assets**.
- CPU-only: `deathleak-linux-build:bookworm` Docker image, `SDL_VIDEODRIVER=dummy`, `SDL_RENDER_DRIVER=software`, `DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1`, `DEATHLEAK_TRACE_ENCOUNTERS=1`, `--simulate-only --fixed-dt 0.0166667`. No GPU/display capture. Build/run products are under ignored `port/build/health-first-divergence-20261024T0023Z/`; the old executable lives in the old worktree's matching ignored subdir. Host compiler lacked SDL/libpng/GL headers, so builds used the existing container image.
- Trace receipts: `port/build/health-first-divergence-20261024T0023Z/pre-ee289a5.log` and `current.log`. Old projectile hit frames are **1461, 1555, 1682, 3746, 3839, 3965, 5042, 5136** in this fresh run, each `damage=3`. They are close to but not identical with the earlier baseline's 3969/5135, so do not silently equate those runs. Old endpoint: `village_central.xml`, `(7287.18,-1525.13)`, 99 orbs, HP 1, spawned 10, kills 2. Current endpoint: `waterfall_bottom.xml`, `(833.383,-286.624)`, 12 orbs, HP 25, spawned 0.
- The prior gate's proposed current-tree R:710 route is thus obsolete after the new transition implementation. Its current-tree health assertions should not be interpreted from this route until route parity is restored.

## Exact next discriminator

Compare the adjacent commit-local pair `ee289a5^` and `ee289a5` with the already-built old binary and a fresh `ee289a5` binary, using the same current assets, clock and script. This removes intervening loading-transition changes. First command (from the repository root; unique worktree/output, no shared source edit):

```sh
git worktree add --detach /tmp/deathleak-health-ee289a5-0023 ee289a5
```

Then build that worktree's `port/linux/main.cpp`, `port/linux/SDLRenderer.cpp`, `port/linux/SDLSoundPlayer.cpp`, `port/game/{LevelScene,Combat,PlayerMotion,Hud}.cpp`, and `Game/tinyxml/{tinyxml,tinyxmlerror,tinyxmlparser,tinystr}.cpp` with the same `g++ -O2 -std=c++17` and `pkg-config --cflags/--libs sdl2 libpng gl` command used here, output to its own ignored `port/build/health-first-divergence-20261024T0023Z/ee289a5`; run it mounted as `/new-binary` against current `/src` assets with the CPU-only environment and command ` /new-binary --play Game/XmlFiles/levels/forest_exploration_1.xml --script R:710,E:40,B:4500 --fixed-dt 0.0166667 --simulate-only`. Compare the ordered `Encounter projectile hit` and `Encounter state` lines to the old log and stop at the first changed hit. If that pair also differs before combat, instrument the first state divergence in that isolated worktree rather than attributing the +3 HP to knives.

No software, tests, shared checkpoints, or expectations changed; no commit made.

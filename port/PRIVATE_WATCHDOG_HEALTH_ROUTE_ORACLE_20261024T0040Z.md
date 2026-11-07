# Forest-2 health route: minimal private oracle and loading boundary (2026-10-24 00:40Z)

## Decision

Use **two separate CPU gates**. The adjacent `a4bfb61` → `ee289a5` gate proves the knife/enemy/shot causal chain on a matched route. The current `f7d04b4`-plus-dirty-tree gate proves that the same forest-start script is **not** a matched route: at frame 398 it enters a loading phase instead of immediately replacing the level. Do not apply the adjacent 1682-frame HP expectation directly to the current tree, change any existing `integration.sh` expected value, or claim Windows parity.

The existing dirty `port/tests/integration.sh` asserts `health=4` for the village/long earned route and changed later death frames. Those are observational current integration expectations, not this private oracle's proof; this lane has not edited them.

## Exact private regression oracle (adjacent pair only)

Prerequisite: run the existing **instrumented detached-worktree binaries** (not a stock binary) with the current physical `Game/` assets mounted read-only. IDs are per-run spawn/shot ordinals emitted by the temporary `PROBE` instrumentation; they are not production IDs. Keep the instrumentation in detached worktrees only. The old/new receipt binaries are `/tmp/deathleak-health-pre-ee289a5-0023/port/build/deathleak-linux` and `/tmp/deathleak-health-ee289a5-0026/port/build/deathleak-linux`. Both use `--play Game/XmlFiles/levels/forest_exploration_1.xml --script R:710,E:40,B:4500 --fixed-dt 0.0166667 --frames 1683 --simulate-only`; 1683 steps include frame 1682. Use `SDL_VIDEODRIVER=dummy`, `SDL_RENDER_DRIVER=software`, `DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1`, `DEATHLEAK_TRACE_ENCOUNTERS=1`, in `deathleak-linux-build:bookworm`; mount the current source root `/src:ro` and the chosen executable read-only. Save stdout in a **unique `/tmp` output**, never in shared `port/build` or `integration.sh`.

The following is an executable assertion against fresh logs, with `OLD_LOG` and `NEW_LOG` set to those unique output paths. For immediate receipt validation, set them to `/tmp/deathleak-health-pre-ee289a5-0023/port/build/provenance-0036.log` and `/tmp/deathleak-provenance-new-0036.log` respectively. This checks event provenance, not just aggregate `hits` or final HP:

```sh
OLD_LOG=/tmp/deathleak-health-pre-ee289a5-0023/port/build/provenance-0036.log \
NEW_LOG=/tmp/deathleak-provenance-new-0036.log python3 - <<'PY'
import os, re
from pathlib import Path
old = Path(os.environ['OLD_LOG']).read_text().splitlines()
new = Path(os.environ['NEW_LOG']).read_text().splitlines()
def require(lines, pattern):
    assert any(re.search(pattern, line) for line in lines), pattern
def forbid(lines, pattern):
    assert not any(re.search(pattern, line) for line in lines), pattern
for lines in (old, new):
    require(lines, r'^PROBE spawn f=1359 e=1 type=kNinja ')
    require(lines, r'^PROBE knife f=1489 s=47 ')
    require(lines, r'^PROBE knife f=1503 s=48 ')
    require(lines, r'^PROBE enemyhit f=1614 s=56 e=1 ')
require(old, r'^PROBE shotend f=1503 s=47 e=0$')
require(old, r'^PROBE shotend f=1517 s=48 e=0$')
for frame, shot in ((1510, 47), (1519, 48)):
    require(new, rf'^PROBE enemyhit f={frame} s={shot} e=1 hp=')
require(new, r'^PROBE enemydeath f=1614 e=1 s=56$')
require(old, r'^PROBE enemyshot f=1668 s=60 e=1 ')
require(old, r'^PROBE playerhit f=1682 s=60 e=1 ')
forbid(new, r'^PROBE enemyshot f=1668 .* e=1(?: |$)')
forbid(new, r'^PROBE playerhit f=1682 ')
require(new, r'^Simulation ended at Game/XmlFiles/levels/forest_exploration_2.xml .*health=19 ')
require(old, r'^Simulation ended at Game/XmlFiles/levels/forest_exploration_2.xml .*health=16 ')
print('adjacent forest-2 knife/shot/HP oracle: PASS')
PY
```

The existing receipt hashes are old `f140e586424a528cfad69b126d7d2703d266925821b5bc052cb8f1285306b17a`, new `5a9dea9ce4d570093bfe482939569df9e630a55250cfc99fc1a370d96d09c16b`. The old enemy-1 shot at 1668 hits at 1682; the new enemy 1 dies at 1614, so that **specific** old shot cannot be emitted. A new shot numbered 60 exists from enemy 2 at 1674, so forbidding all `s=60` would be a false oracle. Source `Game/src/GameObjects/Projectile.cpp` uses the 0.25-s value only after inactivity, while port `ee289a5`'s 10-s active bound is still a surrogate rather than an exact Windows timer.

## Independent current-tree boundary gate

With the same assets/script/fixed dt and `--frames 399 --simulate-only`, check **both** the frame-398 trigger and endpoint level. The old adjacent binary must report the frame-398 trigger then `Simulation ended at .../waterfall_top.xml player=(-2191.01,3131)`; the current-tree binary `/src/port/build/health-first-divergence-20261024T0023Z/current` must report the same trigger then `Simulation ended at .../forest_exploration_1.xml player=(3120.99,3120.45)`. These two CPU-only 399-step reruns were verified for this report using `docker run --rm`, `/src:ro`, dummy/software renderer. Both had HP 25 and seven orbs at that boundary. The current source sets `loadingPhase=FadeIn` at the forest-1 → waterfall-top trigger in `port/linux/main.cpp` (~1425) and defers `nextScene.Load` until the fade-in update (~825-855); the adjacent old route loaded immediately. Thus the first matched **observed** state divergence is frame 398 (not a proven first internal mutation), before enemy combat.

A reproducible one-command shape for either side is:

```sh
docker run --rm -v /srv/openclaw/workspace/death_leak_analysis/source:/src:ro \
  -v /tmp/deathleak-health-pre-ee289a5-0023/port/build/deathleak-linux:/probe:ro \
  -w /src -e SDL_VIDEODRIVER=dummy -e SDL_RENDER_DRIVER=software \
  -e DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER=1 deathleak-linux-build:bookworm \
  /probe --play Game/XmlFiles/levels/forest_exploration_1.xml \
  --script R:710,E:40,B:4500 --fixed-dt 0.0166667 --frames 399 --simulate-only
```

For the current side, change the mounted executable to `/srv/openclaw/workspace/death_leak_analysis/source/port/build/health-first-divergence-20261024T0023Z/current` or run that binary at its existing `/src/...` path. Assert exact transition/endpoint tokens, not elapsed wall-clock time. This boundary gate is a **guard against invalid cross-tree route comparison**, not a reason to force current loading behavior back to the adjacent path.

## Next integration decision

First land/retain the private adjacent oracle after rebuilding or preserving its trace instrumentation in isolated worktrees; do not paste private `PROBE` assertions into the shared broad integration suite without a stable production test hook. Separately derive a current-tree loading-aware route that actually reaches the same forest-2 encounter (or explicitly freeze the pre-loading revision as the scope of the health test), then rerun its event trace and HP endpoint. Only after that matched current-tree result is attributed should anyone adjudicate dirty `integration.sh` health/death expectations. The adjacent result supports **19 HP at frame 1682 and the missing 1668 enemy-1 star on `ee289a5`**, and the earlier full-route adjacent receipt supports its 4-HP endpoint; neither proves current-tree or Windows parity.

No shared source, test, status, or index write was made by this lane; this report is its only write.

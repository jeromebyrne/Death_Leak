# Damage-burst physical-GPU capture route (read-only audit, 2026-10-22)

**Status: recipe only; no GPU hit frames captured or accepted by this audit.** The earlier `POST_DAMAGE_GPU_GAMEPLAY_REVIEW_2026-10-22.md` proves an ordinary post-build forest GPU run, not an accepted hit. Damage-burst implementation is commit `496f906`; this audit used the existing executable for **CPU simulation only** and did not rebuild, invoke X11/GPU/FFmpeg, or alter source/checkpoint files. Current checkout HEAD at audit was `68a0385` with unrelated working-tree changes; verify the exact binary/source identity again before a GPU acceptance run.

## Why the existing hooks suffice

- `--script`, `--fixed-dt`, `--frames`, `--capture` work in rendered `--play`, and `--capture` saves the **last drawn frame before present** as BMP. Thus separate identical-prefix runs at different `--frames` yield precise frames without video timing ambiguity. `--simulate-only` must **not** be used for GPU capture; `H` is simulation-only and cannot demonstrate the rendered enemy path.
- `Combat::Update` advances existing hit ages before new collisions; accepted `DamageEnemy`/`ApplyPlayerDamage` reset only that actor to age 0. `DamageBurstGain` is 5 at hit age 0, 3 at .125 s, and 1 at .25 s. A `--fixed-dt 0.0125` step makes the latter offsets exactly 10/20 additional frames. The player solid contact occurs after `Combat::Update`, also yielding age 0 in its hit frame.
- The existing `solid_boundary_damaging.xml` fixture supplies a damaging collision box and a player body. `R:25,.:20` hits exactly once at zero-based frame **24** and idles thereafter; avoid `R:45`, which re-hits every frame and continually resets the burst. CPU simulation confirmed health 25 at `--frames 24`, then 23 at 25, 35, and 45. `hits=0` is expected for solid contact: the combat hit counter counts projectiles/enemy hits, not this path. `DEATHLEAK_TRACE_ENCOUNTERS=1` emits `Encounter state frame=24 ... health_before=25 health_after=23 solid_damage=2`.
- `encounter_open.xml` spawns a live ninja. `F:20,.:28` fires two normal knives; the second is an accepted enemy projectile hit at zero-based frame **27**. CPU simulation confirmed `hits=0, enemies=1` at `--frames 27`; `hits=1, kills=0, enemies=1` at 28, 38, and 48. The target remains alive and visible; the hit kicks it 30 world units right. Do not use `F:1` (misses) or `H` (simulation-only injection). `--combat-state` also shows the ninja's `combo=1` after the hit.

Those CPU observations locate the expected hit frames; they are **not** proof that rendered GPU output contains a burst. The two fixtures are intentionally sparse, so they test per-actor pixels and adjacent unaffected pixels, **not** authored forest composition. A later matched Windows-state comparison remains a separate gate.

## Physical GPU capture procedure

Prerequisites: repository root as cwd; the already-built binary checked against `496f906` damage-burst source; `deathleak-linux-build:bookworm` image; host LightDM Xorg `:0`, `/dev/dri`, Xauthority at `/var/run/lightdm/root/:0`, Docker access; no competing X11 recording/game lane. These are the machine-specific settings used by `port/tools/run_physical_xorg_route.sh`. Use a new ignored directory, e.g. `port/build/damage-burst-gpu-capture-2026-10-22`, and record the binary hash/mtime and Git HEAD. The following is a proposed command sequence, **not executed by this audit**:

```sh
mkdir -p port/build/damage-burst-gpu-capture-2026-10-22
sha256sum port/build/deathleak-linux > port/build/damage-burst-gpu-capture-2026-10-22/binary.sha256
git rev-parse HEAD > port/build/damage-burst-gpu-capture-2026-10-22/head.txt
for spec in 'player 24 25 35 45' 'enemy 27 28 38 48'; do
  set -- $spec
  actor=$1; shift
  if [ "$actor" = player ]; then
    level=port/tests/fixtures/solid_boundary_damaging.xml
    inputs='R:25,.:20'
  else
    level=port/tests/fixtures/encounter_open.xml
    inputs='F:20,.:28'
  fi
  for count in "$@"; do
    docker run --rm --network host --hostname jerome-Linux-MacBookPro \
      --device /dev/dri \
      --mount "type=bind,src=$PWD,dst=/src" \
      --mount type=bind,src=/tmp/.X11-unix,dst=/tmp/.X11-unix \
      --mount type=bind,src=/var/run/lightdm/root/:0,dst=/tmp/deathleak.Xauthority,readonly \
      -e DISPLAY=:0 -e XAUTHORITY=/tmp/deathleak.Xauthority \
      -e SDL_VIDEODRIVER=x11 -e SDL_RENDER_DRIVER=opengl -e SDL_AUDIODRIVER=dummy \
      -e DEATHLEAK_TRACE_ENCOUNTERS=1 -w /src deathleak-linux-build:bookworm \
      port/build/deathleak-linux --play "$level" --script "$inputs" \
      --frames "$count" --fixed-dt 0.0125 --combat-state \
      --capture "port/build/damage-burst-gpu-capture-2026-10-22/$actor-$count.bmp" \
      > "port/build/damage-burst-gpu-capture-2026-10-22/$actor-$count.log" 2>&1 || exit 1
  done
done
```

Here `player-24` and `enemy-27` are **pre-hit references**; `player-25`/`enemy-28` are age 0; `player-35`/`enemy-38` are age .125; `player-45`/`enemy-48` are age .25. Keep the full 1280×720 BMPs and logs. Crop the actor **with margin and an adjacent unaffected region** from each full frame using the same rectangle per actor (select rectangle by inspecting the pre-hit and 0 frames, then keep it fixed across that actor's four frames). Include the complete sprite at its moved positions; the enemy kickback and player/camera movement make tiny fixed crops misleading. Save lossless crop PNGs and a contact sheet with frame count/age labels; do not color-adjust or rescale individual frames before comparison.

## Objective acceptance criteria

1. All eight runs exit 0 and produce readable, correctly sized full-frame BMPs. Every log says `video_driver=x11 name=opengl`, `accelerated=1 software=0`, `mode=strict-accelerated`, and `OpenGL device vendor=nouveau renderer=NVE7` (or explicitly documented equivalent **physical** device), with no test-software override. Preserve hashes and logs.
2. Hit provenance matches the frame boundaries above: player health 25 before, 23 from hit through endpoint, with exactly the frame-24 trace; enemy `hits=0` before and `hits=1`, `kills=0`, `enemies=1`, `combo=1` from hit through endpoint. If physical logs differ, locate/reverify the real hit boundary before labeling ages; do **not** promote these predicted frame numbers to acceptance.
3. In registered actor crops, the player and ninja visibly brighten at age 0, attenuate at .125, and return to normal by .25, consistent with expected RGB gains 5/3/1. The adjacent non-actor region stays unchanged except for explicable camera/animation movement; no full-scene flash, opaque fringe, disappearing sprite, broken texture UV/flip, or post-hit residual. Compare the pre-hit and endpoint at matched/registered actor pixels, accounting for animation, kickback and scene sepia. Record any visual ambiguity rather than calling it a pass.
4. Full-frame BMPs are composited RGB, so they **cannot numerically prove source-alpha preservation**. Use the existing focused `damage_burst_test.cpp` assertion (`tint.a==255`, opacity unchanged, neighbors at gain 1) and source shader formula as structural evidence; visually check silhouette edges/background show no new opaque halo. A raw GPU alpha assertion would need a separate readback hook and is **not** supplied by this route. Neither these fixture captures nor the prior forest video establishes Windows parity or user acceptance.

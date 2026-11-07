# ID15 same-frame physical-GPU capture gate — 2026-10-23

## Status and prerequisite

**Protocol only. No capture, build, game run, visual acceptance, or Windows parity claim was made in this lane.** At this report's read-only preflight, Docker `port/tests/integration.sh` rerun3 and a `deathleak-linux` fixture were active. The integration owner controls the shared executable/build lane. **Do not execute any command below until that owner explicitly hands off the binary lane, its rerun/build/game children have exited, and the coordinator assigns exclusive physical Xorg `:0`/GPU ownership.** A momentary empty `ps` result is not a handoff. If the owner rebuilds or the hash changes during preflight, abandon this snapshot and establish a new frozen binary after handoff; never compare frames from two different binaries.

Read-only feasibility: `port/game/LevelScene.cpp` implements `DEATHLEAK_RENDER_HIDE_XML_IDS` and `DEATHLEAK_RENDER_ONLY_XML_IDS` as draw-only XML sprite filters. `port/linux/main.cpp` supports deterministic `--script`, `--frames`, `--fixed-dt`, final-frame `--capture`, and a `Simulation ended at ...` endpoint. The earlier targeted route used forest-1, `B:300,F:240`, 540 frames at 0.0166667 s, and emitted baseline/hide-ID15/only-ID15 BMPs with equal endpoints. Existing executable at audit time: 8,974,616 bytes, SHA-256 `4011c2fc6de7a758b46331b9c8bffcf49da933743806f8572f334dcdb318e158`; this is a **historical preflight value**, not permission to assume it is unchanged after integration. Physical `X0` socket, `deathleak-video-capture:bookworm` image, host Python 3/Pillow 10.2.0 were present. The LightDM Xauthority path `/var/run/lightdm/root/:0` was not readable from this sandbox UID, so the authorized GPU operator must verify the host bind/container can actually read it. No matched Windows frame at this exact camera/time/effect state is available.

## Exclusive-run recipe (coordinator/GPU owner, after handoff only)

Run from `/srv/openclaw/workspace/death_leak_analysis/source`. The commands below are a recipe, not a log of execution. Use a fresh evidence directory; do not reuse or overwrite the historical audit. Confirm the Docker image's Xauthority access and `/dev/dri` mapping before launching. Use the previously proven capture-container identity/permissions if the host requires them; do not silently substitute offscreen/llvmpipe.

```sh
# Gate: owner has explicitly released binary/build lane and coordinator has assigned :0 exclusively.
ps -eo pid,ppid,stat,etime,args | grep -E '[d]eathleak-linux|[d]ocker run.*deathleak|[m]ake -C port|[g]++|[f]fmpeg|[X]org'
docker ps --format '{{.Names}} {{.Image}} {{.Command}}'
test -S /tmp/.X11-unix/X0
test -e /var/run/lightdm/root/:0
test -e /dev/dri/renderD128
# Stop here if any build/integration/game/capture owner remains, or if handoff is missing.
OUT="$PWD/port/build/id15-current-tree-gpu-2026-10-23"
test ! -e "$OUT" || { echo 'Evidence directory exists; choose a fresh run name' >&2; exit 1; }
mkdir -p "$OUT"
git rev-parse HEAD > "$OUT/head.txt"
git status --short > "$OUT/tree-status.txt"
sha256sum port/build/deathleak-linux > "$OUT/source-binary.sha256"
cp port/build/deathleak-linux "$OUT/deathleak-linux.frozen"
chmod +x "$OUT/deathleak-linux.frozen"
sha256sum "$OUT/deathleak-linux.frozen" > "$OUT/frozen-binary.sha256"
# Compare the two digest fields before continuing; they must be identical.
```

Within a **single** container, launch the two variants **sequentially**, with no other Xorg game/recorder. The command assumes the host makes the Xauthority bind readable in-container; otherwise stop and fix access through the approved host launch route. It intentionally uses the frozen copy, not the mutable shared binary. `--hostname` mirrors the prior physical-GPU route; if host/container permissions differ, record the exact working launch command and identity.

```sh
docker run --rm --name deathleak-id15-exclusive --network host \
  --hostname jerome-Linux-MacBookPro --device /dev/dri \
  -v "$PWD:/src" -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v /var/run/lightdm/root/:0:/xauthority:ro \
  -e DISPLAY=:0 -e XAUTHORITY=/xauthority \
  deathleak-video-capture:bookworm sh -eu -c '
    cd /src
    out=port/build/id15-current-tree-gpu-2026-10-23
    for mode in baseline hide-15; do
      unset DEATHLEAK_RENDER_HIDE_XML_IDS DEATHLEAK_RENDER_ONLY_XML_IDS
      if [ "$mode" = hide-15 ]; then export DEATHLEAK_RENDER_HIDE_XML_IDS=15; fi
      SDL_VIDEODRIVER=x11 SDL_RENDER_DRIVER=opengl SDL_AUDIODRIVER=dummy \
        "$out/deathleak-linux.frozen" \
        --play Game/XmlFiles/levels/forest_exploration_1.xml \
        --script B:300,F:240 --frames 540 --fixed-dt 0.0166667 \
        --capture "$out/$mode.bmp" > "$out/$mode.log" 2>&1
      test -s "$out/$mode.bmp"
    done
  '
sha256sum "$OUT/deathleak-linux.frozen" > "$OUT/frozen-binary-after.sha256"
sha256sum "$OUT"/baseline.bmp "$OUT"/hide-15.bmp > "$OUT/frame-bmps.sha256"
sha256sum port/build/deathleak-linux > "$OUT/shared-binary-after.sha256"
```

Compare `source-binary.sha256`, `frozen-binary.sha256`, and `frozen-binary-after.sha256` digest fields exactly. The shared-binary-after digest should also match; if not, flag a concurrent rebuild and ask the coordinator whether the frozen snapshot is still the intended current tree. Preserve both raw logs and BMPs; do not edit source/XML/texture or rerun a variant on a different binary.

## Mandatory acceptance of the *diagnostic run* (not visual parity)

1. Both logs must contain `video_driver=x11 name=opengl ... accelerated=1 software=0 ... mode=strict-accelerated`, `OpenGL device vendor=nouveau renderer=NVE7`, and `Sepia.fx GPU postprocess initialized`. Reject offscreen, software, llvmpipe, missing `/dev/dri`, or a capture with no final BMP. Verify 1280×720 decoded dimensions.
2. Extract the entire `Simulation ended at ...` line from each log and require byte-for-byte equality, including level, player coordinates, orbs, health, balance, fired, elapsed, and other fields. The historical expected endpoint was forest-1 `player=(2360.52,3114.74)`, `orbs=5`, but equality between **these two current-tree runs** is the hard gate; any changed endpoint invalidates same-frame attribution.
3. Record SHA-256 of frozen executable before/after and both BMPs. Compute the decoded RGB per-pixel delta, changed count, half-open bbox `[left,top,right,bottom)`, ROI x=900..1199/y=300..549 count, and exact RGB at `(1100,350)` and `(1200,400)` for baseline and hide-ID15. The prior-tree reference was 81,525 changed pixels, bbox `[985,323,1280,612)`, ROI 47,343; samples `(149,129,93)→(95,82,58)` and `(120,103,74)→(52,44,31)`. **Do not demand those values on a changed tree**; report measured current-tree values and discrepancies. A zero delta, delta outside the anticipated shaft region, or unequal endpoint requires investigation, not an art fix.
4. Optionally, under the same frozen binary and exclusive lane, run `DEATHLEAK_RENDER_ONLY_XML_IDS=15` with all other arguments identical to confirm the slab silhouette, and `DEATHLEAK_RENDER_HIDE_XML_IDS=38,39` to distinguish broad white-overlay brightness. Record each gate/hash/endpoint. These are secondary diagnostics; baseline vs hide-15 is the required pair.

Reproducible comparison after capture (host Pillow is present):

```sh
BASE="$OUT/baseline.log"; HIDE="$OUT/hide-15.log"
grep -E 'SDL renderer selected|OpenGL device|Sepia.fx GPU postprocess|Loaded .*original level sprites|Simulation ended at' "$BASE" "$HIDE"
base_end=$(grep '^Simulation ended at ' "$BASE" | tail -n 1)
hide_end=$(grep '^Simulation ended at ' "$HIDE" | tail -n 1)
test -n "$base_end" && test "$base_end" = "$hide_end" || { echo 'Endpoint mismatch' >&2; exit 1; }
python3 - "$OUT/baseline.bmp" "$OUT/hide-15.bmp" <<'PY'
from PIL import Image, ImageChops
import sys
base = Image.open(sys.argv[1]).convert('RGB')
hide = Image.open(sys.argv[2]).convert('RGB')
assert base.size == hide.size == (1280, 720), (base.size, hide.size)
diff = ImageChops.difference(base, hide)
red, green, blue = diff.split()
mask = ImageChops.lighter(ImageChops.lighter(red, green), blue)
roi = (900, 300, 1200, 550)
print('size', base.size, 'changed_pixels', sum(bool(v) for v in mask.getdata()))
print('bbox_half_open', mask.getbbox())
print('roi_xyxy_half_open', roi, 'changed_pixels', sum(bool(v) for v in mask.crop(roi).getdata()))
for point in ((1100, 350), (1200, 400)):
    print('pixel', point, 'baseline', base.getpixel(point), 'hide15', hide.getpixel(point))
PY
```

A current-tree hide delta establishes Linux-renderer contribution of authored ID15 at this fixed frame, **not** original-Windows parity or a mandate to remove/retouch the shaft. Matched original-Windows forest-1 camera, viewport, simulation/effect phase, and ID15 frame are still missing. Human review and release acceptance remain separate gates.

# Post-UV forest-1 gameplay visual review — 2026-10-22

## Verdict

Commit `60faa5e` (ordinary sprite vertical UV correction) is at local HEAD and was verified at `origin/feature/linux-renderer-foundation` via `git ls-remote`. A new **direct game-window**, active forest-1 video was captured after the level-entry/ambient-audio log appeared. It is H.264 1280×720 at 30 fps, exactly 300 readable frames/10.000000 seconds, with a clean full decode. The capture ran alone on physical Xorg `:0`, SDL X11/OpenGL `accelerated=1 software=0 mode=strict-accelerated`, nouveau/NVE7, GPU `Sepia.fx`, 71 original sprites and zero missing/unsupported assets. The MP4 has no game-audio track (dummy audio for this scripted check).

Artifact: ignored `port/build/post-uv-video-2026-10-22/forest-1-post-uv-active-10s.mp4` (10,795,638 bytes, SHA-256 `2b3ecf69d926d520983fe0afd25e5261f2418bfb8ec78dce0e342496bf4b6514`). Evidence in that directory: `record.sh`, `game.log`, `window.txt`, `ffprobe.txt`, empty `decode.log`, and `still-1s.png` / `still-5s.png` / `still-9s.png`. `record.sh` used the prior known-good game-window/forest-audio gate, then recorded only that window; it did not include menu, setup, desktop, or lock screen. Its scripted input was `B:300,F:99700` (move right/fire, then fire). The stills differ across the entire frame, and show player/camera progression, orbs 2→5, projectile activity, HUD, forest layers, and sepia.

## Representative visual comparison

The earlier clip `port/build/pixel-wobble-gpu-video-2026-10-22/forest-1-pixel-wobble-active-10s.mp4` predates `60faa5e`. Its 5-second still has conspicuous light, flat horizontal ground/fog strips around the lower-middle of the frame. The new 5-second still shows more of the tree trunks and illustrated ground through that area, with the ground color/opacity concentrated lower. This is qualitatively consistent with the source-backed ID17/19 V correction and the controlled same-frame before/after attribution in `port/MIDDLE_BAND_UV_ATTRIBUTION.md`. The two 5-second wall-clock stills are **not the same simulation frame/camera**; no pixel-level before/after metric is inferred from them.

Steam `store_media/screenshots/14.jpg` is a recognizable forest-1 composition (tree/branch, bridge, orb grouping) and supports only a qualitative check: the new early still has the same authored illustrated forest language and layered occlusion, but different player/camera/state and HUD count. The Steam image has softer, brighter atmospheric treatment in places; this alone does not identify a renderer bug. Dark foreground occlusion and the remaining rectangular-looking atmospheric edges may warrant focused source-shader attribution, but ID37's near-black area was already traced to authored alpha/depth/UV in `port/ID37_PARITY_FOLLOWUP.md`. No concrete new source-backed defect was established by these unmatched frames; **no code or art change was made**.

## Limits and next priority

This is a Linux physical-GPU gameplay capture, not matched Windows pixel parity or Jerome's visual acceptance. The next highest-value fidelity check is a matched original-Windows forest-1 lossless frame with camera, viewport, elapsed frame/effect clock, player and weather/pickup state recorded, followed by a controlled Linux match. If that is unavailable, a bounded foliage-sway/pixel-wobble shader orientation/UV audit can test the remaining mid-band behavior against Windows source, without changing authored XML or ID37 on appearance alone.

Discord delivery was **not attempted**: this child has no verified Jerome Discord user/channel destination and no Discord message/file-send tool in its available tool set. A coordinator with a valid destination should deliver the above MP4 for human review; this report does not claim delivery.

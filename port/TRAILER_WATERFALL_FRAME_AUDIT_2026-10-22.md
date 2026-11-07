# Steam trailer waterfall/raft-frame audit — 2026-10-22

## Result

**Negative within the available local trailer.** No original Windows frame matching the raft/ledge composition of store screenshot 16 appears in `store_media/steam-trailer-reference-30s.mp4`. This says nothing about trailers beyond this saved 30-second excerpt or other footage.

## Reference landmarks

Store `store_media/screenshots/16.jpg` (1200×675) shows a large open central drop, two separate raft-like ledges entering from the lower left and lower right and ending toward the middle, an airborne character above the gap, pale pickups in the upper-middle/right and center, and dark foliage along the bottom. The ledges are low in the image and slope inward; there is no continuous platform bridging the middle.

## Trailer audit and reproducibility

- Decoded the saved MP4 using GStreamer software video decode only, with no game execution, display, or GPU. It contains **1,800 video frames**, 1920×1080, timestamps **0.000–29.983 seconds** at 60 fps (30.000-second stream).
- Reviewed the visual sequence at one-second intervals from 0 through 29 seconds. It stays in a moonlit, multi-platform arena: broad continuous or staggered platforms and combat occupy most of the clip. The early open-ground run (roughly 0–2 s) and late open-ground run (roughly 28–30 s) have no opposing raft ends.
- Every decoded frame was also compared to screenshot 16 after both were converted to grayscale at 160×90. Mean absolute pixel difference supplied candidate frame IDs; this is a coarse retrieval aid, **not** proof of scene identity. The best candidate in each five-second block was inspected visually:
  - frame **273**, **4.550 s** — stacked continuous platforms, not two raft ends.
  - frame **340**, **5.667 s** — stacked arena platforms.
  - frame **660**, **11.000 s** — combat on continuous platforms, with blood at lower right.
  - frame **1144**, **19.067 s** — long platform across upper/middle field.
  - frame **1436**, **23.933 s** — multiple staggered platforms across the screen; overall nearest grayscale score, but composition is wrong.
  - frame **1638**, **27.300 s** — scattered short arena platforms, not the paired low ledges and central void.
- The top whole-frame numerical candidates were frame 1436 (23.933 s), 1437 (23.950 s), 1438 (23.967 s), 1439 (23.983 s), 1440 (24.000 s), and frame 660 (11.000 s). Their shared sepia palette/central brightness explains the coarse similarity; their platform geometry does not match screenshot 16.

**Conclusion:** This 30-second saved trailer cannot supply an original Windows waterfall-top/raft reference frame for the screenshot-16 camera composition. Use screenshot 16 itself as the available original visual reference unless another source video is found.

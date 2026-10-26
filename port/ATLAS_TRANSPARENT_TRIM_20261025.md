# Transparent-margin atlas trim — 2026-10-25

The packer now crops each sprite to its alpha bounds plus a two-pixel transparent filtering guard where available, then applies its existing two-pixel edge extrusion. Each v3 manifest entry records `original_size`, `trim_offset`, and the packed `rect`; the single-page runtime lookup has the same eight numeric fields. Runtime intersects authored source crops with the retained content, scales and repositions the destination to the original virtual canvas, accounts for X/Y flip, and rotates around the *original* destination center. Repeat/shift/private shader paths retain their dedicated-source fallback. An all-transparent image retains a one-pixel transparent payload.

The source images have nonzero hidden RGB under zero alpha, so exact source-byte reconstruction is not meaningful after trimming. Packer verification reconstructs and compares every alpha byte and RGB bytes wherever alpha is nonzero. All 21 currently integrated sprites pass, and regenerated page/manifest/lookup hashes are deterministic.

- Gamepad icons: source rectangles 468,992 → 446,634 pixels (4.8% saved); page 2156×320 → 2146×315.
- Gameplay HUD: 26,008 → 21,740 pixels (16.4% saved); page 234×132 → 212×124.
- Scene branches: 738,691 → 694,625 pixels (6.0% saved); page 1746×607 → 1710×576.

Isolated Linux candidate compiled in the existing Docker toolchain. Serial physical Xorg `:0`/nouveau NVE7/OpenGL strict accelerated captures of Forest B:180, fixed dt, control versus all three opt-in atlas families: 1,185/921,600 RGB pixels differed (0.129%), 1,174 with max-channel delta 1, ten with 2, one with 3; no larger delta. Renderer reported accelerated=1, software=0. This is near-parity, **not** byte-identical. Capture/logs are under ignored `port/build/atlas-trim/`. No FPS gain or release acceptance is claimed.

Other textures remain outside the currently integrated atlas families. The page-size ceiling remains 4096, not a target size. Future family additions must use the v3 lookup and separately verify their active display paths.

# Modal/pause point-sampled atlas gate — 2026-10-26

## Source eligibility

Clean detached source at exact remote base `470bb541e6214efdda3a20b47b2828f5b3855c48`; primary dirty checkout was never built or overwritten. The XML `pause_menu.xml` names `pause_backing_2.png`, but the Linux runtime never loads or draws it; it is not eligible for an active-route bind saving. The actual post-scene paper and kDeflection upgrade modals use `blank_dark.png`, already in the HUD atlas, and `key_esc.png`/`y.png`, already in the icon atlas. The distinct unatlased art is `Game/Media/UI/orb.png`: `main.cpp` loads it and draws it during the upgrade modal at `{520,460,56,85}` from a 167×255 full RGBA source. It is an ordinary post-scene point-sampled copy, no wrap, source crop, flip, private shader or tint override. The modal remains visible for repeated frames until dismissed. `--test-upgrade-modal` is a fixed-dt/capture-only fixture selecting this actual draw branch; without the flag, gameplay behavior is unchanged.

## Asset/build gate

The deterministic full-RGBA packer added `orb.png` to the prior thirteen-source gameplay-HUD page. Fourteen source rectangles and their two-pixel nearest-extruded edges byte-reconstruct. One trimmed 2504×1343 page, both dimensions <4096, source-rectangle occupancy 85.93%; no second page. Default `DEATHLEAK_ATLAS_MODAL_ORB=on` maps the active draw through the existing HUD virtual-region path, while `off` keeps only the orb dedicated and all previously accepted HUD atlas routes on. Whole-HUD off remains a broader control. Isolated Docker production build passed, binary SHA-256 `5eb8e82404442b1ec9d8365963c457ecd8c14a25d88efef4be5e0fb0237bc684`.

## Physical NVE7 evidence

Serial authenticated Xorg :0 SDL/OpenGL accelerated=1 software=0 vsync=1, nouveau/NVE7 1280×720, fixed 1/60-dt, three-present forced actual upgrade-modal route. Orb-only A(off)/B(on)/A(off), with established HUD atlas held on, produced byte-identical full RGB captures (0/921,600 pixels changed), SHA-256 `48cedbdce5b5b50217ba743042e720417370cf48b9f0b00617e55af55644294f`. Traced captures were likewise exact. A no-modal control differed from modal control by 831,604 RGB pixels, proving the modal branch really drew. This parity is bounded to the modal fixture and its three frames, not an all-scene claim.

Physical apitrace warm frames 1–2 each: `glDrawArrays` **35/34/35**, `glBindTexture` **102/101/102**, texture uploads **0/0/0** for orb-only A/B/A. Three-frame totals: draws **105/102/105**, binds **1070/1065/1070** (includes initialization), uploads **382/381/382** (initialization). The incremental modal orb saves one warm draw submission and one warm bind per frame; no batching claim is inferred from atlas identity alone. Serial untraced 360-frame profiles, 120 warmup/240 measured: frame-time p50 **21.2500/21.2707/21.2346 ms**, p95 **24.5554/24.5269/24.5412 ms**, p99 **27.2693/24.5563/24.5837 ms**. Controls bracket candidate p50/p95; no repeatable FPS/tail gain. Capture/readback last-frame maxima were ~50 ms in all arms.

Uncompressed RGBA page proxy rises from 13,140,992 to 13,451,488 bytes (+310,496); replacing the 167×255 dedicated orb avoids 170,340 bytes, for a **net +140,156 bytes** versus the prior page plus dedicated source. Direct driver VRAM residency and thermal behavior were not measured. PNG page disk bytes rise 899,597→937,115; source remains on disk for fallback.

## Disposition

Accept the bounded, default-on point-sampled modal orb in the gameplay-HUD atlas: exact physical modal pixels, one draw/bind per warm modal frame saved, small raw texture-area cost, no FPS improvement. Coverage rises **22→23/896**. Pause backing remains an inactive-source exception; paper/upgrade buttons and dark background were already covered by their existing atlases. Do not claim 8K/high-Hz/60-FPS/sustained/thermal/release acceptance. Next distinct gate: audit an actually drawn recurrent post-scene family outside existing pages; reject XML-only assets at source gate before GPU work.

Raw captures/logs/traces/profiles: `/srv/openclaw/workspace/death_leak_analysis/modal-atlas-evidence/`.

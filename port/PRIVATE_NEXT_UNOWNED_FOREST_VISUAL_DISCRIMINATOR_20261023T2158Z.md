# Next unowned Forest visual discriminator — 2026-10-23 21:58Z

## Verdict

**No new, unowned source-backed visual discrepancy is proven by the current physical-GPU Forest clip.** Do not nominate a renderer, XML, or art correction from this evidence. This is a bounded negative finding, not original-Windows parity or visual acceptance.

## Evidence and ownership

The reviewed current artifact is `port/build/private-running-rate-order-gpu-20261023t2145z/forest-running-10s.mp4` (SHA-256 `d89089f178c0d4cf5fc5ea4d5da67ac9fa0f874f4cb10166833e661e635219c1`), H.264 1280×720, 30 fps, 300 frames/10 s. Its saved 1/5/9 s stills show the bridge-to-right-ground route, authored branches/trunks/orbs, moving player, and HUD. `game.log` confirms X11/OpenGL accelerated=1/software=0, nouveau NVE7, GPU Sepia, and 71 loaded original sprites with zero missing/unsupported assets. The stills are wall-clock H.264 samples, not atomic state packets.

The only newly controlled visual delta in this packet is **already owned Running body timing**: `PRIVATE_RUNNING_RATE_ORDER_GPU_GATE_20261023T2145Z.md` compares pre-exact and current exact ordinals 60/180, finding respectively 5,882 and 4,573 changed RGB pixels confined to player-local boxes, and zero changed pixels outside those boxes. Original `Character.cpp:1006-1056` / `AnimationPart.cpp:142-196` select/advance Running before storing the velocity-derived rate; `PRIVATE_FOREST_BODY_RUNNING_RATE_ORDER_20261023T2136Z.md` and the GPU gate own that correction. These Linux A/B frames cannot establish original-Windows appearance.

The ~1 s curved bridge, right branch, pickup and nearby orbs are the authored Forest-1 landmark neighborhood (XML IDs 11/57/58, 30/29, 117), already identified in `FOREST_CURRENT_CLIP_LANDMARK_QA_2026-10-23.md`; its Steam screenshot 14 has different camera, pose, orb count and compression. No current still isolates a new geometric offset there. The broad pale middle field and bottom shrub are already causally assigned to authored IDs 38/39, 17/19 and ID37 in `FOREST_POST_ID37_MIDDLE_FIELD_GPU_GATE_2026-10-23.md` / `FOREST_MIDDLE_BAND_CURRENT_FRAME_GPU_CAUSAL_2026-10-23.md`, without a matched-Windows residual. The conspicuous late player/tree issue, waterfall upper curtain, Sepia and delivery are deliberately outside this lane and have separate owners. The arrow/HUD/orbs visible during travel do not themselves constitute a source-backed discrepancy; their appearance alone has no matched original state.

## One narrow next test, only when original reference is available

Acquire **one original-D3D10 lossless frame plus same-render-ordinal draw/state sidecar at the early bridge/right-branch landmark corresponding to the current controlled ordinal 60**. Record original executable/content hashes, viewport, camera, player position and animation pose, orb/HUD state, render/update clock, and ordered raft IDs 11/57/58, branch IDs 30/29 and pickup ID117 commands (texture, destination, UV, flip, alpha/blend, sampler/mips). Capture a Linux frame using that recorded state/history, then register the static raft/branch/pickup landmarks and compare their object-local lossless pixels **outside the player mask**. A first divergent draw field with a registered pixel residual would become a distinct owner; agreement would close the early-composition suspicion. This is different from re-testing Running body pose, late player occlusion, atmosphere or waterfall. The current clip and unmatched Steam JPEG cannot execute this comparison; `FOREST_WINDOWS_MATCHED_FRAME_CURRENT_ROUTE_2026-10-23.md` documents the absent qualified original runtime/packet.

## Scope and release

Read-only inspection of existing clip, stills, logs, source-backed reports and provenance. Only this unique report was written. No GPU, build, source, tests, shared status, index, commit or unrelated file was touched. **Read-only visual lane released.**

# Frame-415 v1 atomic packet schema correction — 2026-10-23 23:46Z

## Result

Corrected only the two source-proven extra `transition` members in `port/linux/main.cpp`: `firstPostfadeOrdinal` and `loadingSplashSubmitted` are now serialized only for `postfadePacket`. The frame-415 schema tag remains `deathleak-linux-frame415-v1`, with its saved original key/type shape; the postfade schema tag and its two fields remain intact. Common transition fields and JSON-last atomic write are unchanged. The latest camera-fix report (`PRIVATE_WATCHDOG_WATERFALL_ENTRY_CAMERA_FIX_20261023T2343Z.md`) was read; this patch does not touch that camera fix.

## CPU verification

`python3 port/tests/private_frame415_v1_schema_cpu_20261023t2346z.py` — **PASS**. The fixture parses the saved original frame-415 packet, the later regressed frame-415 packet, and the latest camera-fix postfade frame-424 packet. It verifies the only recursive key/type difference between saved frame-415 packets is the two new transition keys; confirms original-v1 lacks them and postfade retains integer/bool types; confirms postfade-only pre-HUD/draw identifiers and frame-415 unavailable ordered-draw shape; and asserts the serializer has a unique conditional source seam for the two keys. `git diff --check -- port/linux/main.cpp` — **PASS**.

This is a CPU/source-contract gate, not a fresh runtime packet or GPU capture. The separately noted same-run frame-415/424 pairing limitation and Windows parity question remain outside this bounded change.

## Scope

Only `port/linux/main.cpp`, the focused CPU fixture, and this report are included in the scoped commit. Existing unrelated dirty worktree files were not staged, edited, or tested.

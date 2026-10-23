# Encounter-roll scoped closure — 2026-10-23

## Fixture repair and ownership

The `B:330` to `B:220,F:120` encounter-roll fixture repair is already isolated in commit `63855f2749cad2cd389de101027137275b339d42` (`Repair range-kill encounter integration script`), which is both current HEAD and the remote `feature/linux-renderer-foundation` tip. Its commit diff changes only the `range_kill` hunk of `port/tests/integration.sh` and adds the scoped regression report. No duplicate fixture commit or push was needed. The same script has 22 other dirty added/deleted lines covering village/orb/health route expectations; these were present on entry and were not staged or modified by this lane. Other shared gameplay, visual, and status edits remain untouched.

## Verified gates

- Previous full `sh port/tests/integration.sh` process exited 0; its log `port/build/encounter-roll-gate-recovery-full.log` ends in the overall `PASS:` marker. This is the current shared dirty-tree integration gate, not a clean-HEAD claim.
- Next bounded non-GPU gate: forced `make -C port -B -j1 TARGET=/out/deathleak-linux all` in the existing `deathleak-linux-build:bookworm` container, mounting the repository read-only and output under `/tmp/deathleak-scoped-build-nW7MmK`. **PASS (exit 0)**. The relevant main, scene, header, and integration-script SHA-256 values were identical before and after compilation. Only pre-existing TinyXML warnings were emitted. This did not overwrite `port/build/deathleak-linux`.
- The isolated current-tree binary, using the test-only offscreen Mesa allowance and `--simulate-only --combat-state`, passed the focused `encounter_roll.xml --script B:220,F:120` endpoint: `spawned=1 enemies=0 hits=15 kills=1`, player health 22. No physical Xorg/GPU used.

A first isolated-binary invocation omitted the test-only software-renderer allowance and correctly refused llvmpipe in strict mode. It was rerun with the same test-only settings used by integration; the result above is from that successful run.

## Checkpoint

Fixture repair: **closed and already pushed**. Full integration: **PASS on shared dirty tree**. Independent isolated build/focused encounter: **PASS**. The broader port remains partial; unrelated dirty edits and other acceptance gates are outside this lane.

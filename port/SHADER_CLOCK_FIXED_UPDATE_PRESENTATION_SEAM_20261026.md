# Shader-clock fixed-update presentation seam — 2026-10-26

Base: remote `feature/linux-renderer-foundation` at `0a07f4d6c36c0b63ac2163d2442f06cd8deb05d2`. Clean detached source and baseline worktrees; the primary checkout's seven unrelated dirty tracked files and untracked files were not used as build input or staged.

## Decision: accept bounded seam

`ShaderClock::Prepare()` performs the old `Draw()` phase/direction mutation exactly once after each fixed update and before the existing catch-up `continue`/presentation fork. `Draw() const` now returns only the prepared total/wobble clock. `RenderCompleted(dt)` stays at its original branch locations, so the previous-rendered-delta ordering and strict >10/<5 oscillator reversal remain unchanged for native one-update/one-render 60-Hz play. Intermediate live catch-up steps still prepare wobble as the old code did. Menu, modal and scene-reset clocks remain process-owned. No update-free draw loop was added.

The assertions-enabled C++17 purity test passed 1,800 updates and 1.8 million repeated same-clock const draws without changing time or reversal state. The scene-clock fixture passed menu/modal, fresh scene, foliage and strict reversal thresholds after updating its call contract and stale renderer interface. Final **native-clock** Docker production build passed (SHA-256 `649d523cce75e82058903636e025c0f7758412a3fe99ef4cec9441c531243616`); `git diff --check` passed.

Strict serial physical NVE7 gate: X11 `:0`, SDL OpenGL Mesa nouveau NVE7, accelerated=1 software=0 vsync=1, 1280×720, fixed-dt 1/60 fresh-save Forest-1 held-Interact `I:100` active intro frame 100. Baseline/candidate/baseline full post-HUD BMPs are byte-identical (SHA-256 `e717d9c2358438b4001ddec1e8a3732fc0bf169d00976bdd1d23df03902b48eb`, 0 changed pixels); gameplay summaries match. Both builds used an **identical test-only** `frame*1000/60` gameplay-film clock to remove unrelated native wall-clock phase. This fixture was removed from candidate before final production build/commit. BMP/logs retained under ignored primary `port/build/shader-clock-20261026/`.

This proves one bounded read-only shader-clock presentation seam and synchronized fixed-60 pixels on this route. It does **not** prove arbitrary native film-clock A/B parity, extra >60-Hz emitted frames, 8K support, sustained FPS or release acceptance. Resource residency and packet/capture side effects remain separate presentation impurities.

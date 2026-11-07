# Strafe body CPU fixture repair — 2026-10-22

Scope: `port/tests/strafe_body_cpu_test.cpp` only. No runtime, build gate, or shared checkpoint edits; no compile, Docker, GPU, Xorg, or capture.

## Repair

- Preserved the existing correct `vx=-600` clock expectation: entry `dt=.030` uses XML 30 FPS and remains `_019`; the next `.001` reaches `.031`, crosses stored 47.7 FPS (`1/47.7 ≈ .02096436`) and advances to `_018`; a further `.001` leaves the residual at about `.01104` and holds `_018`. Later same-sequence re-entry at `.021` crosses persisted 47.7 FPS after clock reset. QA's finding referred to an earlier `vx=-100` fixture, not this already-updated high-speed row.
- Added a fresh `vx=-100` discriminator for that QA timing error: `.010` entry holds `_019`; cumulative `.034` after `.024` still holds `_019` because stored rate is 7.95 FPS (`1/7.95 ≈ .12578616`); cumulative `.126` after another `.092` advances exactly one frame to `_018`.
- Made every draw path-classified: exactly one Player body, at most one distinct back arm and one aim line; an unknown draw, duplicate body, or arm substitution now fails. When an arm exists it must immediately precede body with identical destination rectangle. Initial Still must show arm+body; StrafeBack must show body alone. The aim-line comparison now includes width/height, and the strafe draw requires body then aim line with no arm.
- Retained explicit ordered observation of all 20 forward `_000.._019` and all 20 reverse `_019.._000` frames. Texture IDs are used only as path-derived equality handles, not numeric-order evidence.

## Remaining limits and handoff

Static-only review: this fixture has **not been compiled or run** and is not registered in Makefile/integration. When the reserved build lane releases, compile with assertions enabled (`-UNDEBUG`) and normal `LevelScene.cpp`, `PlayerMotion.cpp`, and four tinyxml implementation files plus `-Iport/compat`; then execute from repository root with the real Game assets. Resolve any compile/runtime finding before registering as a gate.

This fixture checks the port's focused raw-helper and body behavior. It does not prove real SDL disconnect/KeyDown integration, Windows first-entry pose timing, post-landing impact-recovery duration, wet/sprint/slow-motion rate variants, exact footstep metadata, or Windows pixel parity. Running-only spray is checked for non-emission under strafe, not a positive Running emission in this fixture.

# Linux first post-fade Waterfall atomic packet — 2026-10-23 23:29Z

## Result

Implemented an opt-in `--atomic-postfade-waterfall-packet <json>` diagnostic in `port/linux/main.cpp`. It requires a distinct `--capture <bmp>`, positive `--frames`, `--fixed-dt`, `--script`, and `--play`; it is disabled in ordinary gameplay. At the final requested render it commits the same-run post-HUD/post-loading BMP and then a temporary-file-renamed JSON, but only when the *observed* Waterfall transition has completed: loading was seen, the first subsequent fully active draw has phase None, no splash command/alpha, and gameplay/weather/scene/player updates all ran. The frame ordinal is measured, not baked into the state predicate; this recipe yielded 424.

The frame-415 packet was independently checked. Its saved BMP hash and XML hash match its JSON and its transition fields describe first-reveal gameplay. Its claimed executable SHA-256 **does not** match its saved binary: the original code called `sha256sum /proc/self/exe` after fork/exec, so it hashed `sha256sum` rather than the game. This change fixes the parent-process executable path. A new frame-415 regression run confirms image and executable hashes match its sidecar. The original frame-415 artifact remains valid for its image and partial transition state, not executable provenance.

## Physical evidence

Ignored local directory: `port/build/private-watchdog-postfade-frame424-atomic-20261023t2329z/`. It contains unique `deathleak-linux`, `build.log`, `capture.sh`, `capture.log`, `frame424.bmp`, `frame424.json`, review `frame424.png`, and separate `frame415-recheck` BMP/JSON/log. Built with `deathleak-linux-build:bookworm`, `make -C port TARGET=build/private-watchdog-postfade-frame424-atomic-20261023t2329z/deathleak-linux`; pass with existing TinyXML warnings. Physical Xorg `:0` / nouveau `NVE7`, SDL x11/OpenGL strict accelerated (`accelerated=1 software=0 vsync=1`), Sepia GPU scene before HUD; no software fallback. Fresh process input: Forest-1, `--script B:1800`, `--fixed-dt 0.016666667`, `--frames 425`. Trigger at 398; Waterfall installed. Capture exit 0.

Independent Python JSON/BMP decode and SHA-256 checks passed:

- Final `frame424.bmp`: 1280×720×32, 3,686,522 bytes, SHA-256 `6edb480dc30d47b5f67d4f0e95ce94ea177b1a3a9aec8a4f5d863b5a1e9764d8` = JSON image hash.
- Same-run executable: `e1cb29bff04bc9755eb1db3cb7df3799be29cac258cbb51954c25e0060a29eb2` = JSON executable hash. Waterfall XML: `d1e8a7a4986a54f0d5ba55329237adced9283cd3fc540e32604299967cde02b6` = JSON XML hash.
- `frame424.json`: SHA-256 `86dfeb7474609a28cb918c518e60a14ead38db9a9ccfd3c24f4683835d531b92`. Render/update/first-postfade ordinal all 424; phase on entry 3 (FadeOut), at draw 0 (None); `loadingSplashSubmitted=false`, alpha and tint sentinel -1, pause false, all four update flags true. Draw camera `(-1082.38098,3000)` and player `(-2139.01465,3114.6958)`; health 25, focus 100, seven orbs. This is active Waterfall gameplay after loading UI retirement, not a splash capture.
- Separate fixed frame-415 run: image SHA-256 `a10cc3a7a796c90562a18396c7976c3b05fbb449c65a0a9d48a6475d8c4ef358` and JSON SHA-256 `b7667d922c680b3c709246a08be4877ab85710716a85ca619010e641db44633b`; both image/executable identity checks pass. Phase 3 and positive fractional loading alpha verified. A malformed opt-in invocation exits 2 before capture; `git diff --check` passed.

## Limits and ownership

This is a **single final image plus partial same-ordinal state**, not an original D3D10 matched-state packet. `matchedStateEligible=false`: actual submitted matrices, ordered per-object scene draws/texture hashes, body/arm clip-frame, displayed HUD fill, content manifest, and effect RNG history remain unavailable. No pre-UI or pre-splash readback is in this packet. It cannot establish original parity or classify any residual image difference without a source-built original same-state packet. The exact next discriminator remains acquiring both platforms' same-state pre-UI and final images plus ordered draw/HUD state.

Only previously clean `port/linux/main.cpp` and this unique report were edited. Preexisting dirty renderer/tests/shared checkpoints and unrelated untracked reports were untouched. Physical GPU/Docker lane released after captures; temporary X authorization removed.

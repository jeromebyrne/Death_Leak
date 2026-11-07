# Post-knife non-GPU release gate — 2026-10-23 21:09Z

## One highest-priority actionable failure: grounded keyboard cannot shoot

**Source-proven, reproducible input failure / scripted-suite blind spot.** In unscripted living gameplay with no connected/right-aiming controller, holding S or Down while grounded always requests crouch, and that same `Down` state is the *only* keyboard fire predicate. `crouchInput` is true, so `fire` is false on every such frame. There is no separate keyboard-fire key in the renderer's `KeyDown` switch. Jumping permits an airborne Down shot, but that is not a usable grounded attack control. A scripted `F`/`B` route bypasses this live-input arbitration, so `integration.sh` can pass its grounded knife/combat/earned-route assertions while this live keyboard path remains broken. This is a bounded Linux keyboard-adaptation failure, not a claim about original Windows keyboard parity or about physical scancode delivery.

### Exact evidence at HEAD `c5d3a00`

- `port/linux/SDLRenderer.cpp:764-769` maps `Down` to S/Down or gamepad D-pad Down. Its full `KeyDown` switch at `:763-775` defines no independent fire key; `port/render/Renderer.h:95` likewise has no `Fire` action.
- `port/linux/main.cpp:793-795` computes `crouchInput = crouchHeld && motion.Grounded() && motion.WallSide()==0 && !motion.Rolling() && !combat.MeleeActive()`, with `crouchHeld = renderer->KeyDown(Down)` when no script. At `:857`, live fire is `(renderer->KeyDown(Down) && !crouchInput) || stickAim`. At `:839-840`, `stickAim` requires a connected controller and active right axis. Thus grounded, neutral, ordinary keyboard S/Down yields `(true && !true) || false == false`.
- `port/tests/integration.sh:58-59,105-110` uses `--script F:60` and scripted combined fire routes; its live audio checks at `:872-884` do not actuate a keyboard. `PRIVATE_CROUCH_ANIMATION_OWNER_20261023T1845Z.md` explicitly notes grounded crouch suppresses overlapping Down-to-fire mapping, but does not assign a keyboard-fire repair. The 20:32Z real-input gate only proposes physical D/Right movement; it does not validate attack input. Latest checkpoint and current visual gate do not close this input issue.

### Executable CPU discriminator (no GPU or Docker)

Run this read-only source/boolean oracle now from repo root; it exits nonzero if the cited source wiring changes, and prints the failing reachable state:

```sh
python3 - <<'PY'
from pathlib import Path
main = Path('port/linux/main.cpp').read_text()
sdl = Path('port/linux/SDLRenderer.cpp').read_text()
assert 'const bool crouchHeld = !script.empty() ? command == \'C\' : renderer->KeyDown(Down);' in main
assert 'const bool fire = (!script.empty() ? (command == \'F\'' in main
assert ': (renderer->KeyDown(Down) && !crouchInput)) || stickAim;' in main
assert 'case Down: return keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]' in sdl
assert 'case Fire:' not in sdl
keydown_down = grounded = True
wall = rolling = melee = stick_aim = False
crouch = keydown_down and grounded and not wall and not rolling and not melee
fire = (keydown_down and not crouch) or stick_aim
assert crouch and not fire
print('FAIL-GATE: grounded keyboard Down/S crouches and cannot fire; scripted F is independent')
PY
```

This proves the control-logic branch, not actual SDL event delivery. After correction, add a **CPU fake-Renderer/main input-arbitration fixture** (or extract a pure resolver) that drives grounded neutral keyboard fire separately from crouch, and asserts one accepted knife attempt/`fired>0` on the corrected key, crouch without fire on S/Down, plus airborne, right-stick and script controls. Then execute that fixture under a normal C++ test command with no display. Do not substitute a scripted `F` test for the unscripted keyboard branch.

### Next owner scope and disposition

Assign a non-GPU input/gameplay owner to `port/linux/main.cpp`, `port/linux/SDLRenderer.cpp`, `port/render/Renderer.h`, a new focused CPU fixture, and `port/README.md` control documentation; touch `port/tests/integration.sh` only after its current dirty owner releases it. Introduce an independent keyboard fire binding or action, keep S/Down crouch and right-stick fire semantics, and gate the live branch with the focused fixture. The physical-keyboard event gate remains a separate later check. No implementation, build, Docker, binary execution, GPU/Xorg, shared checkpoint/status edit, stage/commit/push was done in this lane; all existing dirty changes were preserved. This unique report is the only file written.

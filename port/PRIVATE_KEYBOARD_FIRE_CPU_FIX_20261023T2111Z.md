# Grounded Linux keyboard fire — CPU closure, 2026-10-23 21:11Z

## Change

- Added backend-neutral `Fire` action and mapped physical SDL F scancode to it; S/Down and D-pad Down still map only to `Down`.
- Centralized crouch/fire arbitration in `ResolveGameplayAttackInput`. On ground, S/Down crouches without firing; F fires independently, including while crouched. Airborne Down, active right-stick aim, and scripted F/B/N/Y/Z remain fire routes.
- Live `main.cpp` uses the resolver; crouch launch state now reflects an actual simultaneous crouch+fire attempt. README controls corrected.

## Focused non-GPU evidence

From repo root:

```sh
cd port && g++ -std=c++17 -O0 -UNDEBUG -Icompat tests/keyboard_fire_cpu_test.cpp game/LevelScene.cpp game/Combat.cpp ../Game/tinyxml/tinyxml.cpp ../Game/tinyxml/tinyxmlerror.cpp ../Game/tinyxml/tinyxmlparser.cpp ../Game/tinyxml/tinystr.cpp -o /tmp/deathleak-keyboard-fire-cpu-test && /tmp/deathleak-keyboard-fire-cpu-test
```

Output: `keyboard fire CPU: grounded knife fired, crouch/air/stick/script passed`.
The fake Renderer sets the independent action on a grounded neutral frame and calls real `Combat::Update`; `Fired()` increases by one and a knife exists. It also asserts grounded S/Down does not fire, airborne Down does, right-stick fire does, scripted F does, and F+Down can fire crouched. `g++ -std=c++17 -fsyntax-only -Iport/compat port/linux/main.cpp` passed. SDL `case Fire: return keys[SDL_SCANCODE_F];` was source-checked; SDL2 development package is absent from this local sandbox, so SDLRenderer compilation and physical scancode delivery were not verified here.

## Residual acceptance gate

Use a physical keyboard on a live rendered Linux session to press F while grounded and observe a knife, then hold S/Down to confirm crouch does not fire by itself; check F+S if desired. This gate is not replaced by the fake Renderer or scripted F path. No GPU/Xorg or shared build work was done here.

// SDL virtual-device mapping/lifecycle gate, not physical controller acceptance.
#include "../render/Renderer.h"
#include <SDL.h>
#include <cassert>
#include <iostream>
#include <string>

int main() {
    using namespace deathleak::render;
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    assert(SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) == 0);
    const auto attach = [] {
        const int index = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,
            SDL_CONTROLLER_AXIS_MAX, SDL_CONTROLLER_BUTTON_MAX, 0);
        assert(index >= 0 && SDL_IsGameController(index));
        return index;
    };
    int index = attach();
    auto renderer = CreateRenderer(true);
    std::string error;
    assert(renderer->Initialize("Input backend gate", 320, 180, false, error));
    SDL_Joystick* pad = SDL_JoystickOpen(index);
    assert(pad && renderer->ControllerAxes().connected);
    auto tick = [&] { SDL_JoystickUpdate(); renderer->PollQuit(); };
    const auto button = [&](SDL_GameControllerButton button, Key key) {
        assert(SDL_JoystickSetVirtualButton(pad, button, 1) == 0); tick();
        assert(renderer->KeyDown(key));
        assert(SDL_JoystickSetVirtualButton(pad, button, 0) == 0); tick();
        assert(!renderer->KeyDown(key));
    };
    button(SDL_CONTROLLER_BUTTON_A, Up);
    button(SDL_CONTROLLER_BUTTON_A, MenuAccept);
    button(SDL_CONTROLLER_BUTTON_A, Restart);
    button(SDL_CONTROLLER_BUTTON_X, Interact);
    button(SDL_CONTROLLER_BUTTON_Y, Purchase);
    button(SDL_CONTROLLER_BUTTON_LEFTSHOULDER, Roll);
    button(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, Melee);
    button(SDL_CONTROLLER_BUTTON_START, Escape);
    button(SDL_CONTROLLER_BUTTON_B, ModalBack);
    button(SDL_CONTROLLER_BUTTON_DPAD_LEFT, Left);
    button(SDL_CONTROLLER_BUTTON_DPAD_RIGHT, Right);
    button(SDL_CONTROLLER_BUTTON_DPAD_UP, MenuUp);
    button(SDL_CONTROLLER_BUTTON_DPAD_DOWN, MenuDown);
    assert(SDL_JoystickSetVirtualAxis(pad, SDL_CONTROLLER_AXIS_LEFTX, -16000) == 0);
    assert(SDL_JoystickSetVirtualAxis(pad, SDL_CONTROLLER_AXIS_LEFTY, 16000) == 0);
    assert(SDL_JoystickSetVirtualAxis(pad, SDL_CONTROLLER_AXIS_RIGHTX, 20000) == 0);
    assert(SDL_JoystickSetVirtualAxis(pad, SDL_CONTROLLER_AXIS_RIGHTY, -20000) == 0); tick();
    assert(renderer->KeyDown(Left) && renderer->KeyDown(MenuDown));
    const auto axes = renderer->ControllerAxes();
    assert(axes.leftX == -16000 && axes.rightX == 20000 && axes.rightY == -20000);
    assert(SDL_JoystickDetachVirtual(index) == 0); tick();
    assert(!renderer->ControllerAxes().connected);
    assert(!renderer->KeyDown(Left) && !renderer->KeyDown(MenuDown));
    SDL_JoystickClose(pad);
    index = attach(); tick();
    assert(renderer->ControllerAxes().connected);
    assert(SDL_JoystickDetachVirtual(index) == 0); tick();
    assert(!renderer->ControllerAxes().connected);
    // SDL_PushEvent deliberately does NOT update SDL_GetKeyboardState: this
    // models remote synthetic delivery and down/up coalesced in one pump.
    auto key = [&](Uint32 type, SDL_Scancode scan, SDL_Keycode symbol, SDL_Keymod mods = KMOD_NONE) {
        SDL_Event event{}; event.type = type; event.key.type = type;
        event.key.keysym.scancode = scan; event.key.keysym.sym = symbol;
        event.key.keysym.mod = mods; assert(SDL_PushEvent(&event) == 1);
    };
    for (const auto& mapping : {std::pair<SDL_Keycode, Key>{SDLK_a, Left},
         {SDLK_d, Right}, {SDLK_w, Up}, {SDLK_s, Down}, {SDLK_LEFT, Left},
         {SDLK_RIGHT, Right}, {SDLK_UP, Up}, {SDLK_DOWN, Down}}) {
        key(SDL_KEYDOWN, SDL_SCANCODE_UNKNOWN, mapping.first); tick();
        assert(renderer->KeyDown(mapping.second));
        tick(); assert(renderer->KeyDown(mapping.second)); // Held, no repeat required.
        key(SDL_KEYUP, SDL_SCANCODE_UNKNOWN, mapping.first); tick();
        assert(!renderer->KeyDown(mapping.second));
        key(SDL_KEYDOWN, SDL_SCANCODE_UNKNOWN, mapping.first);
        key(SDL_KEYUP, SDL_SCANCODE_UNKNOWN, mapping.first); tick();
        assert(renderer->KeyDown(mapping.second)); // At least one update for a remote tap.
        tick(); assert(!renderer->KeyDown(mapping.second));
    }
    key(SDL_KEYDOWN, SDL_SCANCODE_D, SDLK_d); tick();
    SDL_Event focus{}; focus.type = SDL_WINDOWEVENT;
    focus.window.event = SDL_WINDOWEVENT_FOCUS_LOST;
    assert(SDL_PushEvent(&focus) == 1); tick(); assert(!renderer->KeyDown(Right));
    SDL_Event click{}; click.type=SDL_MOUSEBUTTONDOWN; click.button.button=SDL_BUTTON_LEFT;
    assert(SDL_PushEvent(&click)==1);
    click.type=SDL_MOUSEBUTTONUP; assert(SDL_PushEvent(&click)==1);
    tick(); assert(renderer->Mouse().leftDown);
    tick(); assert(!renderer->Mouse().leftDown);
    key(SDL_KEYDOWN, SDL_SCANCODE_Q, SDLK_q, KMOD_CTRL);
    assert(renderer->PollQuit());
    key(SDL_KEYUP, SDL_SCANCODE_Q, SDLK_q); tick();
    key(SDL_KEYDOWN, SDL_SCANCODE_F4, SDLK_F4, KMOD_ALT);
    assert(renderer->PollQuit());
    SDL_Event close{}; close.type = SDL_WINDOWEVENT;
    close.window.event = SDL_WINDOWEVENT_CLOSE; assert(SDL_PushEvent(&close) == 1);
    assert(renderer->PollQuit());
    renderer.reset();
    std::cout << "PASS: SDL initial gamepad, mapped buttons/axes, detach fallback and hot-plug reconnect; remote/local key edges, focus reset and emergency/window exits\n";
}

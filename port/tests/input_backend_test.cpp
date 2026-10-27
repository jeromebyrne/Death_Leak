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
    renderer.reset();
    std::cout << "PASS: SDL initial gamepad, mapped buttons/axes, detach fallback and hot-plug reconnect\n";
}

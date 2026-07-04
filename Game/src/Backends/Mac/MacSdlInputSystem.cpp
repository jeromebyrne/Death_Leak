#include "Backends/Mac/MacSdlInputSystem.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr float kAxisMax = 32767.0f;
constexpr float kAxisDeadZone = 0.18f;

float NormalizeAxis(Sint16 value)
{
    float normalized = static_cast<float>(value) / kAxisMax;
    if (std::fabs(normalized) < kAxisDeadZone)
    {
        return 0.0f;
    }

    return std::clamp(normalized, -1.0f, 1.0f);
}

Uint16 NormalizeRumble(float value)
{
    const float clamped = std::clamp(value, 0.0f, 1.0f);
    return static_cast<Uint16>(clamped * 0xFFFF);
}
}

MacSdlInputSystem::~MacSdlInputSystem()
{
    if (mController != nullptr)
    {
        SDL_GameControllerClose(mController);
        mController = nullptr;
    }
}

void MacSdlInputSystem::Update()
{
    mKeyboardState = SDL_GetKeyboardState(nullptr);
    Controller();
}

bool MacSdlInputSystem::IsDown(InputButton button) const
{
    if (mKeyboardState == nullptr)
    {
        return false;
    }

    switch (button)
    {
        case InputButton::Pause:
            return mKeyboardState[SDL_SCANCODE_ESCAPE] || ControllerButtonDown(SDL_CONTROLLER_BUTTON_START);
        case InputButton::Left:
            return mKeyboardState[SDL_SCANCODE_A] || mKeyboardState[SDL_SCANCODE_LEFT] || LeftStickX() < -kAxisDeadZone;
        case InputButton::Right:
            return mKeyboardState[SDL_SCANCODE_D] || mKeyboardState[SDL_SCANCODE_RIGHT] || LeftStickX() > kAxisDeadZone;
        case InputButton::Up:
            return mKeyboardState[SDL_SCANCODE_W] || mKeyboardState[SDL_SCANCODE_UP] || LeftStickY() > kAxisDeadZone;
        case InputButton::Down:
            return mKeyboardState[SDL_SCANCODE_S] || mKeyboardState[SDL_SCANCODE_DOWN] || LeftStickY() < -kAxisDeadZone;
        case InputButton::Jump:
            return mKeyboardState[SDL_SCANCODE_SPACE] || ControllerButtonDown(SDL_CONTROLLER_BUTTON_A);
        case InputButton::Roll:
            return mKeyboardState[SDL_SCANCODE_LSHIFT] || ControllerButtonDown(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        case InputButton::Sprint:
            return mKeyboardState[SDL_SCANCODE_LSHIFT] || ControllerButtonDown(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        case InputButton::Melee:
            return mKeyboardState[SDL_SCANCODE_E] || ControllerButtonDown(SDL_CONTROLLER_BUTTON_X);
        case InputButton::PrimaryWeapon:
            return mKeyboardState[SDL_SCANCODE_J] || ControllerButtonDown(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
        case InputButton::SecondaryWeapon:
            return mKeyboardState[SDL_SCANCODE_K] || ControllerButtonDown(SDL_CONTROLLER_BUTTON_LEFTSTICK);
        case InputButton::SlowMotion:
            return mKeyboardState[SDL_SCANCODE_L] || AxisValue(SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 0.5f;
    }

    return false;
}

float MacSdlInputSystem::LeftStickX() const
{
    return AxisValue(SDL_CONTROLLER_AXIS_LEFTX);
}

float MacSdlInputSystem::LeftStickY() const
{
    return -AxisValue(SDL_CONTROLLER_AXIS_LEFTY);
}

float MacSdlInputSystem::RightStickX() const
{
    return AxisValue(SDL_CONTROLLER_AXIS_RIGHTX);
}

float MacSdlInputSystem::RightStickY() const
{
    return -AxisValue(SDL_CONTROLLER_AXIS_RIGHTY);
}

void MacSdlInputSystem::SetVibration(float leftPercent, float rightPercent, float seconds)
{
    SDL_GameController* controller = Controller();
    if (controller == nullptr)
    {
        return;
    }

    const Uint32 durationMs = static_cast<Uint32>(std::max(0.0f, seconds) * 1000.0f);
    SDL_GameControllerRumble(
        controller,
        NormalizeRumble(leftPercent),
        NormalizeRumble(rightPercent),
        durationMs);
}

SDL_GameController* MacSdlInputSystem::Controller() const
{
    if (mController != nullptr)
    {
        return mController;
    }

    const int joystickCount = SDL_NumJoysticks();
    for (int i = 0; i < joystickCount; ++i)
    {
        if (SDL_IsGameController(i) == SDL_TRUE)
        {
            mController = SDL_GameControllerOpen(i);
            break;
        }
    }

    return mController;
}

float MacSdlInputSystem::AxisValue(SDL_GameControllerAxis axis) const
{
    SDL_GameController* controller = Controller();
    if (controller == nullptr)
    {
        return 0.0f;
    }

    return NormalizeAxis(SDL_GameControllerGetAxis(controller, axis));
}

bool MacSdlInputSystem::ControllerButtonDown(SDL_GameControllerButton button) const
{
    SDL_GameController* controller = Controller();
    if (controller == nullptr)
    {
        return false;
    }

    return SDL_GameControllerGetButton(controller, button) == 1;
}

#ifndef DEATHLEAK_BACKENDS_MAC_MACSDLINPUTSYSTEM_H
#define DEATHLEAK_BACKENDS_MAC_MACSDLINPUTSYSTEM_H

#include "Engine/Platform/IInputSystem.h"

#include <SDL.h>

class MacSdlInputSystem final : public IInputSystem
{
public:
    MacSdlInputSystem() = default;
    ~MacSdlInputSystem() override;

    void Update() override;
    bool IsDown(InputButton button) const override;
    float LeftStickX() const override;
    float LeftStickY() const override;
    float RightStickX() const override;
    float RightStickY() const override;
    void SetVibration(float leftPercent, float rightPercent, float seconds) override;
    bool IsKeyDown(SDL_Scancode scancode) const;

private:
    SDL_GameController* Controller() const;
    float AxisValue(SDL_GameControllerAxis axis) const;
    bool ControllerButtonDown(SDL_GameControllerButton button) const;

    mutable SDL_GameController* mController = nullptr;
    const Uint8* mKeyboardState = nullptr;
};

#endif

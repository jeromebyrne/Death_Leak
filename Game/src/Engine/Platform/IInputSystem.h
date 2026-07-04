#ifndef DEATHLEAK_ENGINE_PLATFORM_IINPUTSYSTEM_H
#define DEATHLEAK_ENGINE_PLATFORM_IINPUTSYSTEM_H

enum class InputButton
{
    Pause,
    Left,
    Right,
    Up,
    Down,
    Jump,
    Roll,
    Sprint,
    Melee,
    PrimaryWeapon,
    SecondaryWeapon,
    SlowMotion
};

class IInputSystem
{
public:
    virtual ~IInputSystem() = default;

    virtual void Update() = 0;
    virtual bool IsDown(InputButton button) const = 0;
    virtual float LeftStickX() const = 0;
    virtual float LeftStickY() const = 0;
    virtual float RightStickX() const = 0;
    virtual float RightStickY() const = 0;
    virtual void SetVibration(float leftPercent, float rightPercent, float seconds) = 0;
};

#endif

#ifndef DEATHLEAK_ENGINE_RUNTIME_GAMELOOPCONTROLLER_H
#define DEATHLEAK_ENGINE_RUNTIME_GAMELOOPCONTROLLER_H

#include "Engine/Audio/IAudioSystem.h"
#include "Engine/Platform/IInputSystem.h"
#include "Engine/Platform/IPlatformApp.h"
#include "Engine/Platform/ITimeSource.h"
#include "Engine/Services/ISteamService.h"

#include <functional>

struct GameLoopConfig
{
    double TargetDeltaSeconds = 1.0 / 60.0;
    int MaxFrames = -1;
};

class GameLoopController
{
public:
    GameLoopController(
        IPlatformApp& platform,
        IInputSystem& input,
        IAudioSystem& audio,
        ISteamService& steam,
        ITimeSource& timeSource);

    int Run(const GameLoopConfig& config, const std::function<void(float)>& frameCallback);

private:
    IPlatformApp& mPlatform;
    IInputSystem& mInput;
    IAudioSystem& mAudio;
    ISteamService& mSteam;
    ITimeSource& mTime;
};

#endif

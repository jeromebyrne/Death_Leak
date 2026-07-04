#include "Engine/Runtime/GameLoopController.h"

#include "Core/Timing.h"

GameLoopController::GameLoopController(
    IPlatformApp& platform,
    IInputSystem& input,
    IAudioSystem& audio,
    ISteamService& steam,
    ITimeSource& timeSource):
    mPlatform(platform),
    mInput(input),
    mAudio(audio),
    mSteam(steam),
    mTime(timeSource)
{
}

int GameLoopController::Run(const GameLoopConfig& config, const std::function<void(float)>& frameCallback)
{
    int frameCount = 0;
    while (mPlatform.PumpEvents())
    {
        mInput.Update();
        mAudio.Update();
        mSteam.Update();

        if (Timing::Instance() != nullptr)
        {
            Timing::Instance()->Update(static_cast<float>(config.TargetDeltaSeconds));
        }

        frameCallback(static_cast<float>(config.TargetDeltaSeconds));

        if (config.MaxFrames >= 0 && ++frameCount >= config.MaxFrames)
        {
            break;
        }

        mTime.SleepUntilNextFrame(config.TargetDeltaSeconds);
    }

    return 0;
}

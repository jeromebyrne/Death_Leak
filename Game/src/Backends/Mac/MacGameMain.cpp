#include "precompiled.h"

#include "Backends/Mac/MacFileSystem.h"
#include "Backends/Mac/MacLaunchPaths.h"
#include "Backends/Mac/MacNullAudioSystem.h"
#include "Backends/Mac/MacNullSteamService.h"
#include "Backends/Mac/MacSdlInputSystem.h"
#include "Backends/Mac/MacSdlPlatformApp.h"
#include "Core/Graphics.h"
#include "Engine/Runtime/GameLoopController.h"
#include "Engine/Platform/ITimeSource.h"
#include "Game.h"
#include "Core/Timing.h"
#include "StringManager.h"

#include <filesystem>
#include <chrono>
#include <thread>

CSteamAchievements* g_SteamAchievements = nullptr;

void PostDestroyMessage()
{
}

class ChronoTimeSource final : public ITimeSource
{
public:
    double Seconds() const override
    {
        const auto elapsed = std::chrono::steady_clock::now() - mStart;
        return std::chrono::duration<double>(elapsed).count();
    }

    void SleepUntilNextFrame(double targetDeltaSeconds) override
    {
        mNextFrameTime += targetDeltaSeconds;
        const auto target = mStart + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
            std::chrono::duration<double>(mNextFrameTime));
        const auto now = std::chrono::steady_clock::now();
        if (target > now)
        {
            std::this_thread::sleep_for(target - now);
        }
    }

private:
    std::chrono::steady_clock::time_point mStart = std::chrono::steady_clock::now();
    double mNextFrameTime = 0.0;
};

int main()
{
    Timing::Create();

    PlatformWindowConfig config;
    config.Title = "Death Leak";
    config.Width = 1280;
    config.Height = 720;

    MacSdlPlatformApp app;
    MacSdlInputSystem input;
    MacNullAudioSystem audio;
    MacNullSteamService steam;
    MacFileSystem files(MacLaunchPaths::DetectRepoRoot());
    Graphics graphics;

    if (!app.Initialise(config))
    {
        return 1;
    }

    if (!audio.Initialise())
    {
        return 1;
    }
    audio.SetMasterVolume(0.5f);

    steam.Initialise(995990);
    StringManager::Create();
    StringManager::GetInstance()->SetIsOnSteamDeck(steam.IsSteamDeck());

    if (FAILED(graphics.Initialise(app.Window(), config.Width, config.Height, 0.0f, 1.0f, 0.0f, 0.0f)))
    {
        return 1;
    }

    Timing::Instance()->SetTargetDelta(1.0 / 60.0);

    Game::Create(&files);
    Game* game = Game::GetInstance();
    game->Initialise();

    ChronoTimeSource timeSource;
    GameLoopController loop(app, input, audio, steam, timeSource);
    GameLoopConfig loopConfig;
    loopConfig.TargetDeltaSeconds = 1.0 / 60.0;

    loop.Run(loopConfig, [game](float delta) {
        game->Update(delta);
        game->Draw();
        game->PostDraw();
    });

    Game::Destroy();
    return 0;
}

#include "Engine/Audio/IAudioSystem.h"
#include "Engine/Platform/IFileSystem.h"
#include "Engine/Platform/IInputSystem.h"
#include "Engine/Platform/IPlatformApp.h"
#include "Engine/Platform/ITimeSource.h"
#include "Engine/Platform/PlatformDefines.h"
#include "Engine/Render/IRenderer.h"
#include "Engine/Services/ISteamService.h"
#include "Backends/Mac/MacFileSystem.h"
#include "Backends/Mac/MacSdlInputSystem.h"
#include "Backends/Mac/MacSdlPlatformApp.h"
#include "Backends/Mac/MacSdlRenderer.h"
#include "Core/DataValue.h"
#include "Core/Timing.h"
#include "Core/Vector2.h"
#include "Core/XmlDocument.h"
#include "Core/XmlUtilities.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

namespace
{
class ChronoTimeSource final : public ITimeSource
{
public:
    double Seconds() const override
    {
        const auto now = std::chrono::steady_clock::now();
        const std::chrono::duration<double> elapsed = now - mStart;
        return elapsed.count();
    }

    void SleepUntilNextFrame(double targetDeltaSeconds) override
    {
        std::this_thread::sleep_for(std::chrono::duration<double>(targetDeltaSeconds));
    }

private:
    std::chrono::steady_clock::time_point mStart = std::chrono::steady_clock::now();
};
}

int main()
{
    MacSdlPlatformApp app;
    MacSdlInputSystem input;
    ChronoTimeSource time;
    MacFileSystem files(std::filesystem::current_path());
    Timing::Create();

    PlatformWindowConfig config;
    config.Title = "Death Leak";
    config.Width = 1280;
    config.Height = 720;

    if (!app.Initialise(config))
    {
        return 1;
    }

    std::cout << "Death Leak SDL macOS smoke app initialised: "
              << config.Width << "x" << config.Height << "\n";

    MacSdlRenderer renderer(app.Window());
    if (!renderer.Initialise(config.Width, config.Height))
    {
        return 1;
    }

    int frameCount = 0;
    while (app.PumpEvents() && frameCount < 120)
    {
        input.Update();
        if (input.IsDown(InputButton::Pause))
        {
            app.RequestQuit();
        }

        Timing::Instance()->Update(1.0f / 60.0f);
        renderer.BeginFrame(Color{0.02f, 0.015f, 0.01f, 1.0f});
        if (frameCount % 30 == 0)
        {
            std::cout << "macOS smoke frame at " << time.Seconds()
                      << "s leftX=" << input.LeftStickX()
                      << " jump=" << (input.IsDown(InputButton::Jump) ? "true" : "false")
                      << "\n";
        }
        renderer.EndFrame();
        time.SleepUntilNextFrame(1.0 / 60.0);
        ++frameCount;
    }

    Vector2 vectorCheck(3.0f, 4.0f);
    DataValue dataValueCheck("12.3456789");
    XmlDocument xmlDocument;
    const bool loadedSettings = xmlDocument.Load(files.AssetPath("XmlFiles\\settings.xml"));
    bool musicEnabled = false;
    if (loadedSettings)
    {
        musicEnabled = XmlUtilities::ReadAttributeAsBool(xmlDocument.GetRoot(), "audio", "music_enabled");
    }
    std::cout << "portable core sample: length=" << vectorCheck.Length()
              << ", value=" << dataValueCheck.asFloat() << "\n";
    std::cout << "xml sample: settings loaded=" << (loadedSettings ? "true" : "false")
              << ", music=" << (musicEnabled ? "true" : "false") << "\n";
    std::cout << "asset sample: " << files.AssetPath("Media\\UI\\title.png") << "\n";
    std::cout << "save sample: " << files.SavePath("save.xml") << "\n";
    std::cout << "Death Leak macOS smoke app shutdown\n";
    return 0;
}

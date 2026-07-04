#ifndef DEATHLEAK_ENGINE_PLATFORM_IPLATFORMAPP_H
#define DEATHLEAK_ENGINE_PLATFORM_IPLATFORMAPP_H

#include <string>

struct PlatformWindowConfig
{
    std::string Title = "Death Leak";
    int Width = 1920;
    int Height = 1080;
    bool Fullscreen = false;
};

class IPlatformApp
{
public:
    virtual ~IPlatformApp() = default;

    virtual bool Initialise(const PlatformWindowConfig& config) = 0;
    virtual bool PumpEvents() = 0;
    virtual void RequestQuit() = 0;
    virtual bool ShouldQuit() const = 0;
    virtual void SetCursorVisible(bool visible) = 0;
};

#endif

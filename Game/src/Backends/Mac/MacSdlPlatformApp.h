#ifndef DEATHLEAK_BACKENDS_MAC_MACSDLPLATFORMAPP_H
#define DEATHLEAK_BACKENDS_MAC_MACSDLPLATFORMAPP_H

#include "Engine/Platform/IPlatformApp.h"

#include <SDL.h>

class MacSdlPlatformApp final : public IPlatformApp
{
public:
    MacSdlPlatformApp() = default;
    ~MacSdlPlatformApp() override;

    bool Initialise(const PlatformWindowConfig& config) override;
    bool PumpEvents() override;
    void RequestQuit() override;
    bool ShouldQuit() const override;
    void SetCursorVisible(bool visible) override;

    SDL_Window* Window() const { return mWindow; }

private:
    SDL_Window* mWindow = nullptr;
    bool mShouldQuit = false;
};

#endif

#include "Backends/Mac/MacSdlPlatformApp.h"

#include <iostream>

namespace
{
SDL_Window* gCurrentWindow = nullptr;
}

SDL_Window* MacSdlPlatformApp::CurrentWindow()
{
    return gCurrentWindow;
}

MacSdlPlatformApp::~MacSdlPlatformApp()
{
    if (mWindow != nullptr)
    {
        SDL_DestroyWindow(mWindow);
        mWindow = nullptr;
    }

    if (gCurrentWindow != nullptr)
    {
        gCurrentWindow = nullptr;
    }

    SDL_Quit();
}

bool MacSdlPlatformApp::Initialise(const PlatformWindowConfig& config)
{
    SDL_SetMainReady();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) != 0)
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
    if (config.Fullscreen)
    {
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    mWindow = SDL_CreateWindow(
        config.Title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        config.Width,
        config.Height,
        windowFlags);

    if (mWindow == nullptr)
    {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        return false;
    }

    gCurrentWindow = mWindow;

    SDL_ShowWindow(mWindow);
    SDL_RaiseWindow(mWindow);

    return true;
}

bool MacSdlPlatformApp::PumpEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0)
    {
        if (event.type == SDL_QUIT)
        {
            RequestQuit();
        }
        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
        {
            RequestQuit();
        }
    }

    return !ShouldQuit();
}

void MacSdlPlatformApp::RequestQuit()
{
    mShouldQuit = true;
}

bool MacSdlPlatformApp::ShouldQuit() const
{
    return mShouldQuit;
}

void MacSdlPlatformApp::SetCursorVisible(bool visible)
{
    SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
}

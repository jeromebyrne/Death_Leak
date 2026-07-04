#ifndef DEATHLEAK_WINDOWS_WINDOWSGAMERUNTIME_H
#define DEATHLEAK_WINDOWS_WINDOWSGAMERUNTIME_H

#include "Engine/Runtime/IGameRuntime.h"
#include "Windows/WindowsFileSystem.h"

#include <windows.h>

class DXWindow;
class Graphics;
class Game;
class CSteamAchievements;

class WindowsGameRuntime final : public IGameRuntime
{
public:
    WindowsGameRuntime();
    ~WindowsGameRuntime();

    int Run(HINSTANCE hInstance, int nCmdShow);
    LRESULT HandleWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);
    void Update(float delta);
    void Render();
    void Present();
    void Shutdown();

    DXWindow* mWindow = nullptr;
    Graphics* mGraphics = nullptr;
    Game* mGame = nullptr;
    CSteamAchievements* mAchievements = nullptr;
    WindowsFileSystem mFileSystem;
    bool mDestroyGame = false;
    bool mSteamInitialised = false;
};

#endif

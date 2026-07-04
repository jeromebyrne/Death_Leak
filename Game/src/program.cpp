#include "precompiled.h"
#include "Windows/WindowsGameRuntime.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    WindowsGameRuntime runtime;
    return runtime.Run(hInstance, nCmdShow);
}

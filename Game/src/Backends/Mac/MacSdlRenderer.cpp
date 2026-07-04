#include "Backends/Mac/MacSdlRenderer.h"

#include <algorithm>
#include <iostream>

namespace
{
Uint8 ToByte(float value)
{
    return static_cast<Uint8>(std::clamp(value, 0.0f, 1.0f) * 255.0f);
}
}

MacSdlRenderer::MacSdlRenderer(SDL_Window* window):
    mWindow(window)
{
}

MacSdlRenderer::~MacSdlRenderer()
{
    if (mRenderer != nullptr)
    {
        SDL_DestroyRenderer(mRenderer);
        mRenderer = nullptr;
    }
}

bool MacSdlRenderer::Initialise(int backBufferWidth, int backBufferHeight)
{
    if (mWindow == nullptr)
    {
        return false;
    }

    mBackBufferWidth = backBufferWidth;
    mBackBufferHeight = backBufferHeight;

    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (mRenderer == nullptr)
    {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        return false;
    }

    SDL_RenderSetLogicalSize(mRenderer, mBackBufferWidth, mBackBufferHeight);
    return true;
}

void MacSdlRenderer::BeginFrame(const Color& clearColor)
{
    if (mRenderer == nullptr)
    {
        return;
    }

    SDL_SetRenderDrawColor(
        mRenderer,
        ToByte(clearColor.R),
        ToByte(clearColor.G),
        ToByte(clearColor.B),
        ToByte(clearColor.A));
    SDL_RenderClear(mRenderer);
}

void MacSdlRenderer::EndFrame()
{
    if (mRenderer != nullptr)
    {
        SDL_RenderPresent(mRenderer);
    }
}

TextureHandle MacSdlRenderer::LoadTexture(const std::string& assetPath)
{
    std::cerr << "MacSdlRenderer texture loading is not implemented yet: " << assetPath << "\n";
    return 0;
}

void MacSdlRenderer::DrawDebugText(const char* text, float top, float left)
{
    if (text == nullptr)
    {
        return;
    }

    std::cout << "debug text [" << left << "," << top << "]: " << text << "\n";
}

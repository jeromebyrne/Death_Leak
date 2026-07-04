#ifndef DEATHLEAK_BACKENDS_MAC_MACSDLRENDERER_H
#define DEATHLEAK_BACKENDS_MAC_MACSDLRENDERER_H

#include "Engine/Render/IRenderer.h"

#include <SDL.h>

class MacSdlRenderer final : public IRenderer
{
public:
    explicit MacSdlRenderer(SDL_Window* window);
    ~MacSdlRenderer() override;

    bool Initialise(int backBufferWidth, int backBufferHeight) override;
    void BeginFrame(const Color& clearColor) override;
    void EndFrame() override;
    TextureHandle LoadTexture(const std::string& assetPath) override;
    void DrawDebugText(const char* text, float top, float left) override;

private:
    SDL_Window* mWindow = nullptr;
    SDL_Renderer* mRenderer = nullptr;
    int mBackBufferWidth = 0;
    int mBackBufferHeight = 0;
};

#endif

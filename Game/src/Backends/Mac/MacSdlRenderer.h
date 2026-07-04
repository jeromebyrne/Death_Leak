#ifndef DEATHLEAK_BACKENDS_MAC_MACSDLRENDERER_H
#define DEATHLEAK_BACKENDS_MAC_MACSDLRENDERER_H

#include "Engine/Render/IRenderer.h"

#include <SDL.h>

#include <string>
#include <unordered_map>

class MacSdlRenderer final : public IRenderer
{
public:
    explicit MacSdlRenderer(SDL_Window* window);
    ~MacSdlRenderer() override;

    bool Initialise(int backBufferWidth, int backBufferHeight) override;
    void BeginFrame(const Color& clearColor) override;
    void EndFrame() override;
    TextureHandle LoadTexture(const std::string& assetPath) override;
    void DrawSprite(const SpriteDrawCommand& command) override;
    void DrawRect(const RectDrawCommand& command) override;
    void DrawDebugText(const char* text, float top, float left) override;

private:
    struct TextureResource
    {
        SDL_Texture* Texture = nullptr;
        int Width = 0;
        int Height = 0;
    };

    SDL_Window* mWindow = nullptr;
    SDL_Renderer* mRenderer = nullptr;
    int mBackBufferWidth = 0;
    int mBackBufferHeight = 0;
    TextureHandle mNextTextureHandle = 1;
    std::unordered_map<TextureHandle, TextureResource> mTextures;
    std::unordered_map<std::string, TextureHandle> mTextureCache;
};

#endif

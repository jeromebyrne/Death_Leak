#ifndef DEATHLEAK_ENGINE_RENDER_IRENDERER_H
#define DEATHLEAK_ENGINE_RENDER_IRENDERER_H

#include <string>

struct Color
{
    float R = 0.0f;
    float G = 0.0f;
    float B = 0.0f;
    float A = 1.0f;
};

using TextureHandle = unsigned int;
using ShaderHandle = unsigned int;
using RenderTargetHandle = unsigned int;

class IRenderer
{
public:
    virtual ~IRenderer() = default;

    virtual bool Initialise(int backBufferWidth, int backBufferHeight) = 0;
    virtual void BeginFrame(const Color& clearColor) = 0;
    virtual void EndFrame() = 0;
    virtual TextureHandle LoadTexture(const std::string& assetPath) = 0;
    virtual void DrawDebugText(const char* text, float top, float left) = 0;
};

#endif

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

enum class SpriteFlip
{
    None,
    Horizontal,
    Vertical,
    Both
};

struct SpriteDrawCommand
{
    TextureHandle Texture = 0;
    float X = 0.0f;
    float Y = 0.0f;
    float Width = 0.0f;
    float Height = 0.0f;
    float RotationRadians = 0.0f;
    float Alpha = 1.0f;
    float Depth = 0.0f;
    SpriteFlip Flip = SpriteFlip::None;
};

struct RectDrawCommand
{
    float X = 0.0f;
    float Y = 0.0f;
    float Width = 0.0f;
    float Height = 0.0f;
    float RotationRadians = 0.0f;
    Color Fill{1.0f, 0.0f, 1.0f, 0.35f};
    Color Outline{1.0f, 1.0f, 1.0f, 0.85f};
};

class IRenderer
{
public:
    virtual ~IRenderer() = default;

    virtual bool Initialise(int backBufferWidth, int backBufferHeight) = 0;
    virtual void BeginFrame(const Color& clearColor) = 0;
    virtual void EndFrame() = 0;
    virtual TextureHandle LoadTexture(const std::string& assetPath) = 0;
    virtual void DrawSprite(const SpriteDrawCommand& command) = 0;
    virtual void DrawRect(const RectDrawCommand& command) = 0;
    virtual void DrawDebugText(const char* text, float top, float left) = 0;
};

#endif

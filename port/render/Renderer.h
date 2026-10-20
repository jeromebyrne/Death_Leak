#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace deathleak::render {

using TextureId = std::uint32_t;
constexpr TextureId InvalidTexture = 0;

struct Rect {
    float x = 0;
    float y = 0;
    float width = 0;
    float height = 0;
};

struct Color {
    std::uint8_t r = 0, g = 0, b = 0, a = 255;
};

struct Sprite {
    TextureId texture = InvalidTexture;
    Rect source;       // Zero width/height means the complete texture.
    Rect destination;  // Logical 1280x720 coordinates.
    Color tint{255, 255, 255, 255};
    float rotationDegrees = 0;
    bool flipX = false;
    bool flipY = false;
};

// No SDL, Direct3D, Metal, or native-window types cross this boundary.
// A future Metal/SDL-macOS backend can replace the factory implementation.
class Renderer {
public:
    virtual ~Renderer() = default;
    virtual bool Initialize(const char* title, int width, int height, std::string& error) = 0;
    virtual TextureId LoadTexture(const std::string& path, std::string& error) = 0;
    virtual void BeginFrame(Color clear) = 0;
    virtual void DrawSprite(const Sprite& sprite) = 0;
    virtual void FillRect(Rect rect, Color color) = 0;
    virtual void EndFrame() = 0;
    virtual bool PollQuit() = 0;
    virtual bool KeyDown(int key) const = 0;
    virtual bool SaveFrame(const std::string& path, std::string& error) = 0;
};

// Key codes are intentionally backend-neutral.
enum Key { Left = 1, Right, Up, Down, Escape, Restart, Interact, Melee, Purchase };
std::unique_ptr<Renderer> CreateRenderer();

} // namespace deathleak::render

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

struct MouseState {
    float x = -1, y = -1; // Logical 1280x720 coordinates.
    bool leftDown = false;
};

struct Color {
    std::uint8_t r = 0, g = 0, b = 0, a = 255;
};

struct SwayIntensity { float x = 0, y = 0; };
struct FoliageSway {
    bool enabled = false;
    SwayIntensity bottomLeft, bottomRight, topRight, topLeft;
    float worldX = 0, worldY = 0, time = 0, timeMultiplier = 1;
};

struct Sprite {
    TextureId texture = InvalidTexture;
    Rect source;       // Zero width/height means the complete texture.
    Rect destination;  // Logical 1280x720 coordinates.
    Color tint{255, 255, 255, 255};
    float rotationDegrees = 0;
    bool flipX = false;
    bool flipY = false;
    bool repeatX = false; // Original Sprite repeatX: one source-width per world source-width.
    float uvShiftX = 0, uvShiftY = 0; // Normalized wrap offset for authored parallax layers.
    FoliageSway foliageSway{}; // Authored FoliageSway.fx vertex intensities.
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
    // Apply the original Sepia.fx to the scene before UI is drawn.
    virtual void ApplyScenePostprocess() = 0;
    virtual void EndFrame() = 0;
    virtual bool PollQuit() = 0;
    virtual bool KeyDown(int key) const = 0;
    virtual MouseState Mouse() const = 0;
    virtual bool SaveFrame(const std::string& path, std::string& error) = 0;
};

// Key codes are intentionally backend-neutral.
enum Key { Left = 1, Right, Up, Down, Escape, Restart, Interact, Melee, Purchase, Roll, ModalBack };
// Software selection is an explicit headless-test escape hatch, never release acceptance.
std::unique_ptr<Renderer> CreateRenderer(bool testAllowSoftware = false);

} // namespace deathleak::render

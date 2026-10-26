#pragma once

#include <cstdint>
#include <cstddef>
#include <cmath>
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

// Camera2D's orthographic extents contract with zoom; project authored
// logical world destinations about the 1280x720 viewport center.
inline Rect ProjectWorldRect(Rect rect, float zoom) {
    if (!std::isfinite(zoom) || zoom <= 0.f || zoom == 1.f) return rect;
    return {640.f + (rect.x - 640.f) / zoom,
            360.f + (rect.y - 360.f) / zoom,
            rect.width / zoom, rect.height / zoom};
}

struct MouseState {
    float x = -1, y = -1; // Logical 1280x720 coordinates.
    bool leftDown = false;
};

struct ControllerSticks {
    int leftX = 0, rightX = 0, rightY = 0;
    bool connected = false;
};

struct Color {
    std::uint8_t r = 0, g = 0, b = 0, a = 255;
};

struct SwayIntensity { float x = 0, y = 0; };
struct FoliageSway {
    bool enabled = false;
    SwayIntensity bottomLeft, bottomRight, topRight, topLeft;
    float worldX = 0, worldY = 0, time = 0, timeMultiplier = 1;
    float globalTimeMultiplier = 1; // WeatherManager's phase factor, not sway amplitude.
};

struct PixelWobble {
    bool enabled = false;
    float intensity = 0, speedMod = 0, time = 0;
};

struct Sprite {
    TextureId texture = InvalidTexture;
    Rect source;       // Zero width/height means the complete texture.
    Rect destination;  // Logical 1280x720 coordinates.
    Color tint{255, 255, 255, 255};
    float opacity = 1.f; // Float-of-record for custom per-sprite shader alpha.
    float rotationDegrees = 0;
    bool flipX = false;
    bool flipY = false;
    bool repeatX = false; // Original Sprite repeatX: one source-width per world source-width.
    float uvShiftX = 0, uvShiftY = 0; // Normalized wrap offset for authored parallax layers.
    FoliageSway foliageSway{}; // Authored FoliageSway.fx vertex intensities.
    PixelWobble pixelWobble{}; // Per-sprite PixelWobble.fx sampling state.
    float rgbGain = 1.f; // Actor damage burst: RGB only; never increases alpha.
};

// No SDL, Direct3D, Metal, or native-window types cross this boundary.
// A future Metal/SDL-macOS backend can replace the factory implementation.
class Renderer {
public:
    virtual ~Renderer() = default;
    virtual void SetRequestedRefreshHz(int) {} // 0 chooses the closest advertised current rate.
    virtual bool Initialize(const char* title, int width, int height, bool fullscreen, std::string& error) = 0;
    virtual TextureId LoadTexture(const std::string& path, std::string& error) = 0;
    virtual void BeginFrame(Color clear) = 0;
    // Scene-only projection; BeginFrame and ApplyScenePostprocess reset it.
    virtual void SetWorldZoom(float zoom) { (void)zoom; }
    virtual void DrawSprite(const Sprite& sprite) = 0;
    virtual void DrawSpriteBatch(const Sprite* sprites, std::size_t count) {
        for (std::size_t i = 0; i < count; ++i) DrawSprite(sprites[i]);
    }
    virtual void FillRect(Rect rect, Color color) = 0;
    virtual void FillRects(const Rect* rects, std::size_t count, Color color) {
        for (std::size_t i = 0; i < count; ++i) FillRect(rects[i], color);
    }
    // Apply the original Sepia.fx to the scene before UI is drawn.
    virtual void ApplyScenePostprocess() = 0;
    virtual void ResolveUnprocessedFrame() {}
    virtual void EndFrame() = 0;
    // Display-only replay requires a stable physical presentation surface.
    virtual bool PresentationSurfaceStable() const { return false; }
    virtual bool PollQuit() = 0;
    virtual bool KeyDown(int key) const = 0;
    virtual ControllerSticks ControllerAxes() const { return {}; }
    virtual MouseState Mouse() const = 0;
    virtual bool SaveFrame(const std::string& path, std::string& error) = 0;
};

// Key codes are intentionally backend-neutral.
enum Key { Left = 1, Right, Up, Down, Escape, Restart, Interact, Melee, Purchase, Roll, ModalBack, Fire };

// Linux keyboard adaptation: crouch and fire are independent grounded actions.
// Down remains the legacy airborne shot; scripted commands and stick aim keep
// their existing paths. Kept backend-neutral for a no-display input fixture.
struct GameplayAttackInput { bool crouch; bool fire; };
inline GameplayAttackInput ResolveGameplayAttackInput(
    bool scripted, char command, const Renderer& renderer, bool grounded,
    bool atWall, bool rolling, bool meleeActive, bool stickAim) {
    const bool down = scripted ? command == 'C' : renderer.KeyDown(Down);
    const bool crouch = down && grounded && !atWall && !rolling && !meleeActive;
    const bool scriptedFire = command == 'F' || command == 'B' || command == 'N' ||
                              command == 'Y' || command == 'Z';
    const bool fire = (scripted ? scriptedFire :
        (renderer.KeyDown(Fire) || (down && !crouch))) || stickAim;
    return {crouch, fire};
}
// Software selection is an explicit headless-test escape hatch, never release acceptance.
std::unique_ptr<Renderer> CreateRenderer(bool testAllowSoftware = false);

} // namespace deathleak::render

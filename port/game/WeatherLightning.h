#pragma once
#include <algorithm>
#include <cstdint>
#include <cmath>
#include "../render/Renderer.h"

namespace deathleak::game {
// Source WeatherManager's rain/flash timing only. No rain particles or audio.
// Random draws use the original centisecond ranges; the independent seed makes
// port validation repeatable, but is not Windows rand() stream equivalence.
class WeatherLightning {
public:
    explicit WeatherLightning(std::uint32_t seed = 1) : randomState_(seed) {}
    void SetAllowed(bool allowed) { allowed_ = allowed; }
    bool Allowed() const { return allowed_; }
    bool Raining() const { return raining_; }
    float Alpha() const { return raining_ ? alpha_ : 0.f; }
    float Elapsed() const { return elapsed_; }
    render::Sprite SpriteForCamera(render::TextureId texture, float cameraX, float cameraY) const {
        // Source ParallaxLayer sets world XY to camera XY on update. Both cancel
        // in the port's world-to-screen transform; retain arguments as evidence.
        const float scale = 2.f / 3.f;
        const float size = 2048.f * scale;
        const float worldX = cameraX, worldY = cameraY;
        render::Sprite sprite{texture, {},
            {640.f + (worldX-cameraX-1024.f)*scale,
             360.f + (cameraY-worldY-1024.f)*scale, size, size}};
        sprite.tint.a = static_cast<std::uint8_t>(std::lround(
            std::clamp(Alpha(), 0.f, 1.f) * 255.f));
        sprite.flipY = true;
        return sprite;
    }
    void Update(float dt) {
        if (!allowed_) return; // Source pauses its clock with weather disabled.
        if (raining_) {
            if (elapsed_ > rainStart_ + rainDuration_) {
                if (elapsed_ > rainStart_ + rainDuration_ + 3.f) {
                    raining_ = false;
                    alpha_ = 0.f;
                    playingFlash_ = false;
                    nextWeather_ = elapsed_ + 160.f + Centiseconds(14000);
                }
                // Source does not advance flash alpha during rain outro.
            } else if (playingFlash_) {
                if (elapsed_ < flashStart_ + .4f)
                    alpha_ = std::clamp(1.f - (elapsed_ - flashStart_) / .4f, 0.f, 1.f);
                else { alpha_ = 0.f; playingFlash_ = false; }
            } else if (elapsed_ > nextFlash_) {
                flashStart_ = elapsed_;
                playingFlash_ = true;
                nextFlash_ = elapsed_ + 10.f + Centiseconds(2000);
                // The original sets alpha on the *next* rain update.
            }
        } else if ((!hadWeather_ && elapsed_ > 180.f) ||
                   (hadWeather_ && elapsed_ > nextWeather_)) {
            raining_ = hadWeather_ = true;
            rainStart_ = elapsed_;
            rainDuration_ = 70.f + Centiseconds(5000);
            alpha_ = 0.f;
            nextFlash_ = 0.f; // first rain update flashes immediately
        }
        elapsed_ += dt; // WeatherManager advances time after state updates.
    }
private:
    float Centiseconds(std::uint32_t range) {
        randomState_ = randomState_ * 1664525u + 1013904223u;
        return static_cast<float>(randomState_ % range) * .01f;
    }
    std::uint32_t randomState_;
    bool allowed_ = false, hadWeather_ = false, raining_ = false, playingFlash_ = false;
    float elapsed_ = 0.f, rainStart_ = 0.f, rainDuration_ = 0.f;
    float nextWeather_ = 0.f, flashStart_ = 0.f, nextFlash_ = 0.f, alpha_ = 0.f;
};
}

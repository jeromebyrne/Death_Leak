#pragma once

#include "ParticleSpray.h"

namespace deathleak::game {
// WeatherManager's camera-attached directed spray. Unlike XML sprays, its
// particles begin at the source's off-screen placeholder and only enter the
// view when their first lifetime expires. A private RNG is reproducible, not
// equivalent to the original global Windows rand() stream.
class WeatherRain {
public:
    bool Active() const { return active_; }
    bool VisibleParticles() const { return active_ || draining_; }
    const ParticleSpray& Spray() const { return spray_; }
    void SetTexture(render::TextureId texture) { spray_.texture = texture; }
    void Update(float dt, bool raining, float cameraX, float cameraY) {
        if (raining && !active_) Start();
        if (!raining && active_) { active_ = false; draining_ = true; spray_.looping = false; }
        if (!VisibleParticles() || dt <= 0) return;
        // Source UpdateParticleToParent uses the current camera on each expiry,
        // with the creation-time signed spawn offsets retained per particle.
        for (size_t i = 0; i < spray_.particles.size(); ++i) {
            auto& p = spray_.particles[i];
            p.startX = cameraX + offsetsX_[i];
            p.startY = cameraY + 700.f + offsetsY_[i];
        }
        spray_.Update(dt);
        if (draining_) {
            bool any = false;
            for (const auto& p : spray_.particles) any |= !p.dead;
            if (!any) { draining_ = false; spray_.particles.clear(); }
        }
    }
    void Draw(render::Renderer& renderer, float cameraX, float cameraY) const {
        if (VisibleParticles() && spray_.texture) spray_.Draw(renderer, cameraX, cameraY);
    }
private:
    void Start() {
        active_ = true; draining_ = false;
        spray_.particles.clear(); offsetsX_.clear(); offsetsY_.clear();
        spray_.warmed = true; // WeatherManager CreateRainAssets(false) never calls Warm.
        spray_.looping = true; spray_.elapsed = 0; spray_.loopTime = -1;
        spray_.fadeIn = .15f; spray_.fadeOut = .8f;
        spray_.scalesByLifetime = true; spray_.scaleTo = .9f;
        for (int i = 0; i < 300; ++i) {
            ParticleSpray::Particle p;
            p.directionX = -.2f; p.directionY = -.8f;
            p.lifetime = .85f - ((.85f - .7f) / 300.f) * i;
            auto offset = [&](unsigned spread) {
                float value = static_cast<float>(spray_.Random(spread * 10 + 1));
                return spray_.Random(2) ? -value : value;
            };
            offsetsX_.push_back(offset(140)); offsetsY_.push_back(offset(40));
            p.x = p.drawX = 99999.f + offsetsX_.back();
            p.y = p.drawY = 9999.f + offsetsY_.back();
            p.size = p.startSize = 29.f + static_cast<float>(spray_.Random(12));
            p.speed = p.startSpeed = 26.01f + .01f * spray_.Random(600);
            p.gravity = -1.f;
            p.brightness = .9f + .01f * static_cast<float>(spray_.Random(10) + 1);
            spray_.particles.push_back(p);
        }
    }
    ParticleSpray spray_;
    std::vector<float> offsetsX_, offsetsY_;
    bool active_ = false, draining_ = false;
};
} // namespace deathleak::game

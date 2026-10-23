#pragma once

#include "../render/Renderer.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace deathleak::game {

// Level-authored ParticleSpray, in world units. The large XML dimensions are
// an update/draw culling region, never a textured emitter rectangle.
struct ParticleSpray {
    struct Particle {
        float x = 0, y = 0, startX = 0, startY = 0;
        float drawX = 0, drawY = 0, drawSize = 0;
        float directionX = 0, directionY = 0;
        float speed = 0, startSpeed = 0, size = 0, startSize = 0;
        float lifetime = 1, elapsed = 0, gravity = 0, brightness = 1, alpha = 1;
        bool dead = false;
    };
    render::TextureId texture = render::InvalidTexture;
    std::string xmlId;
    float x = 0, y = 0, width = 0, height = 0;
    int depth = 0;
    size_t order = 0;
    bool updateable = true, alwaysUpdate = false, looping = true, warmed = false;
    float fadeIn = .1f, fadeOut = .6f, scaleTo = 1, loopTime = 0, elapsed = 0;
    bool scalesByLifetime = false;
    std::vector<Particle> particles;

    // A private stream keeps captures reproducible; the Windows global rand()
    // stream also includes earlier level objects and cannot be aligned here.
    std::uint32_t randomState = 0x4c454146u;
    unsigned Random(unsigned limit) {
        randomState = randomState * 1103515245u + 12345u;
        return limit ? (randomState >> 16) % limit : 0;
    }
    void Update(float dt, const float* parentXY = nullptr) {
        if (!updateable || dt <= 0) return;
        elapsed += dt;
        for (auto& p : particles) {
            if (p.dead) continue;
            // Source Update builds vertices before advancing particle position.
            p.drawX = p.x; p.drawY = p.y; p.drawSize = p.size;
            if (p.gravity > 0) p.speed *= .99f;
            const float ticks = dt * 60.f; // source target delta is 1/60 s
            p.x += p.directionX * p.speed * ticks;
            p.y += (p.directionY * p.speed - p.gravity) * ticks;
            p.elapsed += dt;
            const float left = p.lifetime - p.elapsed;
            const float percentLeft = left / p.lifetime;
            p.alpha = 1;
            if (fadeOut > 0 && percentLeft < 1 - fadeOut)
                p.alpha = percentLeft / (1 - fadeOut);
            else if (fadeIn > 0 && 1 - percentLeft < fadeIn)
                p.alpha = (1 - percentLeft) / fadeIn;
            if (scalesByLifetime)
                p.size = p.startSize * (1 + (scaleTo - 1) * p.elapsed / p.lifetime);
            if (left <= 0) {
                if (looping) {
                    p.x = parentXY ? parentXY[0] : p.startX;
                    p.y = parentXY ? parentXY[1] : p.startY;
                    if (parentXY) { p.startX = p.x; p.startY = p.y; }
                    p.speed = p.startSpeed; p.size = p.startSize; p.elapsed = 0;
                } else p.dead = true;
            }
        }
        if (looping && loopTime > 0 && elapsed > loopTime) looping = false;
        // GameObjectManager requests deferred warm-up on load. Source Warm runs
        // after the first Update and randomizes age and position independently.
        if (!warmed) {
            for (auto& p : particles) {
                const float age = Random(static_cast<unsigned>(p.lifetime * 1000)) / 1000.f;
                p.x = p.startX + p.directionX * p.speed * age;
                p.y = p.startY + (p.directionY * p.speed + p.gravity) * age;
                p.elapsed += age;
            }
            warmed = true;
        }
    }
    void Draw(render::Renderer& renderer, float cameraX, float cameraY) const {
        constexpr float scale = 2.f / 3.f;
        for (const auto& p : particles) {
            if (p.dead) continue;
            render::Sprite leaf;
            leaf.texture = texture;
            // Source ParticleSpray binds U=1 at world-left and U=0 at world-right.
            leaf.flipX = true;
            const float drawSize = p.drawSize > 0 ? p.drawSize : p.size;
            leaf.destination = {640.f + (p.drawX-cameraX-drawSize*.5f)*scale,
                                360.f + (cameraY-p.drawY-drawSize*.5f)*scale,
                                drawSize*scale, drawSize*scale};
            if (leaf.destination.x + leaf.destination.width < 0 || leaf.destination.x > 1280 ||
                leaf.destination.y + leaf.destination.height < 0 || leaf.destination.y > 720) continue;
            const auto brightness = static_cast<std::uint8_t>(std::lround(std::clamp(p.brightness, 0.f, 1.f)*255));
            // Particle.fx multiplies sampled RGBA by brightness, then alpha by fade.
            leaf.tint = {brightness, brightness, brightness,
                         static_cast<std::uint8_t>(std::lround(std::clamp(p.alpha * p.brightness, 0.f, 1.f)*255))};
            renderer.DrawSprite(leaf);
        }
    }
};
}

#pragma once

#include "../render/Renderer.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace deathleak::game {

// A fixed-update snapshot shared by presentation and the atomic capture record.
// No simulation state or capture result is read while submitting this overlay.
struct LoadingOverlayPacket {
    bool visible = false;
    float alpha = -1.f;
    int tintAlpha = -1;

    static LoadingOverlayPacket Prepare(bool visible, bool holdExited,
                                        bool fadeIn, bool fadeOut,
                                        float phaseTime, float fadeSeconds) {
        LoadingOverlayPacket packet;
        if (!visible) return packet;
        packet.visible = true;
        packet.alpha = holdExited ? 1.f : fadeIn
            ? std::min(1.f, phaseTime / fadeSeconds)
            : fadeOut ? std::max(0.f, 1.f - phaseTime / fadeSeconds) : 1.f;
        packet.tintAlpha = static_cast<int>(std::lround(packet.alpha * 255.f));
        return packet;
    }

    void Draw(render::Renderer& renderer, render::TextureId art) const {
        if (!visible) return;
        const render::Color tint{255, 255, 255, static_cast<std::uint8_t>(tintAlpha)};
        if (art) renderer.DrawSprite({art, {}, {0, 0, 1280, 720}, tint});
        else renderer.FillRect({0, 0, 1280, 720}, {0, 0, 0, tint.a});
    }
};
} // namespace deathleak::game

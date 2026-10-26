#pragma once
#include "PresentationCommandPacket.h"
#include <cmath>
#include <set>

namespace deathleak::linux_port {
// Conservative whole-packet oracle. No draw may escape the identity ledger;
// painter boundaries remain in their original positions. This is an admission
// gate, not a renderer: an admitted sample still needs a verified runtime path.
enum class FullFrameDecision { Interpolate, HoldCurrent, Invalid };
struct FullFrameSnapshot {
    PresentationCommandPacket packet;
    std::uint64_t sceneEpoch = 0;
    float cameraX = 0, cameraY = 0; // screen-space camera, same convention as the scene oracle
    bool discontinuity = false;
};
inline bool SameColor(render::Color a, render::Color b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}
inline bool SameRect(render::Rect a, render::Rect b) {
    return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
}
inline bool SameSpriteExceptPosition(const render::Sprite& a, const render::Sprite& b) {
    return a.texture == b.texture && SameRect(a.source,b.source) &&
        a.destination.width == b.destination.width && a.destination.height == b.destination.height &&
        SameColor(a.tint,b.tint) && a.opacity == b.opacity &&
        a.rotationDegrees == b.rotationDegrees && a.flipX == b.flipX && a.flipY == b.flipY &&
        a.repeatX == b.repeatX && a.uvShiftX == b.uvShiftX && a.uvShiftY == b.uvShiftY &&
        a.rgbGain == b.rgbGain && a.foliageSway.enabled == b.foliageSway.enabled &&
        a.pixelWobble.enabled == b.pixelWobble.enabled &&
        (!a.foliageSway.enabled && !a.pixelWobble.enabled);
}
inline FullFrameDecision AdmitFullFrame(const FullFrameSnapshot& a, const FullFrameSnapshot& b, float alpha) {
    if (!a.packet.Sealed() || !b.packet.Sealed() || !std::isfinite(alpha) || alpha < 0 || alpha > 1 ||
        !std::isfinite(a.cameraX) || !std::isfinite(a.cameraY) ||
        !std::isfinite(b.cameraX) || !std::isfinite(b.cameraY)) return FullFrameDecision::Invalid;
    if (a.sceneEpoch != b.sceneEpoch || a.discontinuity || b.discontinuity ||
        std::fabs(b.cameraX-a.cameraX) > 250.f || std::fabs(b.cameraY-a.cameraY) > 250.f)
        return FullFrameDecision::HoldCurrent;
    if (a.packet.Size() != b.packet.Size()) return FullFrameDecision::HoldCurrent;
    std::set<TopologyKey> keys;
    const auto& ac = a.packet.Commands(); const auto& bc = b.packet.Commands();
    for (std::size_t i=0; i<ac.size(); ++i) {
        const auto& x=ac[i]; const auto& y=bc[i];
        if (x.kind != y.kind) return FullFrameDecision::HoldCurrent;
        switch (x.kind) {
        case PresentationCommandPacket::Kind::Begin:
        case PresentationCommandPacket::Kind::Fill:
            if (!SameColor(x.color,y.color) || (x.kind == PresentationCommandPacket::Kind::Fill && !SameRect(x.rect,y.rect)))
                return FullFrameDecision::HoldCurrent;
            break;
        case PresentationCommandPacket::Kind::Zoom:
            if (x.zoom != y.zoom) return FullFrameDecision::HoldCurrent;
            break;
        case PresentationCommandPacket::Kind::Sprite: {
            // Any untagged actor/effect/dynamic draw prevents partial motion.
            if (!x.key.object || !(x.key == y.key) || !keys.insert(x.key).second ||
                x.cameraFactorX != y.cameraFactorX || x.cameraFactorY != y.cameraFactorY ||
                !std::isfinite(x.cameraFactorX) || !std::isfinite(x.cameraFactorY) ||
                !SameSpriteExceptPosition(x.sprite,y.sprite)) return FullFrameDecision::HoldCurrent;
            const float worldAx=x.sprite.destination.x+a.cameraX*x.cameraFactorX;
            const float worldBx=y.sprite.destination.x+b.cameraX*x.cameraFactorX;
            const float worldAy=x.sprite.destination.y+a.cameraY*x.cameraFactorY;
            const float worldBy=y.sprite.destination.y+b.cameraY*x.cameraFactorY;
            if (std::fabs(worldBx-worldAx)>250.f || std::fabs(worldBy-worldAy)>250.f)
                return FullFrameDecision::HoldCurrent;
            break;
        }
        case PresentationCommandPacket::Kind::SpriteBatch:
        case PresentationCommandPacket::Kind::FillBatch:
            // No per-element semantic identity yet; preserve painter boundary.
            return FullFrameDecision::HoldCurrent;
        case PresentationCommandPacket::Kind::Postprocess:
        case PresentationCommandPacket::Kind::Resolve:
            break;
        }
    }
    return FullFrameDecision::Interpolate;
}
} // namespace deathleak::linux_port

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
    float shaderTime = 0, wobbleTime = 0;
};
inline bool SameColor(render::Color a, render::Color b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}
inline bool SameRect(render::Rect a, render::Rect b) {
    return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
}
inline bool SameSwayShape(const render::FoliageSway& a, const render::FoliageSway& b) {
    return a.enabled == b.enabled && a.bottomLeft.x == b.bottomLeft.x && a.bottomLeft.y == b.bottomLeft.y &&
        a.bottomRight.x == b.bottomRight.x && a.bottomRight.y == b.bottomRight.y &&
        a.topRight.x == b.topRight.x && a.topRight.y == b.topRight.y &&
        a.topLeft.x == b.topLeft.x && a.topLeft.y == b.topLeft.y &&
        a.timeMultiplier == b.timeMultiplier && a.globalTimeMultiplier == b.globalTimeMultiplier;
}
inline bool SameSpriteExceptPosition(const render::Sprite& a, const render::Sprite& b) {
    return a.texture == b.texture && SameRect(a.source,b.source) &&
        a.destination.width == b.destination.width && a.destination.height == b.destination.height &&
        SameColor(a.tint,b.tint) && a.opacity == b.opacity &&
        a.rotationDegrees == b.rotationDegrees && a.flipX == b.flipX && a.flipY == b.flipY &&
        a.repeatX == b.repeatX &&
        a.rgbGain == b.rgbGain && SameSwayShape(a.foliageSway,b.foliageSway) &&
        a.pixelWobble.enabled == b.pixelWobble.enabled &&
        a.pixelWobble.intensity == b.pixelWobble.intensity &&
        a.pixelWobble.speedMod == b.pixelWobble.speedMod;
}
inline bool NearPhase(float actual, float expected) {
    return std::isfinite(actual) && std::isfinite(expected) && std::fabs(actual-expected) <= 0.00002f;
}
inline bool PhaseAdmitted(const PresentationCommandPacket::Command& x,
                          const PresentationCommandPacket::Command& y,
                          const FullFrameSnapshot& a, const FullFrameSnapshot& b) {
    using Phase = PresentationCommandPacket::PhasePolicy;
    if (x.phase != y.phase || x.parallaxX != y.parallaxX || x.parallaxY != y.parallaxY ||
        !std::isfinite(x.sprite.uvShiftX) || !std::isfinite(y.sprite.uvShiftX) ||
        !std::isfinite(x.sprite.uvShiftY) || !std::isfinite(y.sprite.uvShiftY)) return false;
    const auto& u=x.sprite; const auto& v=y.sprite;
    if (x.phase == Phase::CameraParallax) {
        // Snapshot camera uses projected coordinates; authored parallax UV uses raw camera.
        const float rawDx=(b.cameraX-a.cameraX)*1.5f;
        const float rawDy=(b.cameraY-a.cameraY)*-1.5f;
        return NearPhase(v.uvShiftX-u.uvShiftX,rawDx*x.parallaxX) &&
               NearPhase(v.uvShiftY-u.uvShiftY,rawDy*x.parallaxY) &&
               !u.foliageSway.enabled && !u.pixelWobble.enabled;
    }
    if (u.uvShiftX != v.uvShiftX || u.uvShiftY != v.uvShiftY) return false;
    if (x.phase == Phase::AutoScrollY) return false; // needs an explicit once-per-update phase clock
    if (x.phase == Phase::ShaderClock) {
        return (!u.foliageSway.enabled ||
                (NearPhase(u.foliageSway.time,a.shaderTime) && NearPhase(v.foliageSway.time,b.shaderTime))) &&
            (!u.pixelWobble.enabled ||
                (NearPhase(u.pixelWobble.time,a.wobbleTime) && NearPhase(v.pixelWobble.time,b.wobbleTime))) &&
            std::isfinite(a.shaderTime) && std::isfinite(b.shaderTime) &&
            std::isfinite(a.wobbleTime) && std::isfinite(b.wobbleTime) &&
            std::isfinite(u.foliageSway.worldX) && std::isfinite(v.foliageSway.worldX) &&
            std::isfinite(u.foliageSway.worldY) && std::isfinite(v.foliageSway.worldY);
    }
    return !u.foliageSway.enabled && !u.pixelWobble.enabled;
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
                !SameSpriteExceptPosition(x.sprite,y.sprite) || !PhaseAdmitted(x,y,a,b)) return FullFrameDecision::HoldCurrent;
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
// Copy-on-sample leaves both sealed simulation packets immutable. alpha=1 returns
// the exact current packet, including floats, commands, and painter ordering.
inline PresentationCommandPacket SampleFullFrame(const FullFrameSnapshot& a,
                                                  const FullFrameSnapshot& b, float alpha) {
    if (alpha == 1.f || AdmitFullFrame(a,b,alpha) != FullFrameDecision::Interpolate) return b.packet;
    PresentationCommandPacket out;
    auto lerp=[alpha](float x,float y) { return x+(y-x)*alpha; };
    const auto& ac=a.packet.Commands(); const auto& bc=b.packet.Commands();
    for (std::size_t i=0;i<bc.size();++i) {
        auto c=bc[i];
        if (c.kind == PresentationCommandPacket::Kind::Sprite) {
            const auto& p=ac[i].sprite;
            c.sprite.destination.x=lerp(p.destination.x,c.sprite.destination.x);
            c.sprite.destination.y=lerp(p.destination.y,c.sprite.destination.y);
            c.sprite.uvShiftX=lerp(p.uvShiftX,c.sprite.uvShiftX);
            c.sprite.uvShiftY=lerp(p.uvShiftY,c.sprite.uvShiftY);
            if (c.phase == PresentationCommandPacket::PhasePolicy::ShaderClock) {
                c.sprite.foliageSway.time=lerp(p.foliageSway.time,c.sprite.foliageSway.time);
                c.sprite.pixelWobble.time=lerp(p.pixelWobble.time,c.sprite.pixelWobble.time);
                c.sprite.foliageSway.worldX=lerp(p.foliageSway.worldX,c.sprite.foliageSway.worldX);
                c.sprite.foliageSway.worldY=lerp(p.foliageSway.worldY,c.sprite.foliageSway.worldY);
            }
        }
        out.Add(std::move(c));
    }
    out.Seal();
    return out;
}
} // namespace deathleak::linux_port

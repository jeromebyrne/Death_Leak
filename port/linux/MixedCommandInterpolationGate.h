#pragma once
#include "FullFrameInterpolationGate.h"
#include <algorithm>
#include <vector>

namespace deathleak::linux_port {
// Preserve the current packet's painter stream. A command is sampled only at
// the same ordinal with unique stable identity and unchanged GPU draw state.
// Births, deaths, animation/film texture swaps and batch topology hold current.
struct MixedCommandDecision {
    bool valid = false;
    std::vector<bool> interpolate;
    std::size_t moving = 0;
};
inline MixedCommandDecision AdmitMixedCommands(const FullFrameSnapshot& a,
                                                const FullFrameSnapshot& b, float alpha) {
    MixedCommandDecision out;
    if (!a.packet.Sealed() || !b.packet.Sealed() || !std::isfinite(alpha) || alpha<0.f || alpha>1.f ||
        !std::isfinite(a.cameraX) || !std::isfinite(a.cameraY) ||
        !std::isfinite(b.cameraX) || !std::isfinite(b.cameraY)) return out;
    out.valid=true;
    out.interpolate.resize(b.packet.Size(),false);
    if (a.sceneEpoch!=b.sceneEpoch || a.discontinuity || b.discontinuity ||
        std::fabs(b.cameraX-a.cameraX)>250.f || std::fabs(b.cameraY-a.cameraY)>250.f) return out;
    const auto& ac=a.packet.Commands(); const auto& bc=b.packet.Commands();
    using Kind=PresentationCommandPacket::Kind;
    // Unowned sprites can be player, projectile, or attached effect draws.
    // Scenery sampled around their current-step silhouette is incoherent.
    // Conservatively hold the complete packet until those phases are owned.
    const auto hasUnownedSprite=[](const auto& commands) {
        return std::any_of(commands.begin(),commands.end(),[](const auto& c) {
            return (c.kind==Kind::Sprite || c.kind==Kind::SpriteBatch) && !c.key.object;
        });
    };
    if (hasUnownedSprite(ac) || hasUnownedSprite(bc)) return out;
    // Global render-pass state cannot be mixed. A changed pass topology or zoom
    // could make a previously sampled draw use the wrong target/shader/blend.
    for (std::size_t i=0;i<std::max(ac.size(),bc.size());++i) {
        const auto structural=[](Kind k) { return k==Kind::Begin || k==Kind::Zoom || k==Kind::Postprocess || k==Kind::Resolve; };
        if (i>=ac.size() || i>=bc.size()) {
            if ((i<ac.size() && structural(ac[i].kind)) || (i<bc.size() && structural(bc[i].kind))) return out;
            continue;
        }
        if (structural(ac[i].kind) || structural(bc[i].kind)) {
            if (ac[i].kind!=bc[i].kind ||
                (ac[i].kind==Kind::Begin && !SameColor(ac[i].color,bc[i].color)) ||
                (ac[i].kind==Kind::Zoom && ac[i].zoom!=bc[i].zoom)) return out;
        }
    }
    std::set<TopologyKey> seenA,seenB;
    for (const auto& c:ac) if (c.kind==Kind::Sprite && c.key.object) seenA.insert(c.key);
    for (const auto& c:bc) if (c.kind==Kind::Sprite && c.key.object) seenB.insert(c.key);
    for (std::size_t i=0;i<std::min(ac.size(),bc.size());++i) {
        const auto& x=ac[i]; const auto& y=bc[i];
        if (x.kind!=Kind::Sprite || y.kind!=Kind::Sprite || !x.key.object || !(x.key==y.key) ||
            !seenA.count(x.key) || !seenB.count(y.key) ||
            x.cameraFactorX!=y.cameraFactorX || x.cameraFactorY!=y.cameraFactorY ||
            !std::isfinite(x.cameraFactorX) || !std::isfinite(x.cameraFactorY) ||
            !SameSpriteExceptPosition(x.sprite,y.sprite) || !PhaseAdmitted(x,y,a,b)) continue;
        // Duplicate identities are ambiguous; never interpolate either copy.
        const auto countKey=[&](const auto& list) { return std::count_if(list.begin(),list.end(),[&](const auto& c) { return c.kind==Kind::Sprite && c.key==x.key; }); };
        if (countKey(ac)!=1 || countKey(bc)!=1) continue;
        const float ax=x.sprite.destination.x+a.cameraX*x.cameraFactorX;
        const float bx=y.sprite.destination.x+b.cameraX*y.cameraFactorX;
        const float ay=x.sprite.destination.y+a.cameraY*x.cameraFactorY;
        const float by=y.sprite.destination.y+b.cameraY*y.cameraFactorY;
        if (!std::isfinite(ax) || !std::isfinite(bx) || !std::isfinite(ay) || !std::isfinite(by) ||
            std::fabs(bx-ax)>250.f || std::fabs(by-ay)>250.f) continue;
        out.interpolate[i]=true; ++out.moving;
    }
    return out;
}
inline PresentationCommandPacket SampleMixedCommands(const FullFrameSnapshot& a,
                                                      const FullFrameSnapshot& b, float alpha) {
    if (alpha==1.f) return b.packet;
    const auto decision=AdmitMixedCommands(a,b,alpha);
    if (!decision.valid || !decision.moving) return b.packet;
    PresentationCommandPacket out;
    auto lerp=[alpha](float x,float y) { return x+(y-x)*alpha; };
    for (std::size_t i=0;i<b.packet.Size();++i) {
        auto c=b.packet.Commands()[i];
        if (decision.interpolate[i]) {
            const auto& p=a.packet.Commands()[i].sprite;
            c.sprite.destination.x=lerp(p.destination.x,c.sprite.destination.x);
            c.sprite.destination.y=lerp(p.destination.y,c.sprite.destination.y);
            c.sprite.uvShiftX=lerp(p.uvShiftX,c.sprite.uvShiftX);
            c.sprite.uvShiftY=lerp(p.uvShiftY,c.sprite.uvShiftY);
            if (c.phase==PresentationCommandPacket::PhasePolicy::ShaderClock) {
                c.sprite.foliageSway.time=lerp(p.foliageSway.time,c.sprite.foliageSway.time);
                c.sprite.pixelWobble.time=lerp(p.pixelWobble.time,c.sprite.pixelWobble.time);
                c.sprite.foliageSway.worldX=lerp(p.foliageSway.worldX,c.sprite.foliageSway.worldX);
                c.sprite.foliageSway.worldY=lerp(p.foliageSway.worldY,c.sprite.foliageSway.worldY);
            }
        }
        out.Add(std::move(c));
    }
    out.Seal(); return out;
}
} // namespace deathleak::linux_port

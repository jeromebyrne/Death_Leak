#pragma once
#include "FullFrameInterpolationGate.h"
#include <algorithm>
#include <vector>

namespace deathleak::linux_port {
// Preserve the current packet's painter stream. A command is sampled only at
// the same ordinal with unique stable identity and unchanged GPU draw state.
// Births, deaths, actor animation swaps and batch topology hold current.
// The four persistent HUD-film textures alone may remain on the prior frame
// until the next fixed boundary while sampled spatial phases advance.
inline bool IsPersistentHudFilm(const PresentationCommandPacket::Command& c) {
    return c.kind == PresentationCommandPacket::Kind::Sprite &&
           c.key.object == ((std::uint64_t{1} << 61) | 1) && c.key.generation == 0;
}
inline bool IsProjectileCommand(const PresentationCommandPacket::Command& c) {
    return c.kind == PresentationCommandPacket::Kind::Sprite &&
           (c.key.object >> 59) == 1 && (c.key.object & ((std::uint64_t{1} << 59)-1)) != 0;
}
inline bool IsParticleCommand(const PresentationCommandPacket::Command& c) {
    if (c.kind != PresentationCommandPacket::Kind::Sprite) return false;
    const auto object = c.key.object;
    // Scene spray and runtime material spray occupy disjoint tagged namespaces.
    return (object >> 62) == 1 || (object >> 60) == 1;
}
// ParticleSpray emits the previous update's position/size and this update's
// lifetime fade. Both size and alpha are continuous within a stable slot life;
// texture, flip, shader, blend and RGB remain discrete authored draw state.
inline bool ParticlePhaseAdmitted(const PresentationCommandPacket::Command& x,
                                  const PresentationCommandPacket::Command& y) {
    if (!IsParticleCommand(x) || !IsParticleCommand(y) || !(x.key==y.key)) return false;
    const auto& a=x.sprite; const auto& b=y.sprite;
    if (a.texture!=b.texture || !SameRect(a.source,b.source) ||
        a.flipX!=b.flipX || a.flipY!=b.flipY || a.repeatX!=b.repeatX ||
        a.tint.r!=b.tint.r || a.tint.g!=b.tint.g || a.tint.b!=b.tint.b ||
        a.opacity!=b.opacity || a.rgbGain!=b.rgbGain ||
        a.rotationDegrees!=b.rotationDegrees ||
        a.foliageSway.enabled || b.foliageSway.enabled ||
        a.pixelWobble.enabled || b.pixelWobble.enabled ||
        a.uvShiftX!=b.uvShiftX || a.uvShiftY!=b.uvShiftY ||
        !std::isfinite(a.destination.width) || !std::isfinite(b.destination.width) ||
        !std::isfinite(a.destination.height) || !std::isfinite(b.destination.height) ||
        a.destination.width<=0.f || b.destination.width<=0.f ||
        std::fabs(a.destination.width-a.destination.height)>0.01f ||
        std::fabs(b.destination.width-b.destination.height)>0.01f ||
        std::fabs(b.destination.width-a.destination.width)>32.f ||
        std::abs(int(b.tint.a)-int(a.tint.a))>64) return false;
    return true;
}
inline float ShortAngleDelta(float from, float to) {
    return std::remainder(to-from, 360.f);
}
// Projectile phase is attached to one birth ID. Texture/flip/blend/UV and
// shader state remain discrete; only motion, authored knife fade-in scale,
// opacity and a bounded continuous heading may be sampled.
inline bool ProjectilePhaseAdmitted(const PresentationCommandPacket::Command& x,
                                    const PresentationCommandPacket::Command& y) {
    if (!IsProjectileCommand(x) || !IsProjectileCommand(y) || !(x.key==y.key)) return false;
    const auto& a=x.sprite; const auto& b=y.sprite;
    if (a.texture!=b.texture || !SameRect(a.source,b.source) ||
        a.flipX!=b.flipX || a.flipY!=b.flipY || a.repeatX!=b.repeatX ||
        a.tint.r!=b.tint.r || a.tint.g!=b.tint.g || a.tint.b!=b.tint.b ||
        a.opacity!=b.opacity || a.rgbGain!=b.rgbGain ||
        !SameSwayShape(a.foliageSway,b.foliageSway) ||
        a.pixelWobble.enabled!=b.pixelWobble.enabled ||
        a.uvShiftX!=b.uvShiftX || a.uvShiftY!=b.uvShiftY ||
        !std::isfinite(a.rotationDegrees) || !std::isfinite(b.rotationDegrees) ||
        std::fabs(ShortAngleDelta(a.rotationDegrees,b.rotationDegrees))>45.f ||
        !std::isfinite(a.destination.width) || !std::isfinite(b.destination.width) ||
        !std::isfinite(a.destination.height) || !std::isfinite(b.destination.height) ||
        a.destination.width<=0.f || b.destination.width<=0.f ||
        a.destination.height<=0.f || b.destination.height<=0.f ||
        std::fabs(b.destination.width-a.destination.width)>25.f ||
        std::fabs(b.destination.height-a.destination.height)>25.f ||
        std::abs(int(b.tint.a)-int(a.tint.a))>32) return false;
    // A fade scales both axes and alpha together; inconsistent independent
    // shape/opacity changes are not an interpolatable projectile phase.
    const float widthRatio=b.destination.width/a.destination.width;
    const float heightRatio=b.destination.height/a.destination.height;
    return std::fabs(widthRatio-heightRatio)<0.005f &&
        (a.tint.a==b.tint.a ||
         std::fabs(widthRatio-float(b.tint.a)/float(a.tint.a))<0.015f);
}
// Source-stamped continuous draw state. Frame images, flip, blend, shader
// shape and RGB remain discrete. Actor aim is the only authored rotating
// scene sprite; fade sources are explicit rather than inferred from pixels.
inline bool SceneContinuousPhaseAdmitted(const PresentationCommandPacket::Command& x,
                                         const PresentationCommandPacket::Command& y) {
    if (x.continuousAlpha != y.continuousAlpha ||
        x.continuousRotation != y.continuousRotation) return false;
    auto a=x.sprite, b=y.sprite;
    if (x.continuousAlpha) {
        if (std::abs(int(b.tint.a)-int(a.tint.a))>16 ||
            !std::isfinite(a.opacity) || !std::isfinite(b.opacity) ||
            std::fabs(b.opacity-a.opacity)>.07f) return false;
        a.tint.a=b.tint.a; a.opacity=b.opacity;
    }
    if (x.continuousRotation) {
        if (!std::isfinite(a.rotationDegrees) || !std::isfinite(b.rotationDegrees) ||
            std::fabs(ShortAngleDelta(a.rotationDegrees,b.rotationDegrees))>45.f) return false;
        a.rotationDegrees=b.rotationDegrees;
    }
    return SameSpriteExceptPosition(a,b);
}
// Diagnostic first-blocker classification only; it never changes the sampled packet.
enum class MixedHoldReason { Invalid, SceneCut, UnownedSprite, StructuralPass, OrdinalReorder, DiscreteImage, NoEligibleMotion, Admitted };
struct MixedCommandDecision {
    MixedHoldReason holdReason = MixedHoldReason::Invalid;
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
    out.holdReason=MixedHoldReason::NoEligibleMotion;
    out.interpolate.resize(b.packet.Size(),false);
    if (a.sceneEpoch!=b.sceneEpoch || a.discontinuity || b.discontinuity ||
        std::fabs(b.cameraX-a.cameraX)>250.f || std::fabs(b.cameraY-a.cameraY)>250.f) { out.holdReason=MixedHoldReason::SceneCut; return out; }
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
    if (hasUnownedSprite(ac) || hasUnownedSprite(bc)) { out.holdReason=MixedHoldReason::UnownedSprite; return out; }
    // Global render-pass state cannot be mixed. A changed pass topology or zoom
    // could make a previously sampled draw use the wrong target/shader/blend.
    for (std::size_t i=0;i<std::max(ac.size(),bc.size());++i) {
        const auto structural=[](Kind k) { return k==Kind::Begin || k==Kind::Zoom || k==Kind::Postprocess || k==Kind::Resolve; };
        if (i>=ac.size() || i>=bc.size()) {
            if ((i<ac.size() && structural(ac[i].kind)) || (i<bc.size() && structural(bc[i].kind))) { out.holdReason=MixedHoldReason::StructuralPass; return out; }
            continue;
        }
        if (structural(ac[i].kind) || structural(bc[i].kind)) {
            if (ac[i].kind!=bc[i].kind ||
                (ac[i].kind==Kind::Begin && !SameColor(ac[i].color,bc[i].color)) ||
                (ac[i].kind==Kind::Zoom && ac[i].zoom!=bc[i].zoom)) { out.holdReason=MixedHoldReason::StructuralPass; return out; }
        }
    }
    // A semantic reorder changes occlusion and cannot be mixed with the old
    // packet even when individual sprite IDs survive.
    for (std::size_t i=0;i<std::min(ac.size(),bc.size());++i)
        if ((ac[i].kind==Kind::Sprite || bc[i].kind==Kind::Sprite) &&
            (ac[i].kind!=bc[i].kind || !(ac[i].key==bc[i].key))) { out.holdReason=MixedHoldReason::OrdinalReorder; return out; }
    // A new discrete image is selected only at a fixed simulation boundary.
    // Never combine previous-step spatial phases with current-step actor or
    // scene-animation imagery. Persistent HUD film stays on its prior texture
    // until alpha=1, preserving its discrete fixed-step clock.
    for (std::size_t i=0;i<std::min(ac.size(),bc.size());++i)
        if (ac[i].kind==Kind::Sprite && bc[i].kind==Kind::Sprite &&
            ac[i].key==bc[i].key && ac[i].key.object &&
            ((ac[i].sprite.texture!=bc[i].sprite.texture && !IsPersistentHudFilm(ac[i])) ||
             (!ac[i].continuousAlpha && ac[i].sprite.opacity!=bc[i].sprite.opacity))) { out.holdReason=MixedHoldReason::DiscreteImage; return out; }
    // Projectile motion is never sampled through a birth/death, painter reorder,
    // or batch-cardinality change elsewhere in the emitted active frame.
    bool stableEffectTopology=ac.size()==bc.size();
    if (stableEffectTopology) for (std::size_t i=0;i<ac.size();++i) {
        if (ac[i].kind!=bc[i].kind ||
            ((ac[i].kind==Kind::Sprite || ac[i].kind==Kind::SpriteBatch) && !(ac[i].key==bc[i].key)) ||
            (ac[i].kind==Kind::SpriteBatch && ac[i].sprites.size()!=bc[i].sprites.size())) {
            stableEffectTopology=false; break;
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
            !(IsProjectileCommand(x) ? (stableEffectTopology && ProjectilePhaseAdmitted(x,y)) :
              IsParticleCommand(x) ? (stableEffectTopology && ParticlePhaseAdmitted(x,y)) :
              SceneContinuousPhaseAdmitted(x,y)) ||
            !PhaseAdmitted(x,y,a,b)) continue;
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
    if (out.moving) out.holdReason=MixedHoldReason::Admitted;
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
        if (i<a.packet.Size() && IsPersistentHudFilm(c) &&
            a.packet.Commands()[i].key==c.key &&
            a.packet.Commands()[i].sprite.texture!=c.sprite.texture)
            c=a.packet.Commands()[i];
        if (decision.interpolate[i]) {
            const auto& p=a.packet.Commands()[i].sprite;
            c.sprite.destination.x=lerp(p.destination.x,c.sprite.destination.x);
            c.sprite.destination.y=lerp(p.destination.y,c.sprite.destination.y);
            if (IsParticleCommand(c)) {
                c.sprite.destination.width=lerp(p.destination.width,c.sprite.destination.width);
                c.sprite.destination.height=lerp(p.destination.height,c.sprite.destination.height);
                c.sprite.tint.a=static_cast<std::uint8_t>(std::lround(lerp(float(p.tint.a),float(c.sprite.tint.a))));
            }
            if (IsProjectileCommand(c)) {
                c.sprite.destination.width=lerp(p.destination.width,c.sprite.destination.width);
                c.sprite.destination.height=lerp(p.destination.height,c.sprite.destination.height);
                c.sprite.rotationDegrees=p.rotationDegrees + alpha*ShortAngleDelta(p.rotationDegrees,c.sprite.rotationDegrees);
                c.sprite.tint.a=static_cast<std::uint8_t>(std::lround(lerp(float(p.tint.a),float(c.sprite.tint.a))));
            }
            if (c.continuousAlpha && !IsParticleCommand(c) && !IsProjectileCommand(c)) {
                c.sprite.tint.a=static_cast<std::uint8_t>(std::lround(lerp(float(p.tint.a),float(c.sprite.tint.a))));
                c.sprite.opacity=lerp(p.opacity,c.sprite.opacity);
            }
            if (c.continuousRotation && !IsProjectileCommand(c))
                c.sprite.rotationDegrees=p.rotationDegrees+alpha*ShortAngleDelta(p.rotationDegrees,c.sprite.rotationDegrees);
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

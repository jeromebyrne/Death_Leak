#include "LevelScene.h"
#include "../render/ViewportCull.h"
#include "../render/OrdinarySpriteRotation.h"
#include "../../Game/tinyxml/tinyxml.h"
#include <algorithm>
#include <cmath>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <unordered_map>

namespace deathleak::game {
namespace {
float Attr(const TiXmlElement* e, const char* name, float fallback = 0) {
    if (!e) return fallback;
    double value = fallback;
    e->QueryDoubleAttribute(name, &value);
    return static_cast<float>(value);
}
bool Flag(const TiXmlElement* e, const char* name) {
    return e && e->Attribute(name) && std::string(e->Attribute(name)) == "true";
}
std::string NormalizeAsset(const char* legacy) {
    if (!legacy) return {};
    std::string path(legacy);
    std::replace(path.begin(), path.end(), '\\', '/');
    if (path.empty() || path[0] == '/' || path.find("..") != std::string::npos) return {};
    if (path.rfind("port/tests/fixtures/", 0) == 0) return path;
    return (std::filesystem::path("Game") / path).lexically_normal().string();
}
int Depth(const char* name) {
    // Values mirror GameObject::DepthLayer. Keep original painter's ordering.
    static const std::unordered_map<std::string, int> layers = {
        {"kMoon",5999},{"kFarBackground",6000},{"kMiddleBackground",5500},
        {"kNearBackground",5000},{"kGroundBack",4750},{"kGround",4500},
        {"kGroundBlood",4000},{"kNpc",3500},{"kOrb",3250},{"kPlayer",3000},
        {"kGroundFront",1750},{"kFarForeground",1500},{"kMiddleForeground",1000},
        {"kNearForeground",500},{"kWeatherForeground",20}
    };
    auto it = layers.find(name ? name : "");
    return it == layers.end() ? 3000 : it->second;
}
std::string AnimationFrame(const std::string& xmlPath, const char* requestedSequence) {
    TiXmlDocument animation(xmlPath.c_str());
    if (!animation.LoadFile()) return {};
    auto* root = animation.RootElement();
    if (!root) return {};
    for (auto* part = root->FirstChildElement(); part; part = part->NextSiblingElement()) {
        for (auto* sequence = part->FirstChildElement(); sequence; sequence = sequence->NextSiblingElement()) {
            if (requestedSequence && std::string(sequence->Value()) != requestedSequence) continue;
            auto* frame = sequence->FirstChildElement("frame");
            if (frame && frame->Attribute("filename")) return NormalizeAsset(frame->Attribute("filename"));
        }
    }
    return {};
}
std::string FirstAnimationFrame(const std::string& path) { return AnimationFrame(path, nullptr); }
std::unordered_set<std::string> DiagnosticIds(const char* environmentName) {
    std::unordered_set<std::string> ids;
    const char* value = std::getenv(environmentName);
    if (!value) return ids;
    std::string token;
    for (const char* cursor = value;; ++cursor) {
        if (*cursor == ',' || *cursor == '\0') {
            if (!token.empty()) ids.insert(token);
            token.clear();
            if (*cursor == '\0') break;
        } else if (!std::isspace(static_cast<unsigned char>(*cursor))) {
            token += *cursor;
        }
    }
    return ids;
}
struct SpriteRenderFilter {
    // Both are off by default; when both are set, both conditions apply.
    std::unordered_set<std::string> only = DiagnosticIds("DEATHLEAK_RENDER_ONLY_XML_IDS");
    std::unordered_set<std::string> hide = DiagnosticIds("DEATHLEAK_RENDER_HIDE_XML_IDS");
    bool Draw(const std::string& id) const {
        return (only.empty() || only.count(id)) && !hide.count(id);
    }
};
const SpriteRenderFilter& DiagnosticRenderFilter() {
    static const SpriteRenderFilter filter;
    return filter;
}
struct LoopedFrames { std::vector<std::string> paths; float rate = 0; };
LoopedFrames StillFrames(const std::string& xmlPath) {
    LoopedFrames result;
    TiXmlDocument animation(xmlPath.c_str());
    if (!animation.LoadFile()) return result;
    auto* root = animation.RootElement();
    if (!root) return result;
    for (auto* part = root->FirstChildElement(); part; part = part->NextSiblingElement()) {
        auto* sequence = part->FirstChildElement("Still");
        if (!sequence) continue;
        result.rate = Attr(sequence, "FrameRate");
        for (auto* frame = sequence->FirstChildElement("frame"); frame; frame = frame->NextSiblingElement("frame")) {
            auto path = NormalizeAsset(frame->Attribute("filename"));
            if (!path.empty()) result.paths.push_back(path);
        }
        break; // CurrencyOrb's source Sprite animation uses its body part.
    }
    return result;
}

}
bool LevelScene::Load(const std::string& path, render::Renderer& renderer, std::string& error,
                      const std::string& fromLevel, const std::string& entryDoorId) {
    // Opt-in transition trace for physical-display stalls; no gameplay path changes.
    const bool traceLoad = std::getenv("DEATHLEAK_TRACE_SCENE_LOAD") != nullptr;
    const auto loadStart = std::chrono::steady_clock::now();
    auto trace = [&](const std::string& phase) {
        if (!traceLoad) return;
        const auto ms = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - loadStart).count();
        std::cerr << "Scene load " << path << " " << phase << " elapsed_ms=" << ms << std::endl;
    };
    trace("begin");
    TiXmlDocument document(path.c_str());
    if (!document.LoadFile()) { error = "Cannot parse level: " + path + ": " + document.ErrorDesc(); return false; }
    trace("xml parsed");
    auto* level = document.RootElement();
    if (!level || std::string(level->Value()) != "level") { error = "Not a Death Leak level: " + path; return false; }
    playerVisualWidth_ = playerVisualHeight_ = 0;
    introBodyPhase_ = IntroBodyPhase::None; introBodyFinished_ = false;
    playerSequences_.clear(); playerSequence_.clear(); playerFrame_ = 0; playerFrameElapsed_ = 0; impactVisualFinished_ = false; playerFacing_= playerAimLineFacing_ = 1; playerAimLineRotation_ = 0.f;
    playerAimLineFlipY_ = playerAimLineDirected_ = false; playerGrounded_ = false;
    playerCrouching_ = playerWasCrouching_ = false;
    playerOffSolidTime_ = playerDescentTime_ = 0; playerDescending_ = false;
    playerArmSequences_.clear(); playerArmSequence_.clear(); playerArmFrame_ = 0; playerArmFrameElapsed_ = 0; aimLineHold_ = aimLineAlpha_ = 0.f;
    spawnedOrbTexture_ = orbShatterTexture_ = render::InvalidTexture;
    orbTrails_.clear();
    // New ParallaxLayer vertices start at V=0; Timing::LastUpdateDelta is
    // process-owned and survives a level load on this scene instance.
    pendingModifiedDelta_ = 0.f;
    sprites_.clear(); particleSprays_.clear(); footstepSprays_.clear(); jumpTakeoffSprays_.clear(); landingSprays_.clear(); introSwordPullSprays_.clear(); groundParticleTextures_.clear();
    introBloodTextures_[0] = introBloodTextures_[1] = render::InvalidTexture; introBloodRandomState_ = 0x424c4f44u;
    playerFootstepSegment_ = static_cast<size_t>(-1); lastRunningFrame_ = static_cast<size_t>(-1); footstepRandomState_ = 0x464f4f54u; jumpRandomState_ = 0x4a554d50u; landingRandomState_ = 0x4c414e44u; ground_.clear(); solidBoxes_.clear(); triggers_.clear(); encounters_.clear(); hazards_.clear(); ambience_.clear(); doors_.clear(); boats_.clear(); keys_.clear(); papers_.clear(); healthDevils_.clear(); smashables_.clear(); spawnedOrbs_.clear(); scrolls_.clear(); skipped_ = 0;
    cameraBounds_ = false;
    allowsWeather_ = false;
    if (auto* props = level->FirstChildElement("LevelProperties")) {
        allowsWeather_ = Flag(props->FirstChildElement("weather_properties"), "allow_weather");
        if (auto* bounds = props->FirstChildElement("level_bounds")) {
            cameraBoundLeft_ = Attr(bounds, "left"); cameraBoundRight_ = Attr(bounds, "right");
            cameraBoundTop_ = Attr(bounds, "top"); cameraBoundBottom_ = Attr(bounds, "bottom");
            cameraBounds_ = true;
        }
        auto* camera = props->FirstChildElement("camera_properties");
        cameraX_ = Attr(camera, "posx"); cameraY_ = Attr(camera, "posy");
        cameraOffsetX_ = Attr(camera, "xOffset"); cameraOffsetY_ = Attr(camera, "yOffset");
        cameraLagX_ = Attr(camera, "xLag", 7.5f); cameraLagY_ = Attr(camera, "yLag", 15.f);
        cameraZoom_ = std::clamp(Attr(camera, "zoom_in_percent", 1.f), 0.2f, 1.f);
        cameraFollowX_ = !camera || !camera->Attribute("followx") || Flag(camera, "followx");
        cameraFollowY_ = !camera || !camera->Attribute("followy") || Flag(camera, "followy");
    }
    std::unordered_map<std::string, render::TextureId> textures;
    // Resolve only authored material particle assets used by solid-line contacts.
    std::unordered_map<std::string, std::vector<std::string>> materialParticles;
    TiXmlDocument materials("Game/XmlFiles/materials.xml");
    if (materials.LoadFile() && materials.RootElement())
        for (auto* material = materials.RootElement()->FirstChildElement("material"); material;
             material = material->NextSiblingElement("material")) {
            if (!material->Attribute("name")) continue;
            auto* particles = material->FirstChildElement("particletextures");
            for (auto* texture = particles ? particles->FirstChildElement("texture") : nullptr; texture;
                 texture = texture->NextSiblingElement("texture")) {
                auto filename = NormalizeAsset(texture->Attribute("file"));
                if (!filename.empty()) materialParticles[material->Attribute("name")].push_back(filename);
            }
        }
    // Orb and other repeated sprites share animation XML. Keep these parsed
    // paths only for this load so transitions still observe shipped assets.
    std::unordered_map<std::string, std::string> firstAnimationFrames;
    std::unordered_map<std::string, LoopedFrames> stillAnimationFrames;
    size_t objectOrder = 0;
    for (auto* object = level->FirstChildElement(); object; object = object->NextSiblingElement(), ++objectOrder) {
        auto* position = object->FirstChildElement("position");
        auto* dimensions = object->FirstChildElement("dimensions");
        auto* asset = object->FirstChildElement("texture");
        if (!position || !dimensions) continue;
        if (traceLoad && objectOrder % 32 == 0) trace("object " + std::to_string(objectOrder));
        const std::string kind = object->Value();
        const float x = Attr(position, "x"), y = Attr(position, "y");
        const int interactionDepth = Depth(position->Attribute("depth_layer"));
        const bool updateable = !object->Attribute("updateable") || Flag(object, "updateable");
        const bool alwaysUpdate = Flag(object, "always_update");
        if (kind == "AudioObject") {
            auto* file = object->FirstChildElement("audiofile");
            auto* fade = object->FirstChildElement("fadedimensions");
            auto* adjust = object->FirstChildElement("adjustvolumetocamera");
            auto* repeat = object->FirstChildElement("repeat");
            if (file && file->Attribute("value")) {
                std::string name = file->Attribute("value");
                std::replace(name.begin(), name.end(), '\\', '/');
                // AudioObject paths are relative to Game/Media/Audio, unlike textures.
                if (!name.empty() && name[0] != '/' && name.find("..") == std::string::npos)
                    ambience_.push_back({"Game/Media/Audio/" + name, x, y,
                        Attr(dimensions, "width"), Attr(dimensions, "height"),
                        Attr(fade, "x"), Attr(fade, "y"), Flag(adjust, "value"), Flag(repeat, "value")});
            }
        }
        if (kind == "Player") {
            playerX_ = x; playerY_ = y;
            playerVisualWidth_ = Attr(dimensions, "width");
            playerVisualHeight_ = Attr(dimensions, "height");
        }
        if (kind == "LevelEntry" && !fromLevel.empty()) {
            auto* cameFrom = object->FirstChildElement("level_came_from");
            auto* doorId = object->FirstChildElement("door_id");
            // An empty door ID means a LevelTrigger entry; a door must match both fields.
            if (NormalizeAsset(cameFrom ? cameFrom->Attribute("value") : nullptr) == fromLevel &&
                std::string(doorId && doorId->Attribute("value") ? doorId->Attribute("value") : "") == entryDoorId) {
                playerX_ = x; playerY_ = y;
            }
        }
        if (kind == "DojoScrollPickup") {
            auto* feature = object->FirstChildElement("feature_unlock");
            if (feature && feature->Attribute("value") && object->Attribute("id"))
                scrolls_.push_back({x,y,Attr(dimensions,"width"),Attr(dimensions,"height"),feature->Attribute("value"),std::filesystem::path(path).lexically_normal().string() + "#" + object->Attribute("id"),interactionDepth,objectOrder,updateable,alwaysUpdate});
        }
        if (kind == "KeyPickup") {
            auto* props = object->FirstChildElement("key_props");
            if (props && props->Attribute("id") && *props->Attribute("id"))
                keys_.push_back({x,y,Attr(dimensions,"width"),Attr(dimensions,"height"),props->Attribute("id"),interactionDepth,objectOrder,updateable,alwaysUpdate});
        }
        if (kind == "HealthDevil") healthDevils_.push_back({x,y,Attr(dimensions,"width"),Attr(dimensions,"height"),interactionDepth,objectOrder,updateable,alwaysUpdate});
        if (kind == "PaperPickup") {
            auto* loc = object->FirstChildElement("loc_id");
            if (loc && loc->Attribute("desc") && *loc->Attribute("desc"))
                papers_.push_back({x,y,Attr(dimensions,"width"),Attr(dimensions,"height"),loc->Attribute("desc"),loc->Attribute("title") ? loc->Attribute("title") : "",interactionDepth,objectOrder,updateable,alwaysUpdate});
        }
        if (kind == "Boat" && object->Attribute("id"))
            boats_.push_back({x,y,Attr(dimensions,"width"),Attr(dimensions,"height"),object->Attribute("id"),interactionDepth,objectOrder,updateable,alwaysUpdate});
        if (kind == "Door") {
            auto* to = object->FirstChildElement("to_level_file");
            auto* key = object->FirstChildElement("required_key");
            auto* id = object->FirstChildElement("door_id");
            auto destination = NormalizeAsset(to ? to->Attribute("value") : nullptr);
            if (!destination.empty() && std::filesystem::exists(destination))
                doors_.push_back({x,y,Attr(dimensions,"width"),Attr(dimensions,"height"),destination,
                    key && key->Attribute("value") ? key->Attribute("value") : "",
                    id && id->Attribute("value") ? id->Attribute("value") : "",interactionDepth,objectOrder,updateable,alwaysUpdate});
        }
        if (kind == "LevelTrigger") {
            auto* destination = object->FirstChildElement("level_to_load");
            auto file = NormalizeAsset(destination ? destination->Attribute("value") : nullptr);
            if (!file.empty() && std::filesystem::exists(file))
                triggers_.push_back({x, y, Attr(dimensions, "width"), Attr(dimensions, "height"), file});
        }
        if (kind == "NPCTrigger") {
            auto* spec = object->FirstChildElement("num_enemies");
            encounters_.push_back({x, y, Attr(dimensions, "width"), Attr(dimensions, "height"),
                static_cast<int>(Attr(spec, "value")), spec && spec->Attribute("enemy_type") ? spec->Attribute("enemy_type") : "",
                std::filesystem::path(path).lexically_normal().string() + "#" + (object->Attribute("id") ? object->Attribute("id") : "")});
        }
        // Keep solid entries in the positional hazard list for existing start-hazard
        // indices, but their damage is applied by solid-box contact below.
        if (kind == "WaterBlock" || object->FirstChildElement("applydamage")) {
            auto* damage = object->FirstChildElement("applydamage");
            float amount = kind != "SolidMovingSprite" && Flag(damage, "flag") ? Attr(damage, "amount") : 0.f;
            if (kind == "WaterBlock" || amount > 0 || (kind == "SolidMovingSprite" && Flag(damage, "flag")))
                hazards_.push_back({x, y, Attr(dimensions, "width"), Attr(dimensions, "height"), kind == "WaterBlock", amount});
        }
        if (kind == "SolidLineStrip") {
            const auto* materialAttribute = object->FirstChildElement("material");
            const std::string material = materialAttribute && materialAttribute->Attribute("value") ?
                materialAttribute->Attribute("value") : "";
            if (!materialParticles[material].empty() &&
                !groundParticleTextures_.count(material)) {
                const auto& filename = materialParticles[material].front();
                if (std::filesystem::exists(filename)) {
                    std::string textureError;
                    groundParticleTextures_[material] = renderer.LoadTexture(filename, textureError);
                }
            }
            auto* points = object->FirstChildElement("points");
            int count = static_cast<int>(Attr(points, "num_points"));
            for (int i = 1; i < count; ++i) {
                const std::string x0 = "x_" + std::to_string(i - 1), y0 = "y_" + std::to_string(i - 1);
                const std::string x1 = "x_" + std::to_string(i), y1 = "y_" + std::to_string(i);
                ground_.push_back({x + Attr(points, x0.c_str()), y + Attr(points, y0.c_str()),
                                   x + Attr(points, x1.c_str()), y + Attr(points, y1.c_str()), material});
            }
        }
        if (kind == "Smashable") {
            auto* box = object->FirstChildElement("collisionboxdimensions");
            if (box) {
                const float cx = x + Attr(box, "offsetX"), cy = y + Attr(box, "offsetY");
                const float halfX = Attr(box, "x") * 0.5f, halfY = Attr(box, "y") * 0.5f;
                if (halfX > 0 && halfY > 0) smashables_.push_back({{cx-halfX,cx+halfX,cy-halfY,cy+halfY,0.f},x,y,false});
            }
        }
        if (kind == "SolidMovingSprite") {
            auto* box = object->FirstChildElement("collisionboxdimensions");
            if (box) {
                const float cx = x + Attr(box, "offsetX"), cy = y + Attr(box, "offsetY");
                const float halfX = Attr(box, "x") * 0.5f, halfY = Attr(box, "y") * 0.5f;
                if (halfX > 0 && halfY > 0)
                    solidBoxes_.push_back({cx-halfX, cx+halfX, cy-halfY, cy+halfY,
                        Flag(object->FirstChildElement("applydamage"), "flag") ? Attr(object->FirstChildElement("applydamage"), "amount") : 0.f});
            }
        }
        if (kind == "ParticleSpray") {
            // Admit only the source-audited forest leaves and three waterfall-top splashes.
            const std::string levelName = std::filesystem::path(path).filename().string();
            const std::string sprayId = object->Attribute("id") ? object->Attribute("id") : "";
            const bool forestLeaf = (levelName == "forest_exploration_1.xml" && sprayId == "47") ||
                (levelName == "forest_exploration_2.xml" && sprayId == "37");
            const bool waterfallSplash = levelName == "waterfall_top.xml" &&
                (sprayId == "49" || sprayId == "50" || sprayId == "51");
            if (!forestLeaf && !waterfallSplash) continue;
            auto filename = NormalizeAsset(asset ? asset->Attribute("filename") : nullptr);
            if (filename.empty() || !std::filesystem::exists(filename)) { ++skipped_; continue; }
            ParticleSpray spray;
            spray.xmlId = sprayId; spray.order = objectOrder;
            // Keep the forest stream unchanged. Each splash has a stable private
            // capture stream; none reproduces the original global rand().
            if (waterfallSplash) spray.randomState = 0x53504c48u +
                static_cast<std::uint32_t>(std::stoi(sprayId) - 49);
            spray.x = x; spray.y = y;
            spray.width = Attr(dimensions, "width"); spray.height = Attr(dimensions, "height");
            spray.depth = interactionDepth;
            spray.updateable = updateable; spray.alwaysUpdate = alwaysUpdate;
            spray.looping = Flag(object->FirstChildElement("loop"), "value");
            spray.loopTime = Attr(object->FirstChildElement("loop"), "looptime");
            auto* fade = object->FirstChildElement("fade_time_percent");
            spray.fadeIn = Attr(fade, "in", .1f); spray.fadeOut = Attr(fade, "out", .6f);
            auto* size = object->FirstChildElement("size");
            spray.scalesByLifetime = Flag(size, "scaleto"); spray.scaleTo = Attr(size, "scaletovalue", 1.f);
            auto found = textures.find(filename);
            if (found == textures.end()) {
                std::string textureError;
                spray.texture = renderer.LoadTexture(filename, textureError);
                textures.emplace(filename, spray.texture);
            } else spray.texture = found->second;
            if (!spray.texture) { ++skipped_; continue; }
            const int count = std::max(0, std::min(1024, static_cast<int>(Attr(object->FirstChildElement("numparticles"), "value"))));
            auto* direction = object->FirstChildElement("direction");
            auto* speed = object->FirstChildElement("speed");
            auto* lifetime = object->FirstChildElement("livetime");
            auto* brightness = object->FirstChildElement("brightness");
            const float minSpeed = Attr(speed, "min"), maxSpeed = Attr(speed, "max");
            const float minSize = Attr(size, "min"), maxSize = Attr(size, "max");
            const float minLife = Attr(lifetime, "min"), maxLife = Attr(lifetime, "max");
            const float minBright = Attr(brightness, "min"), maxBright = std::clamp(Attr(brightness, "max"), 0.f, 1.f);
            for (int i = 0; i < count; ++i) {
                ParticleSpray::Particle particle;
                // Source computes randSpread but never assigns it to direction.
                particle.directionX = Attr(direction, "x"); particle.directionY = Attr(direction, "y");
                particle.lifetime = std::max(.1f, maxLife - ((maxLife == minLife ? .1f : maxLife-minLife) / count) * i);
                auto spawn = [&](float spreadAmount) {
                    if (spreadAmount == 0) return 0.f;
                    const float magnitude = static_cast<float>(spray.Random(static_cast<unsigned>(spreadAmount*10.f) + 1));
                    return spray.Random(2) ? -magnitude : magnitude;
                };
                particle.startX = particle.x = x + spawn(Attr(position, "spawn_spread_x"));
                particle.startY = particle.y = y + spawn(Attr(position, "spawn_spread_y"));
                particle.drawX = particle.x; particle.drawY = particle.y;
                particle.size = particle.startSize = maxSize <= minSize ? maxSize :
                    static_cast<float>(spray.Random(static_cast<unsigned>(maxSize-minSize)) + static_cast<unsigned>(minSize) + 1);
                particle.drawSize = particle.size;
                particle.speed = particle.startSpeed = maxSpeed <= minSpeed ? maxSpeed :
                    minSpeed + .01f + spray.Random(static_cast<unsigned>((maxSpeed-minSpeed)*100.f))*.01f;
                particle.gravity = Attr(object->FirstChildElement("gravity"), "value");
                particle.brightness = maxBright <= minBright ? maxBright :
                    minBright + .01f + spray.Random(static_cast<unsigned>((maxBright-minBright)*100.f))*.01f;
                spray.particles.push_back(particle);
            }
            particleSprays_.push_back(std::move(spray));
            continue;
        }
        if (!asset || !asset->Attribute("filename") || !*asset->Attribute("filename")) continue;
        std::string filename = NormalizeAsset(asset->Attribute("filename"));
        std::string animationPath;
        if (Flag(object->FirstChildElement("isanimated"), "value")) {
            auto* animation = object->FirstChildElement("animationfile");
            animationPath = NormalizeAsset(animation ? animation->Attribute("value") : nullptr);
            if (!animationPath.empty()) {
                auto [it, inserted] = firstAnimationFrames.try_emplace(animationPath);
                if (inserted) it->second = FirstAnimationFrame(animationPath);
                filename = it->second;
            }
        }
        if (filename.empty() || !std::filesystem::exists(filename)) { ++skipped_; continue; }
        auto found = textures.find(filename);
        render::TextureId texture = render::InvalidTexture;
        if (found == textures.end()) {
            std::string textureError;
            if (traceLoad) trace("texture begin " + filename);
            texture = renderer.LoadTexture(filename, textureError);
            if (traceLoad) trace("texture end " + filename);
            textures.emplace(filename, texture);
        } else texture = found->second;
        if (!texture) { ++skipped_; continue; }
        SceneSprite sprite;
        sprite.texture = texture;
        if (const char* id = object->Attribute("id")) sprite.xmlId = id;
        sprite.player = kind == "Player";
        sprite.parallaxLayer = kind == "ParallaxLayer";
        if (sprite.parallaxLayer) {
            sprite.followCameraX = Flag(object->FirstChildElement("followcamx"), "value");
            sprite.followCameraY = Flag(object->FirstChildElement("followcamy"), "value");
            sprite.cameraParallaxX = Attr(object->FirstChildElement("cameraparallaxmultiplierx"), "value");
            sprite.cameraParallaxY = Attr(object->FirstChildElement("cameraparallaxmultipliery"), "value");
            sprite.autoScrollY = Flag(object->FirstChildElement("autoscrolly"), "value");
            sprite.autoScrollYSpeed = Attr(object->FirstChildElement("autoscrollyspeed"), "value");
        }
        if (sprite.player && !animationPath.empty()) {
            TiXmlDocument animation(animationPath.c_str());
            if (animation.LoadFile()) {
                auto* body = animation.RootElement() ? animation.RootElement()->FirstChildElement("body") : nullptr;
                // Character::UpdateAnimations selects these authored body sequences.
                for (const char* name : {"Still", "Running", "StrafeForward", "StrafeBack", "JumpingUp", "JumpingDown",
                                         "SlidingDown", "Roll", "Crouch", "CrouchUp", "ImpactLandSoft", "ImpactLandHard", "SwimIdle", "SwimForward", "Melee", "Melee2", "Melee3", "MeleeFinish",
                                         "IntroCutscene1", "IntroCutscene2"}) {
                    auto* sequence = body ? body->FirstChildElement(name) : nullptr;
                    if (!sequence) continue;
                    PlayerSequence visual;
                    visual.rate = Attr(sequence, "FrameRate");
                    visual.currentRate = visual.rate;
                    visual.minRate = Attr(sequence, "MinFrameRate");
                    visual.maxRate = Attr(sequence, "MaxFrameRate", 60.f);
                    for (auto* frame = sequence->FirstChildElement("frame"); frame; frame = frame->NextSiblingElement("frame")) {
                        auto path = NormalizeAsset(frame->Attribute("filename"));
                        if (path.empty() || !std::filesystem::exists(path)) continue;
                        auto foundFrame = textures.find(path);
                        render::TextureId frameTexture = render::InvalidTexture;
                        if (foundFrame == textures.end()) {
                            std::string textureError;
                            frameTexture = renderer.LoadTexture(path, textureError);
                            textures.emplace(path, frameTexture);
                        } else frameTexture = foundFrame->second;
                        if (frameTexture) {
                            visual.frames.push_back(frameTexture);
                            visual.footstepFrames.push_back(frame->Attribute("sfx_type") &&
                                std::string(frame->Attribute("sfx_type")) == "footstep");
                        }
                    }
                    if (!visual.frames.empty()) playerSequences_.emplace(name, std::move(visual));
                }
                if (playerSequences_.count("Still")) playerSequence_ = "Still";
                // Forest Player ID2's transient back-arm is a separate part.
                if (sprite.xmlId == "2") {
                    auto* arm = animation.RootElement() ? animation.RootElement()->FirstChildElement("arm") : nullptr;
                    for (const char* name : {"Still", "Running"}) {
                        auto* sequence = arm ? arm->FirstChildElement(name) : nullptr;
                        if (!sequence) continue;
                        PlayerSequence visual;
                        visual.rate = Attr(sequence, "FrameRate");
                        for (auto* frame = sequence->FirstChildElement("frame"); frame; frame = frame->NextSiblingElement("frame")) {
                            auto path = NormalizeAsset(frame->Attribute("filename"));
                            if (path.empty() || !std::filesystem::exists(path)) continue;
                            auto foundFrame = textures.find(path);
                            render::TextureId frameTexture = render::InvalidTexture;
                            if (foundFrame == textures.end()) {
                                std::string textureError;
                                frameTexture = renderer.LoadTexture(path, textureError);
                                textures.emplace(path, frameTexture);
                            } else frameTexture = foundFrame->second;
                            if (frameTexture) visual.frames.push_back(frameTexture);
                        }
                        if (!visual.frames.empty()) playerArmSequences_.emplace(name, std::move(visual));
                    }
                    if (auto it = playerArmSequences_.find("Still"); it != playerArmSequences_.end()) {
                        playerArmSequence_ = "Still";
                        playerArmFrame_ = 0; // Character::Initialise selects Still and Restart begins at frame zero.
                    }
                }
            }
        }
        if (kind == "CurrencyOrb" && !animationPath.empty()) {
            auto [stillIt, inserted] = stillAnimationFrames.try_emplace(animationPath);
            if (inserted) stillIt->second = StillFrames(animationPath);
            const auto& still = stillIt->second;
            if (still.rate > 0 && still.paths.size() > 1) {
                for (const auto& frame : still.paths) {
                    auto foundFrame = textures.find(frame);
                    render::TextureId frameTexture = render::InvalidTexture;
                    if (foundFrame == textures.end()) {
                        std::string textureError;
                        if (std::filesystem::exists(frame)) frameTexture = renderer.LoadTexture(frame, textureError);
                        textures.emplace(frame, frameTexture);
                    } else frameTexture = foundFrame->second;
                    if (frameTexture) sprite.animationFrames.push_back(frameTexture);
                }
                if (sprite.animationFrames.size() > 1) sprite.animationFrameRate = still.rate;
            }
        }
        sprite.x = Attr(position, "x"); sprite.y = Attr(position, "y");
        sprite.followWorldX = sprite.x;
        sprite.width = Attr(dimensions, "width"); sprite.height = Attr(dimensions, "height");
        sprite.rotation = Attr(position, "rotation");
        sprite.depth = Depth(position->Attribute("depth_layer"));
        sprite.order = objectOrder;
        sprite.currencyOrb = kind == "CurrencyOrb";
        // Authored non-native Forest/Waterfall orbs use the small-orb hover/tracking path.
        sprite.authoredSmallOrb = sprite.currencyOrb &&
            (std::filesystem::path(path).filename() == "waterfall_top.xml" ||
             std::filesystem::path(path).filename() == "forest_exploration_1.xml" ||
             std::filesystem::path(path).filename() == "forest_exploration_2.xml") &&
            !Flag(dimensions, "native");
        if (sprite.authoredSmallOrb) {
            sprite.orbOriginY = sprite.y;
            auto* sine = object->FirstChildElement("sine_wave_props");
            sprite.orbHoverOffsetY = Attr(sine, "y_offset", 20.f);
            sprite.orbHoverSpeed = Attr(sine, "amplitude", 2.f);
            // Deterministic phase for reproducible Linux captures, not Windows
            // rand() parity. Hash the normalized level path and authored ID.
            const auto key = std::filesystem::path(path).lexically_normal().string() + "#" + sprite.xmlId;
            std::uint32_t hash = 2166136261u;
            for (unsigned char c : key) hash = (hash ^ c) * 16777619u;
            sprite.orbPhase = (hash / 4294967296.f) * 6.283185307179586f;
        }
        sprite.keyPickup = kind == "KeyPickup";
        sprite.paperPickup = kind == "PaperPickup";
        sprite.healthDevil = kind == "HealthDevil";
        sprite.smashable = kind == "Smashable";
        if (sprite.smashable && !animationPath.empty()) {
            const auto brokenFrame = AnimationFrame(animationPath, "Damage4");
            if (!brokenFrame.empty() && std::filesystem::exists(brokenFrame)) {
                auto foundBroken = textures.find(brokenFrame);
                if (foundBroken == textures.end()) {
                    std::string textureError;
                    sprite.smashedTexture = renderer.LoadTexture(brokenFrame, textureError);
                    textures.emplace(brokenFrame, sprite.smashedTexture);
                } else sprite.smashedTexture = foundBroken->second;
            }
        }
        sprite.dojoScroll = kind == "DojoScrollPickup";
        if ((sprite.currencyOrb || sprite.dojoScroll) && object->Attribute("id"))
            sprite.persistentId = std::filesystem::path(path).lexically_normal().string() + "#" + object->Attribute("id");
        if (auto* effect = object->FirstChildElement("effect"); effect &&
            effect->Attribute("name") && std::string(effect->Attribute("name")) == "effectfoliagesway") {
            auto* props = object->FirstChildElement("foliage_sway_props");
            if (props) {
                auto& sway = sprite.foliageSway;
                sway.enabled = true;
                sway.bottomLeft = {Attr(props, "bottom_left_intensity_x"), Attr(props, "bottom_left_intensity_y")};
                sway.bottomRight = {Attr(props, "bottom_right_intensity_x"), Attr(props, "bottom_right_intensity_y")};
                sway.topRight = {Attr(props, "top_right_intensity_x"), Attr(props, "top_right_intensity_y")};
                sway.topLeft = {Attr(props, "top_left_intensity_x"), Attr(props, "top_left_intensity_y")};
                sway.timeMultiplier = Attr(props, "time_multiplier", 1.f);
                sway.worldX = sprite.x; sway.worldY = sprite.y;
            }
        }
        if (auto* effect = object->FirstChildElement("effect"); effect &&
            effect->Attribute("name") && std::string(effect->Attribute("name")) == "effectpixelwobble") {
            sprite.pixelWobble.enabled = true;
            sprite.pixelWobble.intensity = Attr(effect, "pixelwobbleintensity");
            sprite.pixelWobble.speedMod = Attr(effect, "pixelwobblespeedmod");
        }
        sprite.flipX = Flag(object->FirstChildElement("horizontalflip"), "value");
        sprite.flipY = Flag(object->FirstChildElement("verticalflip"), "value");
        // Windows Sprite::SetupVertices repeats non-native X at source-pixel scale.
        // ParallaxLayer overrides that UV span to one in its Draw method.
        sprite.repeatX = !sprite.parallaxLayer && !Flag(dimensions, "native") && Flag(dimensions, "repeatX");
        auto alpha = std::clamp(Attr(object->FirstChildElement("alpha"), "value", 1), 0.f, 1.f);
        sprite.alpha = alpha;
        sprite.tint.a = static_cast<std::uint8_t>(std::lround(alpha * 255));
        if (kind == "ScrollingSprite") {
            auto* scroll = object->FirstChildElement("scroll_properties");
            sprite.scrolling = scroll != nullptr;
            sprite.scrollStartX = Attr(scroll, "StartLerpX");
            sprite.scrollStartY = Attr(scroll, "StartLerpY");
            sprite.scrollDistanceX = Attr(scroll, "LerpDistanceX");
            sprite.scrollDistanceY = Attr(scroll, "LerpDistanceY");
            sprite.scrollTime = Attr(scroll, "LerpTime");
            sprite.scrollFadeIn = Attr(scroll, "FadeInTime");
            sprite.scrollFadeOut = Attr(scroll, "FadeOutTime");
            sprite.scrollOriginalAlpha = alpha;
            sprite.scrollWorldX = sprite.x;
            sprite.scrollWorldY = sprite.y;
        }
        auto* fade = object->FirstChildElement("fade_when_player_occluded");
        sprite.fadeWhenPlayerOccluded = Flag(fade, "value");
        sprite.alphaWhenOccluding = Attr(fade, "alphawhenoccluding", .5f);
        sprite.alphaWhenNotOccluding = Attr(fade, "alphawhennotoccluding", 1.f);
        sprite.updateable = updateable;
        sprite.alwaysUpdate = alwaysUpdate;
        if (sprite.width > 0 && sprite.height > 0) sprites_.push_back(sprite);
    }
    trace("objects parsed");
    if (std::any_of(sprites_.begin(), sprites_.end(), [](const SceneSprite& s) { return s.authoredSmallOrb; })) {
        const std::string shatterPath = "Game/Media/orb_shatter.png";
        if (!std::filesystem::exists(shatterPath)) { error = "Missing orb trail texture: " + shatterPath; return false; }
        std::string textureError;
        orbShatterTexture_ = renderer.LoadTexture(shatterPath, textureError);
        if (!orbShatterTexture_) { error = "Cannot load orb trail texture: " + textureError; return false; }
    }
    if (!smashables_.empty()) {
        const auto frame = AnimationFrame("Game/XmlFiles/orb_anim.xml", "Still");
        if (!frame.empty()) spawnedOrbTexture_ = renderer.LoadTexture(frame, error);
    }
    originalSpriteCount_ = sprites_.size();
    // GameObjectManager adds Character's shadow after all XML objects, then
    // orders by depth. Stable sorting keeps it after authored kGround peers.
    const auto playerSprite = std::find_if(sprites_.begin(), sprites_.end(),
        [](const SceneSprite& s) { return s.player; });
    if (playerSprite != sprites_.end()) {
        // LoadLevel adds this non-XML child after the authored player. Stable
        // depth sorting leaves it after the body among kPlayer objects.
        const std::string aimPath = "Game/Media/aim_line.png";
        std::string aimError;
        const auto aimTexture = renderer.LoadTexture(aimPath, aimError);
        if (!aimTexture) { error = "Cannot load player aim line: " + aimError; return false; }
        SceneSprite aim;
        aim.texture = aimTexture;
        aim.xmlId = "__player_aim_line";
        aim.playerAimLine = true;
        aim.width = aim.height = 400.f;
        aim.depth = Depth("kPlayer");
        aim.order = objectOrder;
        sprites_.push_back(aim);
        const std::string shadowPath = "Game/Media/characters/player/shadow.png";
        std::string textureError;
        const auto shadowTexture = renderer.LoadTexture(shadowPath, textureError);
        if (!shadowTexture) { error = "Cannot load player shadow: " + textureError; return false; }
        SceneSprite shadow;
        shadow.texture = shadowTexture;
        shadow.xmlId = "__player_shadow"; // Diagnostic ID for this non-XML child.
        shadow.playerShadow = true;
        shadow.width = 200.f; shadow.height = 128.f;
        shadow.depth = Depth("kGround");
        shadow.order = objectOrder;
        sprites_.push_back(shadow);
    }
    std::stable_sort(sprites_.begin(), sprites_.end(), [](const auto& a, const auto& b) { return a.depth > b.depth; });
    if (sprites_.empty()) { error = "Level has no renderable assets: " + path; return false; }
    trace("complete");
    return true;
}
size_t LevelScene::CollectCurrencyOrbs(float playerX, float playerY, std::unordered_set<std::string>& collected) {
    size_t count = 0;
    for (auto& object : sprites_) {
        if (!object.currencyOrb || object.collected) continue;
        const float dx = object.x - playerX, dy = object.y - playerY;
        // CurrencyOrb::OnCollision uses a 100-world-unit collection radius.
        if (dx * dx + dy * dy >= 100.f * 100.f) continue;
        object.collected = true;
        DetachOrbTrail(object.persistentId);
        if (!object.persistentId.empty()) collected.insert(object.persistentId);
        ++count;
    }
    return count;
}
void LevelScene::ApplyCollected(const std::unordered_set<std::string>& collected) {
    for (auto& object : sprites_)
        if (object.currencyOrb && collected.count(object.persistentId)) {
            object.collected = true;
            DetachOrbTrail(object.persistentId);
        }
}
void LevelScene::ApplyKeys(const std::unordered_set<std::string>& keys) {
    for (auto& object : sprites_) if (object.keyPickup) {
        for (const auto& key : keys_) if (key.x == object.x && key.y == object.y && keys.count(key.keyId))
            object.collected = true;
    }
}
void LevelScene::ApplyPapers(const std::unordered_set<std::string>& papers) {
    for (auto& object : sprites_) if (object.paperPickup) {
        for (const auto& paper : papers_) if (paper.x == object.x && paper.y == object.y && papers.count(paper.descriptionId))
            object.collected = true;
    }
}
void LevelScene::ApplyHealthDevil(bool rewarded) {
    for (auto& object : sprites_) if (object.healthDevil) object.collected = rewarded;
}
void LevelScene::ApplySmashable(bool smashed) {
    for (auto& object : smashables_) object.smashed = smashed;
    for (auto& sprite : sprites_) if (sprite.smashable && smashed && sprite.smashedTexture) sprite.texture = sprite.smashedTexture;
}
bool LevelScene::SmashAt(size_t index) {
    if (index >= smashables_.size() || smashables_[index].smashed) return false;
    smashables_[index].smashed = true;
    // Windows uses rand()%8+1 and SpawnOrbs with random launch velocity and
    // 0.01..0.109 s pickup lockout. A stable per-object sequence makes Linux
    // scripted routes reproducible without claiming a Windows RNG match.
    const auto& origin = smashables_[index];
    unsigned int random = static_cast<unsigned int>(std::lround(std::abs(origin.x)*100.f)) ^
                          (static_cast<unsigned int>(std::lround(std::abs(origin.y)*100.f)) << 1) ^
                          (static_cast<unsigned int>(index+1) * 0x9e3779b9u);
    auto next = [&random]() { random = random * 1664525u + 1013904223u; return random; };
    const unsigned int count = next() % 8 + 1;
    for (unsigned int i = 0; i < count; ++i) {
        const float vx = (next()%100) * 0.18f * ((next()%2) ? -1.f : 1.f) * 60.f;
        const float vy = ((next()%100) * 0.3f + 30.f) * ((next()%2) ? -1.f : 1.f) * 60.f;
        const float delay = (next()%100) * 0.001f + 0.01f;
        spawnedOrbs_.push_back({origin.x, origin.y, vx, vy, delay});
    }
    // The shipped levels have one Smashable each; keep other objects independent.
    const auto& box = smashables_[index].box;
    for (auto& sprite : sprites_) if (sprite.smashable && sprite.smashedTexture &&
        sprite.x >= box.left && sprite.x <= box.right && sprite.y >= box.bottom && sprite.y <= box.top)
        sprite.texture = sprite.smashedTexture;
    return true;
}
size_t LevelScene::UpdateSpawnedOrbs(float playerX, float playerY, float dt) {
    size_t collected = 0;
    for (auto it = spawnedOrbs_.begin(); it != spawnedOrbs_.end();) {
        auto& orb = *it;
        if (orb.spawnDelay > 0) {
            orb.spawnDelay -= dt;
            orb.x += orb.vx * dt;
            orb.y += orb.vy * dt;
            ++it;
            continue;
        }
        orb.trackingTime += dt;
        const float dx = playerX-orb.x, dy = playerY-orb.y;
        const float distance = std::hypot(dx, dy);
        if (distance < 100.f) {
            it = spawnedOrbs_.erase(it);
            ++collected;
            continue;
        }
        if (distance > 0.001f) {
            // Source DoTrackPlayer accelerates 2.5 units/tick (3x after 2 s),
            // prioritizing the dominant axis, with a 20-unit/tick horizontal cap.
            const float acceleration = 2.5f * 60.f * 60.f * (orb.trackingTime > 2.f ? 3.f : 1.f);
            if (std::abs(dx) > std::abs(dy)) orb.vx += acceleration * dx/distance * dt;
            else orb.vy += acceleration * dy/distance * dt;
            orb.vx = std::clamp(orb.vx, -1200.f, 1200.f);
            orb.vy = std::clamp(orb.vy, -3000.f, 3000.f);
        }
        orb.x += orb.vx * dt;
        orb.y += orb.vy * dt;
        ++it;
    }
    return collected;
}
void LevelScene::ApplyDeflection(bool unlocked) {
    if (!unlocked) return;
    for (auto& object : sprites_) if (object.dojoScroll) {
        for (const auto& scroll : scrolls_) if (scroll.feature == "kDeflection" &&
            object.persistentId == scroll.id) object.collected = true;
    }
}
std::vector<LevelScene::OrbTrailView> LevelScene::OrbTrails() const {
    std::vector<OrbTrailView> result;
    for (const auto& trail : orbTrails_) result.push_back({trail.owner, trail.attached, &trail.spray});
    return result;
}
void LevelScene::DetachOrbTrail(const std::string& owner) {
    for (auto& trail : orbTrails_) if (trail.owner == owner) {
        trail.attached = false;
        trail.spray.looping = false;
    }
}
void LevelScene::AddOrbTrail(const SceneSprite& orb) {
    if (!orbShatterTexture_ || orb.persistentId.empty() ||
        std::any_of(orbTrails_.begin(), orbTrails_.end(), [&](const OrbTrail& t) { return t.owner == orb.persistentId; })) return;
    OrbTrail trail;
    trail.owner = orb.persistentId;
    auto& spray = trail.spray;
    spray.texture = orbShatterTexture_;
    spray.x = orb.x; spray.y = orb.y;
    spray.width = 3200.f; spray.height = 1200.f;
    spray.depth = Depth("kOrb");
    spray.looping = true; spray.loopTime = -1.f;
    spray.scalesByLifetime = true; spray.scaleTo = .1f;
    spray.fadeIn = .15f; spray.fadeOut = .8f;
    spray.warmed = true; // Runtime radial factory does not request XML Warm(true).
    std::uint32_t hash = 2166136261u;
    for (unsigned char c : orb.persistentId) hash = (hash ^ c) * 16777619u;
    spray.randomState = hash;
    for (int i = 0; i < 15; ++i) {
        ParticleSpray::Particle p;
        p.directionX = (spray.Random(100)+1)*.01f * (spray.Random(2) ? 1.f : -1.f);
        p.directionY = (spray.Random(100)+1)*.01f * (spray.Random(2) ? -1.f : 1.f);
        p.lifetime = .35f - (.25f/15.f)*i;
        auto spread = [&]() { const float n = static_cast<float>(spray.Random(31)); return spray.Random(2) ? -n : n; };
        p.startX = p.x = orb.x + spread();
        p.startY = p.y = orb.y + spread();
        p.drawX = p.x; p.drawY = p.y;
        p.size = p.startSize = static_cast<float>(spray.Random(40)+41);
        p.drawSize = p.size;
        p.speed = p.startSpeed = 1.51f + spray.Random(190)*.01f;
        p.gravity = .5f;
        p.brightness = .81f + spray.Random(20)*.01f;
        spray.particles.push_back(p);
    }
    orbTrails_.push_back(std::move(trail));
}
void LevelScene::UpdateAuthoredOrbs(float dt, float playerX, float playerY, float cameraX, float cameraY) {
    for (auto& orb : sprites_) {
        if (!orb.authoredSmallOrb || orb.collected || !orb.updateable) continue;
        // GameObjectManager tests current bounds before GameObject::Update.
        if (!orb.alwaysUpdate && !(orb.x + orb.width*.5f > cameraX - 2000.f &&
            orb.x - orb.width*.5f < cameraX + 2000.f &&
            orb.y + orb.height*.5f > cameraY - 2000.f &&
            orb.y - orb.height*.5f < cameraY + 2000.f)) continue;
        if (orb.orbHoverEnabled) {
            orb.orbPhase += orb.orbHoverSpeed * dt;
            orb.y = orb.orbOriginY + orb.orbHoverOffsetY * std::sin(orb.orbPhase);
        }
        if (!orb.orbTracking) {
            const float dx = playerX - orb.x, dy = playerY - orb.y;
            if (dx*dx + dy*dy < 130.f*130.f) {
                orb.orbTracking = true;
                AddOrbTrail(orb);
            }
            continue; // Source arms tracking after the base/sine update.
        }
        // MovingSprite integrates the previous steering command before
        // CurrencyOrb::DoTrackPlayer sets the next command.
        const float tickScale = dt * 60.f;
        orb.orbVx = std::clamp(orb.orbVx + orb.orbAx * orb.orbDirectionX, -20.f, 20.f);
        orb.orbVy += orb.orbAy * orb.orbDirectionY;
        orb.x += orb.orbVx * tickScale;
        orb.y += orb.orbVy * tickScale;
        orb.orbHoverEnabled = false; // First tracking tick had its last sine step.
        orb.orbTrackingTime += dt;
        // Player collision-centre offset is not yet available in this scene API.
        const float dx = playerX - orb.x, dy = playerY - orb.y;
        const float length = std::hypot(dx, dy);
        if (length == 0.f) continue;
        const float directionX = dx / length, directionY = dy / length;
        // CurrencyOrb assigns the full normalized direction before choosing an
        // acceleration axis; the other axis's acceleration may persist.
        orb.orbDirectionX = directionX;
        orb.orbDirectionY = directionY;
        const float multiplier = orb.orbTrackingTime > 2.f ? 3.f : 1.f;
        if (std::abs(directionX) > std::abs(directionY)) {
            orb.orbAx = ((directionX < 0 && orb.orbVx > 0) ||
                         (directionX > 0 && orb.orbVx < 0) ? 4.8f : 2.5f) * multiplier;
            orb.orbDirectionX = directionX < 0 ? -1.f : 1.f;
        } else {
            orb.orbAy = ((directionY < 0 && orb.orbVy > 0) ||
                         (directionY > 0 && orb.orbVy < 0) ? 4.8f : 2.5f) * multiplier;
            orb.orbDirectionY = directionY < 0 ? -1.f : 1.f;
        }
        if (orb.orbVx != 0.f || orb.orbVy != 0.f)
            orb.rotation = std::atan2(orb.orbVx, -orb.orbVy);
    }
}
void LevelScene::UpdateVisuals(float dt, float playerX, float playerY, float cameraX, float cameraY,
                               bool objectsPaused, float precedingRenderedModifiedDelta) {
    // The game loop's current modified update delta is not visible to
    // ParallaxLayer::Draw until after this render has completed. Keep the most
    // recent update when several updates occur without a scene draw.
    pendingModifiedDelta_ = dt;
    if (objectsPaused) return;
    for (auto& spray : footstepSprays_) spray.Update(dt);
    footstepSprays_.erase(std::remove_if(footstepSprays_.begin(), footstepSprays_.end(),
        [](const ParticleSpray& spray) { return std::all_of(spray.particles.begin(), spray.particles.end(),
            [](const ParticleSpray::Particle& particle) { return particle.dead; }); }), footstepSprays_.end());
    for (auto& spray : jumpTakeoffSprays_) spray.Update(dt);
    jumpTakeoffSprays_.erase(std::remove_if(jumpTakeoffSprays_.begin(), jumpTakeoffSprays_.end(),
        [](const ParticleSpray& spray) { return std::all_of(spray.particles.begin(), spray.particles.end(),
            [](const ParticleSpray::Particle& particle) { return particle.dead; }); }), jumpTakeoffSprays_.end());
    for (auto& spray : introSwordPullSprays_) spray.Update(dt);
    introSwordPullSprays_.erase(std::remove_if(introSwordPullSprays_.begin(), introSwordPullSprays_.end(),
        [](const ParticleSpray& spray) { return std::all_of(spray.particles.begin(), spray.particles.end(),
            [](const ParticleSpray::Particle& particle) { return particle.dead; }); }), introSwordPullSprays_.end());
    for (auto& spray : landingSprays_) spray.Update(dt);
    landingSprays_.erase(std::remove_if(landingSprays_.begin(), landingSprays_.end(),
        [](const ParticleSpray& spray) { return std::all_of(spray.particles.begin(), spray.particles.end(),
            [](const ParticleSpray::Particle& particle) { return particle.dead; }); }), landingSprays_.end());
    for (auto& spray : particleSprays_) {
        if (spray.alwaysUpdate || (spray.updateable &&
            spray.x + spray.width*.5f > cameraX - 2000.f && spray.x - spray.width*.5f < cameraX + 2000.f &&
            spray.y + spray.height*.5f > cameraY - 2000.f && spray.y - spray.height*.5f < cameraY + 2000.f))
            spray.Update(dt);
    }
    for (auto& trail : orbTrails_) {
        float parent[2] = {0, 0};
        if (trail.attached) {
            auto found = std::find_if(sprites_.begin(), sprites_.end(), [&](const SceneSprite& s) {
                return s.persistentId == trail.owner && !s.collected;
            });
            if (found == sprites_.end()) { trail.attached = false; trail.spray.looping = false; }
            else { parent[0] = found->x; parent[1] = found->y; trail.spray.x = parent[0]; trail.spray.y = parent[1]; }
        }
        // Detached particles drain even outside the former parent's update zone.
        if (!trail.attached || (trail.spray.x + 1600.f > cameraX - 2000.f &&
            trail.spray.x - 1600.f < cameraX + 2000.f && trail.spray.y + 600.f > cameraY - 2000.f &&
            trail.spray.y - 600.f < cameraY + 2000.f))
            trail.spray.Update(dt, trail.attached ? parent : nullptr);
    }
    orbTrails_.erase(std::remove_if(orbTrails_.begin(), orbTrails_.end(), [](const OrbTrail& t) {
        return !t.attached && std::all_of(t.spray.particles.begin(), t.spray.particles.end(),
            [](const ParticleSpray::Particle& p) { return p.dead; });
    }), orbTrails_.end());
    for (auto& sprite : sprites_) {
        if (sprite.collected) continue;
        // GameObject builds the world matrix before ParallaxLayer follows camera X.
        // Retain GameObject world-matrix X before ParallaxLayer follows camera,
        // including schedules with multiple eligible updates per draw.
        if (sprite.parallaxLayer && sprite.followCameraX &&
            sprite.updateable && sprite.alwaysUpdate) {
            sprite.followWorldX = sprite.x;
            sprite.x = cameraX;
        }
        if (sprite.scrolling && sprite.updateable && sprite.scrollTime > 0.f) {
            // Match GameObjectManager's world-space update zone. Use the current
            // position: a scrolling sprite can enter or leave it mid-cycle.
            const bool inUpdateZone = sprite.alwaysUpdate ||
                (sprite.x + sprite.width * .5f > cameraX - 2000.f &&
                 sprite.x - sprite.width * .5f < cameraX + 2000.f &&
                 sprite.y + sprite.height * .5f > cameraY - 2000.f &&
                 sprite.y - sprite.height * .5f < cameraY + 2000.f);
            if (inUpdateZone) {
                // Sprite::Update builds m_world before ScrollingSprite changes XY.
                sprite.scrollWorldX = sprite.x;
                sprite.scrollWorldY = sprite.y;
                sprite.scrollElapsed += dt;
                if (sprite.scrollElapsed >= sprite.scrollTime) {
                    sprite.scrollElapsed = 0.f;
                    // ScrollingSprite returns here: retain the previous XY/alpha
                    // for this frame; the next update begins the new fade-in.
                } else {
                    const float percent = sprite.scrollElapsed / sprite.scrollTime;
                    sprite.x = sprite.scrollStartX + sprite.scrollDistanceX * percent;
                    sprite.y = sprite.scrollStartY + sprite.scrollDistanceY * percent;
                    if (sprite.scrollElapsed < sprite.scrollFadeIn)
                        sprite.alpha = sprite.scrollOriginalAlpha * (sprite.scrollElapsed / sprite.scrollFadeIn);
                    else if (sprite.scrollElapsed > sprite.scrollTime - sprite.scrollFadeOut)
                        sprite.alpha = sprite.scrollOriginalAlpha * ((sprite.scrollTime - sprite.scrollElapsed) / sprite.scrollFadeOut);
                    else sprite.alpha = sprite.scrollOriginalAlpha;
                    sprite.tint.a = static_cast<std::uint8_t>(std::lround(std::clamp(sprite.alpha, 0.f, 1.f) * 255.f));
                }
            }
        }
        if (sprite.fadeWhenPlayerOccluded && sprite.updateable) {
            // GameObjectManager updates objects whose authored rectangle intersects
            // its 4000x4000 world-space zone (or those marked always_update).
            const bool inUpdateZone = sprite.alwaysUpdate ||
                (sprite.x + sprite.width * .5f > cameraX - 2000.f &&
                 sprite.x - sprite.width * .5f < cameraX + 2000.f &&
                 sprite.y + sprite.height * .5f > cameraY - 2000.f &&
                 sprite.y - sprite.height * .5f < cameraY + 2000.f);
            if (inUpdateZone) {
                const bool occluding = playerX > sprite.x - sprite.width * .5f &&
                    playerX < sprite.x + sprite.width * .5f &&
                    playerY > sprite.y - sprite.height * .5f &&
                    playerY < sprite.y + sprite.height * .5f;
                const float target = occluding ? sprite.alphaWhenOccluding : sprite.alphaWhenNotOccluding;
                const bool decreasing = sprite.alphaWhenOccluding < sprite.alphaWhenNotOccluding;
                // Match DrawableObject::Update: step first, clamp on the next update.
                if (decreasing == occluding) {
                    if (sprite.alpha > target) sprite.alpha -= 1.5f * dt;
                    else sprite.alpha = target;
                } else {
                    if (sprite.alpha < target) sprite.alpha += 1.5f * dt;
                    else sprite.alpha = target;
                }
                sprite.tint.a = static_cast<std::uint8_t>(std::lround(std::clamp(sprite.alpha, 0.f, 1.f) * 255.f));
            }
        }
        if (sprite.animationFrames.size() < 2 || sprite.animationFrameRate <= 0) continue;
        // Sprite::UpdateAnimation runs only when GameObjectManager updates the orb.
        // Keep both its frame and fractional frame clock frozen outside that zone.
        if (!sprite.updateable || (!sprite.alwaysUpdate &&
            !(sprite.x + sprite.width * .5f > cameraX - 2000.f &&
              sprite.x - sprite.width * .5f < cameraX + 2000.f &&
              sprite.y + sprite.height * .5f > cameraY - 2000.f &&
              sprite.y - sprite.height * .5f < cameraY + 2000.f))) continue;
        sprite.animationElapsed += precedingRenderedModifiedDelta >= 0.f ? precedingRenderedModifiedDelta : dt;
        const float frameDuration = 1.f / sprite.animationFrameRate;
        // AnimationPart::AnimateLooped advances at most one frame per update.
        if (sprite.animationElapsed >= frameDuration) {
            sprite.animationElapsed -= frameDuration;
            sprite.animationFrame = (sprite.animationFrame + 1) % sprite.animationFrames.size();
        }
    }
}
void LevelScene::SetPlayerFootstepContact(size_t segment, float x, float y) {
    playerFootstepSegment_ = segment;
    playerFootstepX_ = x; playerFootstepY_ = y;
}
void LevelScene::EmitJumpTakeoff(size_t segment, float x, float postBumpY, int facing) {
    if (segment >= ground_.size()) return;
    const auto texture = groundParticleTextures_.find(ground_[segment].material);
    if (texture == groundParticleTextures_.end() || !texture->second) return;
    ParticleSpray spray;
    spray.texture = texture->second;
    spray.x = x + (facing < 0 ? -10.f : 10.f);
    spray.y = postBumpY - 214.898743f * .5f + 35.f;
    spray.width = 1200.f; spray.height = 720.f;
    spray.depth = Depth("kPlayer"); spray.looping = false; spray.warmed = true;
    spray.fadeIn = .15f; spray.fadeOut = .9f;
    spray.scalesByLifetime = true; spray.scaleTo = 5.f; spray.loopTime = 1.f;
    spray.randomState = jumpRandomState_;
    for (int i = 0; i < 10; ++i) {
        ParticleSpray::Particle particle;
        const float spread = spray.Random(80) * .01f;
        particle.directionX = spread;
        particle.directionY = 1.f - spread;
        particle.lifetime = 1.2f - (.4f / 10.f) * i;
        const int offsetX = static_cast<int>(spray.Random(51));
        const int signX = spray.Random(2) ? -1 : 1;
        const int offsetY = static_cast<int>(spray.Random(3));
        const int signY = spray.Random(2) ? -1 : 1;
        particle.x = particle.startX = particle.drawX = spray.x + offsetX * signX;
        particle.y = particle.startY = particle.drawY = spray.y + offsetY * signY;
        particle.size = particle.startSize = particle.drawSize = 32.f;
        particle.speed = particle.startSpeed = 1.01f + spray.Random(50) * .01f;
        particle.gravity = .8f;
        particle.brightness = .81f + spray.Random(20) * .01f;
        spray.particles.push_back(particle);
    }
    jumpRandomState_ = spray.randomState;
    jumpTakeoffSprays_.push_back(std::move(spray));
}
void LevelScene::EmitLandingMaterialSpray(size_t segment, float x, float y, int facing, float drop) {
    if (drop < 300.f || segment >= ground_.size()) return;
    const auto& line = ground_[segment];
    if (std::abs(line.x2 - line.x1) <= 1.f) return;
    const auto texture = groundParticleTextures_.find(line.material);
    if (texture == groundParticleTextures_.end() || !texture->second) return;
    const bool large = drop >= 700.f;
    const int count = large ? 10 : 5;
    ParticleSpray spray;
    spray.texture = texture->second;
    spray.x = x + (facing < 0 ? -10.f : 10.f);
    spray.y = y - 214.898743f * .5f + (large ? 0.f : 35.f);
    spray.width = 1200.f; spray.height = 720.f;
    spray.depth = Depth("kPlayer"); spray.looping = false; spray.warmed = true;
    spray.fadeIn = .15f; spray.fadeOut = .9f;
    spray.scalesByLifetime = true; spray.scaleTo = large ? 15.f : 5.f;
    spray.loopTime = 1.f; spray.randomState = landingRandomState_;
    for (int i = 0; i < count; ++i) {
        ParticleSpray::Particle particle;
        const float spread = spray.Random(large ? 40 : 80) * .01f;
        particle.directionX = spread;
        particle.directionY = 1.f - spread;
        particle.lifetime = large ? 1.f - (.6f / count) * i : 1.2f - (.4f / count) * i;
        const int offsetX = static_cast<int>(spray.Random(51));
        const int signX = spray.Random(2) ? -1 : 1;
        const int offsetY = static_cast<int>(spray.Random(3));
        const int signY = spray.Random(2) ? -1 : 1;
        particle.x = particle.startX = particle.drawX = spray.x + offsetX * signX;
        particle.y = particle.startY = particle.drawY = spray.y + offsetY * signY;
        particle.size = particle.startSize = particle.drawSize = large ? float(5 + spray.Random(6)) : 32.f;
        particle.speed = particle.startSpeed = large ? 3.01f + spray.Random(200) * .01f
                                                   : 1.01f + spray.Random(50) * .01f;
        particle.gravity = .8f;
        particle.brightness = .81f + spray.Random(20) * .01f;
        spray.particles.push_back(particle);
    }
    landingRandomState_ = spray.randomState;
    landingSprays_.push_back(std::move(spray));
}
void LevelScene::EmitRunningFootstep(const PlayerSequence& sequence, int movementDirection) {
    if (playerFrame_ >= sequence.footstepFrames.size() || !sequence.footstepFrames[playerFrame_] ||
        playerFootstepSegment_ >= ground_.size()) return;
    const auto& material = ground_[playerFootstepSegment_].material;
    if (material == "water") return;
    auto texture = groundParticleTextures_.find(material);
    if (texture == groundParticleTextures_.end() || !texture->second) return;
    ParticleSpray spray;
    spray.texture = texture->second;
    spray.x = playerFootstepX_ + (movementDirection < 0 ? -12.f : 12.f);
    spray.y = playerFootstepY_ - 214.898743f * .5f + 17.f;
    spray.width = 1200.f; spray.height = 720.f;
    spray.depth = Depth("kPlayer"); spray.looping = false; spray.warmed = true;
    spray.fadeIn = .15f; spray.fadeOut = .7f;
    spray.scalesByLifetime = true; spray.scaleTo = 2.5f;
    spray.randomState = footstepRandomState_;
    for (int i = 0; i < 3; ++i) {
        ParticleSpray::Particle particle;
        const float spread = spray.Random(10) * .01f;
        particle.directionX = (movementDirection > 0 ? -.5f : .5f) + spread;
        particle.directionY = .5f - spread;
        particle.lifetime = .7f - (.4f / 3.f) * i;
        // Runtime directed spray uses zero X/Y spawn spread for this call.
        particle.x = particle.startX = particle.drawX = spray.x;
        particle.y = particle.startY = particle.drawY = spray.y;
        particle.size = particle.startSize = particle.drawSize = float(41 + spray.Random(10));
        particle.speed = particle.startSpeed = 2.01f + spray.Random(200) * .01f;
        particle.gravity = .5f;
        particle.brightness = .81f + spray.Random(20) * .01f;
        spray.particles.push_back(particle);
    }
    footstepRandomState_ = spray.randomState;
    footstepSprays_.push_back(std::move(spray));
}
void LevelScene::EmitIntroSwordPullBurst(render::Renderer& renderer, float playerX, float playerY) {
    // The caller emits once on the held-Interact Looped -> Pull transition.
    // Keep this finite emitter distinct from movement and XML particle state.
    if (introBodyPhase_ != IntroBodyPhase::Looped) return;
    for (int i = 0; i < 2; ++i) {
        if (introBloodTextures_[i]) continue;
        std::string textureError;
        introBloodTextures_[i] = renderer.LoadTexture("Game/Media/bloodparticle" + std::to_string(i + 2) + ".png", textureError);
        if (!introBloodTextures_[i]) { std::cerr << "Intro blood texture unavailable: " << textureError << '\n'; return; }
    }
    struct Recipe { int count; float dx, dy, directionX, directionY, spread, minSpeed, maxSpeed, minLife, maxLife, minSize, maxSize, scaleTo; };
    constexpr Recipe recipes[] = {
        {20, -35.f, -15.f, -.75f, -.25f, .2f, 1.f, 4.f, .25f, .75f, 20.f, 50.f, 3.f},
        {50,   0.f, -20.f, -.5f,   .5f, .45f, 8.f, 18.f, .45f, .75f, 50.f, 75.f, 1.75f}
    };
    for (int n = 0; n < 2; ++n) {
        const auto& r = recipes[n];
        ParticleSpray spray;
        spray.texture = introBloodTextures_[n];
        spray.x = playerX + r.dx; spray.y = playerY + r.dy;
        spray.width = spray.height = 4000.f;
        spray.depth = Depth("kPlayer"); spray.looping = false; spray.warmed = true;
        spray.fadeIn = 5.f; spray.fadeOut = 1.f;
        spray.scalesByLifetime = true; spray.scaleTo = r.scaleTo;
        spray.randomState = introBloodRandomState_;
        for (int i = 0; i < r.count; ++i) {
            ParticleSpray::Particle p;
            const float spread = spray.Random(static_cast<unsigned>(r.spread * 100.f)) * .01f;
            p.directionX = r.directionX + spread; p.directionY = r.directionY - spread;
            p.lifetime = r.maxLife - ((r.maxLife - r.minLife) / r.count) * i;
            const float offsetX = static_cast<float>(spray.Random(2)) * .1f;
            const int signX = spray.Random(2) ? -1 : 1;
            const float offsetY = static_cast<float>(spray.Random(9)) * .1f;
            const int signY = spray.Random(2) ? -1 : 1;
            p.x = p.startX = p.drawX = spray.x + offsetX * signX;
            p.y = p.startY = p.drawY = spray.y + offsetY * signY;
            p.size = p.startSize = p.drawSize = r.minSize + 1.f + spray.Random(static_cast<unsigned>(r.maxSize-r.minSize));
            p.speed = p.startSpeed = r.minSpeed + .01f + spray.Random(static_cast<unsigned>((r.maxSpeed-r.minSpeed)*100.f)) * .01f;
            p.gravity = 1.5f;
            p.brightness = .51f + spray.Random(50) * .01f;
            spray.particles.push_back(p);
        }
        introBloodRandomState_ = spray.randomState;
        introSwordPullSprays_.push_back(std::move(spray));
    }
}
void LevelScene::SetIntroBodyPhase(IntroBodyPhase phase) {
    if (phase == introBodyPhase_) return;
    introBodyPhase_ = phase;
    introBodyFinished_ = false;
    if (phase == IntroBodyPhase::None) return;
    const char* name = phase == IntroBodyPhase::Looped ? "IntroCutscene1" : "IntroCutscene2";
    if (playerSequences_.find(name) == playerSequences_.end()) return;
    playerSequence_ = name;
    playerFrame_ = 0;
    playerFrameElapsed_ = 0.f;
    // Both authored intro arm sequences have zero frames. Retain their finished
    // state through phase exit; the first ordinary update then syncs to its
    // body sequence and Finish()es it, as Character::UpdateAnimations does.
    playerArmSequence_ = name;
    playerArmFrame_ = 0;
    playerArmFrameElapsed_ = 0.f;
}

size_t LevelScene::IntroBodyFrameCount() const {
    const char* name = introBodyPhase_ == IntroBodyPhase::Looped ? "IntroCutscene1" :
                       introBodyPhase_ == IntroBodyPhase::Pull ? "IntroCutscene2" : nullptr;
    if (!name) return 0;
    const auto it = playerSequences_.find(name);
    return it == playerSequences_.end() ? 0 : it->second.frames.size();
}

void LevelScene::UpdatePlayerVisual(float dt, float vx, float vy, bool grounded, int wallSide,
                                    bool rolling, bool dashing, bool inWater, float meleeElapsed, int facing,
                                    bool projectileFired, bool eligibleFireAttempt, PlayerStrafeVisual strafe,
                                    bool rightStickAim, float aimX, float aimY, float precedingRenderedModifiedDelta, bool crouch,
                                    PlayerImpactRecovery impactRecovery) {
    impactVisualFinished_ = false;
    // SetCrouching marks the release edge; movement/air branches can cancel it.
    if (playerCrouching_ && !crouch) playerWasCrouching_ = true;
    playerCrouching_ = crouch;
    playerGrounded_ = grounded;
    // Character::Update records the descent edge before UpdateAnimations. Its
    // dry upward tolerance is -0.5 world units/tick (velocity here is /s).
    const bool descending = !grounded && !inWater && wallSide == 0 && vy <= -0.5f * 60.f;
    if (grounded) playerOffSolidTime_ = 0;
    else playerOffSolidTime_ += dt;
    if (!descending) {
        playerDescending_ = false;
        playerDescentTime_ = 0;
    } else if (!playerDescending_) {
        playerDescending_ = true;
        playerDescentTime_ = 0;
    } else {
        playerDescentTime_ += dt;
    }
    // Player::FireWeapon refreshes this before its reload/burst rejection.
    // Player::Update then holds full alpha for 0.05 s and fades at 10/s.
    if (eligibleFireAttempt || strafe.active) aimLineHold_ = .05f;
    if (aimLineHold_ > 0.f) {
        aimLineHold_ -= dt;
        aimLineAlpha_ = 1.f;
    } else if (aimLineAlpha_ > 0.f) {
        aimLineAlpha_ = std::max(0.f, aimLineAlpha_ - 10.f * dt);
    }
    if (playerSequences_.empty()) return;
    if (introBodyPhase_ != IntroBodyPhase::None) {
        const char* name = introBodyPhase_ == IntroBodyPhase::Looped ? "IntroCutscene1" : "IntroCutscene2";
        const auto it = playerSequences_.find(name);
        if (it == playerSequences_.end() || it->second.frames.empty()) return;
        if (playerSequence_ != name) {
            playerSequence_ = name; playerFrame_ = 0; playerFrameElapsed_ = 0.f;
        }
        const auto& sequence = it->second;
        // XML ReadXml stores the authored initial rate directly; no SetFrameRate clamp occurs here.
        const float rate = sequence.rate;
        if (!introBodyFinished_ && rate > 0.f) {
            playerFrameElapsed_ += precedingRenderedModifiedDelta >= 0.f ? precedingRenderedModifiedDelta : dt;
            if (playerFrameElapsed_ >= 1.f / rate) {
                playerFrameElapsed_ -= 1.f / rate; // Source advances at most one frame per update.
                if (introBodyPhase_ == IntroBodyPhase::Looped)
                    playerFrame_ = (playerFrame_ + 1) % sequence.frames.size();
                else if (++playerFrame_ >= sequence.frames.size())
                    introBodyFinished_ = true; // IsFinished is true one tick after the final visible frame.
            }
        }
        playerFacing_ = facing;
        return; // Intro body bypasses ordinary movement/combat selection.
    }
    // InputManager calls SetAimLineDirection only for an active right stick.
    // Its four branches choose the X attachment/vertical reflection from X,
    // and the authored angle sign from Y. Neutral retains the last transform.
    if (rightStickAim && std::hypot(aimX, aimY) > 0.f) {
        const float length = std::hypot(aimX, aimY);
        const float angle = std::acos(std::clamp(aimX / length, -1.f, 1.f));
        playerAimLineFacing_ = aimX > 0.f ? 1 : -1;
        playerAimLineFlipY_ = aimX <= 0.f;
        playerAimLineRotation_ = ((aimY < 0.f) == (aimX > 0.f)) ? -angle : angle;
        playerAimLineDirected_ = true;
    }
    playerFacing_ = facing;
    std::string target = "Still";
    if (impactRecovery == PlayerImpactRecovery::Hard) target = "ImpactLandHard";
    else if (impactRecovery == PlayerImpactRecovery::Soft) target = "ImpactLandSoft";
    else if (meleeElapsed > 0.f) {
        // Combat's existing 38-tick phase model comes from the four source phases.
        if (meleeElapsed < 7.f/60.f) target = "Melee";
        else if (meleeElapsed < 22.f/60.f) target = "Melee2";
        else if (meleeElapsed < 31.f/60.f) target = "Melee3";
        else target = "MeleeFinish";
    } else if (rolling) target = "Roll";
    else if (playerCrouching_) target = "Crouch";
    else if (inWater) target = std::abs(vx) > 3.f ? "SwimForward" : "SwimIdle";
    else if (dashing) target = "JumpingDown"; // no dedicated authored dash body sequence
    else if (!grounded && wallSide != 0 && vy < 0.f) target = "SlidingDown";
    else if (!grounded && vy > -0.5f * 60.f) target = "JumpingUp";
    else if (!grounded) target = "JumpingDown";
    else if (std::abs(vx) > 3.f) {
        if (wallSide == 0 && strafe.active) {
            target = strafe.aimFacing != strafe.movementDirection ? "StrafeBack" : "StrafeForward";
            playerFacing_ = strafe.aimFacing;
        } else target = "Running";
    }
    if (target == "Still" && playerWasCrouching_) target = "CrouchUp";
    else if (target != "CrouchUp" && target != "Crouch") playerWasCrouching_ = false;
    // While a short dry descent is pending, Character keeps both the current
    // body frame and its fractional animation clock unchanged. The long-airtime
    // bypass and the strict 0.05 s comparison are source-authored.
    const bool holdFallingBody = target == "JumpingDown" && descending &&
        !dashing && !rolling && meleeElapsed <= 0.f && !inWater &&
        playerDescentTime_ <= 0.05 && playerOffSolidTime_ <= 0.25 &&
        !playerSequence_.empty();
    if (holdFallingBody) target = playerSequence_;
    auto it = playerSequences_.find(target);
    if (it == playerSequences_.end()) return;
    const bool enteredSequence = target != playerSequence_;
    if (enteredSequence) {
        playerSequence_ = target; playerFrame_ = 0; playerFrameElapsed_ = 0;
    }
    auto& sequence = it->second;
    const bool strafing = target == "StrafeForward" || target == "StrafeBack";
    // Character advances horizontal movement with the sequence's stored rate,
    // then derives the rate for its next update from the current velocity.
    const float advanceRate = (target == "Running" || strafing) ? sequence.currentRate : sequence.rate;
    if (!holdFallingBody && advanceRate > 0 && sequence.frames.size() >= 2) {
        playerFrameElapsed_ += precedingRenderedModifiedDelta >= 0.f ? precedingRenderedModifiedDelta : dt;
        if (playerFrameElapsed_ >= 1.f / advanceRate) {
            playerFrameElapsed_ -= 1.f / advanceRate; // AnimationPart advances at most one frame per update.
            if (target == "Still" || target == "Running" || target == "SlidingDown" ||
                target == "SwimIdle" || target == "SwimForward" || strafing)
                playerFrame_ = (playerFrame_ + 1) % sequence.frames.size();
            else if ((target == "Crouch" || target == "CrouchUp" || target == "ImpactLandSoft" || target == "ImpactLandHard") && playerFrame_ < sequence.frames.size()) ++playerFrame_;
            else if (playerFrame_ + 1 < sequence.frames.size()) ++playerFrame_;
        }
    }
    if (target == "CrouchUp" && playerFrame_ >= sequence.frames.size()) playerWasCrouching_ = false;
    if (target == "ImpactLandSoft" || target == "ImpactLandHard") {
        // AnimationPart::IsFinished becomes true only after advancing past the last frame.
        // Retain the last valid texture for the completion draw, then release motion.
        impactVisualFinished_ = playerFrame_ >= sequence.frames.size();
    }
    if (target == "Running" || strafing) sequence.currentRate = std::clamp(std::abs(vx / 60.f) * 1.8f * 2.65f,
                                                      sequence.minRate, sequence.maxRate);
    // Character uses one numeric frame guard for every horizontal movement
    // sequence, stamping even unmarked frames after the effect check. Strafe
    // body facing follows aim, but the directed spray follows movement.
    if (target == "Running" || strafing) {
        if (playerFrame_ != lastRunningFrame_ && grounded)
            EmitRunningFootstep(sequence, strafing ? strafe.movementDirection : playerFacing_);
        lastRunningFrame_ = playerFrame_;
    } else if (target == "Roll" && playerFrame_ != lastRunningFrame_ && grounded) {
        // Character checks marked Roll frames against the last *running* frame
        // on every update, but never stamps that index during Roll. Retaining
        // it allows repeated sprays while an authored frame is held.
        EmitRunningFootstep(sequence, strafe.movementDirection);
    }
    // Player::Fire restarts/advances once; Character::UpdateAnimations advances
    // again before switching/finishing when the body sequence changes.
    auto advanceArm = [&]() {
        auto arm = playerArmSequences_.find(playerArmSequence_);
        if (arm == playerArmSequences_.end() || playerArmFrame_ >= arm->second.frames.size() || arm->second.rate <= 0) return;
        playerArmFrameElapsed_ += precedingRenderedModifiedDelta >= 0.f ? precedingRenderedModifiedDelta : dt;
        if (playerArmFrameElapsed_ >= 1.f / arm->second.rate) {
            playerArmFrameElapsed_ -= 1.f / arm->second.rate;
            ++playerArmFrame_;
        }
    };
    if (projectileFired && !playerArmSequence_.empty()) {
        playerArmFrame_ = 0;
        playerArmFrameElapsed_ = 0;
        advanceArm();
    }
    advanceArm();
    if (playerArmSequence_ != target) {
        playerArmSequence_ = target;
        playerArmFrameElapsed_ = 0;
        auto arm = playerArmSequences_.find(target);
        playerArmFrame_ = arm == playerArmSequences_.end() ? 0 : arm->second.frames.size();
    }
}
void LevelScene::Draw(render::Renderer& renderer, float cameraX, float cameraY, float playerX, float playerY, std::vector<PainterItem> dynamic, float playerDamageBurstGain, float foliageGlobalTimeMultiplier, ShaderClockSnapshot shaderClock, bool suppressLandingSprayDraw, std::function<void()> afterPlayerDraw) const {
    constexpr float scale = 2.f / 3.f; // Original world is authored for 1920x1080.
    const auto& filter = DiagnosticRenderFilter();
    // Orbs are runtime GameObjects at kOrb, not an unconditional backdrop.
    if (spawnedOrbTexture_ && filter.only.empty()) for (const auto& orb : spawnedOrbs_) {
        dynamic.push_back({3250, [&, orb] {
            render::Sprite draw;
            draw.texture = spawnedOrbTexture_;
            draw.destination = {640.f + (orb.x-cameraX-23.84865f)*scale,
                                360.f + (cameraY-orb.y-36.415604f)*scale,
                                47.6973f*scale, 72.831207f*scale};
            draw.tint = {255,255,255,204};
            renderer.DrawSprite(draw);
        }});
    }
    std::stable_sort(dynamic.begin(), dynamic.end(),
                     [](const PainterItem& a, const PainterItem& b) { return a.depth > b.depth; });
    size_t nextDynamic = 0, nextSpray = 0;
    bool drewOrbTrails = false;
    auto drawDynamicBefore = [&](int depth) {
        while (nextDynamic < dynamic.size() && dynamic[nextDynamic].depth > depth)
            dynamic[nextDynamic++].draw();
    };
    auto drawSpraysBefore = [&](int depth, size_t order) {
        while (nextSpray < particleSprays_.size()) {
            const auto& spray = particleSprays_[nextSpray];
            if (spray.depth < depth || (spray.depth == depth && spray.order >= order)) break;
            drawDynamicBefore(spray.depth);
            if (filter.Draw(spray.xmlId) &&
                ((spray.xmlId != "47" && spray.xmlId != "37") ||
                 spray.Forest47EmitterInView(cameraX, cameraY)))
                spray.Draw(renderer, cameraX, cameraY);
            ++nextSpray;
        }
    };
    auto drawOrbTrails = [&]() {
        if (drewOrbTrails) return;
        drewOrbTrails = true;
        // Runtime trails were inserted after authored kOrb GameObjects. Dynamic
        // same-depth items follow them; this is a deterministic port tie rule.
        if (filter.only.empty()) for (const auto& trail : orbTrails_)
            trail.spray.Draw(renderer, cameraX, cameraY);
    };
    for (const auto& object : sprites_) {
        if (object.depth < Depth("kOrb")) drawOrbTrails();
        drawSpraysBefore(object.depth, object.order);
        drawDynamicBefore(object.depth);
        if (object.collected || (object.playerShadow && !playerGrounded_) ||
            (object.playerAimLine && aimLineAlpha_ <= 0.f) ||
            !filter.Draw(object.xmlId)) continue;
        // ParallaxLayer world matrix precedes camera-follow; current camera
        // coordinates still drive authored UV scroll.
        const float worldX = object.playerAimLine ? playerX + (playerAimLineFacing_ < 0 ? -5.f : 5.f) :
            object.player || object.playerShadow ? playerX :
            object.parallaxLayer && object.followCameraX ? object.followWorldX :
            object.scrolling ? object.scrollWorldX : object.x;
        const float worldY = object.playerAimLine ? playerY + 25.f :
            object.playerShadow ? playerY - 150.f : object.player ? playerY :
            object.parallaxLayer && object.followCameraY ? cameraY :
            object.scrolling ? object.scrollWorldY : object.y;
        float layerX = 1.f, layerY = 1.f;
        if (!object.player && !object.playerShadow && !object.playerAimLine && !object.parallaxLayer) {
            switch (object.depth) {
                case 5999: layerX = .03f; layerY = .90f; break;
                case 6000: layerX = .25f; layerY = .95f; break;
                case 5500: layerX = .50f; layerY = .97f; break;
                case 5000: layerX = .75f; layerY = .99f; break;
                case 1500: layerX = 1.10f; layerY = 1.05f; break;
                case 1000: layerX = 1.30f; layerY = 1.08f; break;
                case 500: case 20: layerX = 1.80f; layerY = 1.20f; break;
            }
        }
        render::Sprite draw;
        draw.texture = object.animationFrames.size() > 1 ? object.animationFrames[object.animationFrame] : object.texture;
        if (object.player) {
            auto it = playerSequences_.find(playerSequence_);
            if (it != playerSequences_.end() && !it->second.frames.empty())
                draw.texture = it->second.frames[std::min(playerFrame_, it->second.frames.size()-1)];
        }
        draw.destination = {640.f + ((worldX-cameraX)*layerX - object.width / 2) * scale,
                            360.f + ((cameraY-worldY)*layerY - object.height / 2) * scale,
                            object.width * scale, object.height * scale};
        if (object.parallaxLayer) {
            // D3D ParallaxLayer writes left UV = shift + 1, right UV = shift.
            // SDL samples left-to-right, so its equivalent phase has the opposite
            // sign. A positive camera move then moves texture features left.
            draw.uvShiftX = cameraX * object.cameraParallaxX * (object.flipX ? -1.f : 1.f);
            draw.uvShiftY = object.autoScrollY ? object.autoScrollYPhase
                                                 : -cameraY * object.cameraParallaxY;
        }
        // ParallaxLayer::Draw replaces Sprite's repeat-X UVs with exactly one
        // texture span whenever camera-X parallax is enabled. Keep its authored
        // UV scroll on the whole quad (not SDL's ordinary repeated-tile path).
        draw.repeatX = object.repeatX && !(object.parallaxLayer && object.cameraParallaxX > 0.f);
        draw.tint = object.tint;
        draw.opacity = object.alpha;
        if (object.player) draw.rgbGain = playerDamageBurstGain;
        if (object.playerAimLine) draw.tint.a = static_cast<std::uint8_t>(std::lround(std::clamp(aimLineAlpha_, 0.f, 1.f) * 255.f));
        draw.flipX = object.player ? playerFacing_ < 0 : object.flipX;
        // D3D Sprite vertices map V=0 to world-bottom (screen-bottom); SDL
        // maps V=0 to screen-top. Plain texture copies need the inverse of
        // the authored vertical-flip flag, not just ParallaxLayer copies.
        // The custom foliage/pixel shader paths own their source UV mapping.
        draw.foliageSway = object.foliageSway;
        if (draw.foliageSway.enabled && !object.player && !object.playerShadow &&
            !object.playerAimLine && !object.parallaxLayer) {
            draw.foliageSway.worldX = worldX - (cameraX - worldX) * (layerX - 1.f);
            draw.foliageSway.worldY = worldY - (cameraY - worldY) * (layerY - 1.f);
        }
        draw.foliageSway.time = shaderClock.totalTime;
        draw.foliageSway.globalTimeMultiplier = foliageGlobalTimeMultiplier;
        draw.pixelWobble = object.pixelWobble;
        draw.pixelWobble.time = shaderClock.pixelWobbleTime;
        draw.flipY = (draw.foliageSway.enabled || draw.pixelWobble.enabled)
            ? object.flipY : (object.playerAimLine ? !playerAimLineFlipY_ : !object.flipY);
        // Authored X/Y reflections follow rotation in Windows, while SDL and
        // custom effect quads reflect source labels before clockwise rotation.
        draw.rotationDegrees = render::OrdinarySpriteRotationDegrees(
            object.playerAimLine ? playerAimLineRotation_ : object.rotation,
            draw.flipX, object.playerAimLine ? playerAimLineFlipY_ : object.flipY);
        if (object.parallaxLayer && object.autoScrollY) {
            // Windows advances stored vertex V in ParallaxLayer::Draw before
            // Sprite::Draw. This also runs for a layer the port later culls.
            constexpr float worldToBackbufferY = 720.f / 1080.f;
            object.autoScrollYPhase += object.autoScrollYSpeed * worldToBackbufferY *
                .001f * (previousRenderedModifiedDelta_ * 60.f);
            object.autoScrollYPhase -= std::floor(object.autoScrollYPhase);
            draw.uvShiftY = object.autoScrollYPhase;
        }
        // Cull using the rendered rotated quad, not its unrotated destination.
        if (!render::SpriteMayIntersectViewport(draw)) continue;
        if (object.player && object.xmlId == "2" && introBodyPhase_ == IntroBodyPhase::None) {
            auto arm = playerArmSequences_.find(playerArmSequence_);
            if (arm != playerArmSequences_.end() && playerArmFrame_ < arm->second.frames.size()) {
                auto armDraw = draw;
                armDraw.texture = arm->second.frames[playerArmFrame_];
                renderer.DrawSprite(armDraw);
            }
        }
        renderer.DrawSprite(draw);
        if (object.player && filter.only.empty()) for (const auto& spray : footstepSprays_)
            spray.Draw(renderer, cameraX, cameraY);
        if (object.player && filter.only.empty()) for (const auto& spray : jumpTakeoffSprays_)
            spray.Draw(renderer, cameraX, cameraY);
        if (object.player && filter.only.empty() && !suppressLandingSprayDraw) for (const auto& spray : landingSprays_)
            spray.Draw(renderer, cameraX, cameraY);
        if (object.player && filter.only.empty()) for (const auto& spray : introSwordPullSprays_)
            spray.Draw(renderer, cameraX, cameraY);
        // Player::Draw emits its child prompt after Character::Draw as one
        // GameObject draw, before a subsequent same-depth object is visited.
        if (object.player && afterPlayerDraw) afterPlayerDraw();
    }
    drawOrbTrails();
    drawSpraysBefore(-1, 0);
    drawDynamicBefore(-2147483647);
    previousRenderedModifiedDelta_ = pendingModifiedDelta_;
}
}

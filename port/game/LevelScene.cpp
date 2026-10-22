#include "LevelScene.h"
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
    visualTime_ = pixelWobbleTime_ = 0.f;
    pixelWobbleReverse_ = false;
    playerSequences_.clear(); playerSequence_.clear(); playerFrame_ = 0; playerFrameElapsed_ = 0; playerFacing_ = 1; playerGrounded_ = false;
    playerOffSolidTime_ = playerDescentTime_ = 0; playerDescending_ = false;
    playerArmSequences_.clear(); playerArmSequence_.clear(); playerArmFrame_ = 0; playerArmFrameElapsed_ = 0; aimLineHold_ = aimLineAlpha_ = 0.f;
    spawnedOrbTexture_ = render::InvalidTexture;
    sprites_.clear(); particleSprays_.clear(); ground_.clear(); solidBoxes_.clear(); triggers_.clear(); encounters_.clear(); hazards_.clear(); ambience_.clear(); doors_.clear(); boats_.clear(); keys_.clear(); papers_.clear(); healthDevils_.clear(); smashables_.clear(); spawnedOrbs_.clear(); scrolls_.clear(); skipped_ = 0;
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
            auto* points = object->FirstChildElement("points");
            int count = static_cast<int>(Attr(points, "num_points"));
            for (int i = 1; i < count; ++i) {
                const std::string x0 = "x_" + std::to_string(i - 1), y0 = "y_" + std::to_string(i - 1);
                const std::string x1 = "x_" + std::to_string(i), y1 = "y_" + std::to_string(i);
                ground_.push_back({x + Attr(points, x0.c_str()), y + Attr(points, y0.c_str()),
                                   x + Attr(points, x1.c_str()), y + Attr(points, y1.c_str())});
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
            // Bounded slice: the forest's authored oak-leaf emitter only.
            if (std::filesystem::path(path).filename() != "forest_exploration_1.xml" ||
                !object->Attribute("id") || std::string(object->Attribute("id")) != "47") continue;
            auto filename = NormalizeAsset(asset ? asset->Attribute("filename") : nullptr);
            if (filename.empty() || !std::filesystem::exists(filename)) { ++skipped_; continue; }
            ParticleSpray spray;
            spray.xmlId = "47"; spray.order = objectOrder;
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
        }
        if (sprite.player && !animationPath.empty()) {
            TiXmlDocument animation(animationPath.c_str());
            if (animation.LoadFile()) {
                auto* body = animation.RootElement() ? animation.RootElement()->FirstChildElement("body") : nullptr;
                // Character::UpdateAnimations selects these authored body sequences.
                for (const char* name : {"Still", "Running", "JumpingUp", "JumpingDown",
                                         "SlidingDown", "Roll", "SwimIdle", "SwimForward", "Melee", "Melee2", "Melee3", "MeleeFinish"}) {
                    auto* sequence = body ? body->FirstChildElement(name) : nullptr;
                    if (!sequence) continue;
                    PlayerSequence visual;
                    visual.rate = Attr(sequence, "FrameRate");
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
                        if (frameTexture) visual.frames.push_back(frameTexture);
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
        sprite.width = Attr(dimensions, "width"); sprite.height = Attr(dimensions, "height");
        sprite.rotation = Attr(position, "rotation");
        sprite.depth = Depth(position->Attribute("depth_layer"));
        sprite.order = objectOrder;
        sprite.currencyOrb = kind == "CurrencyOrb";
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
        if (dx * dx + dy * dy > 100.f * 100.f) continue;
        object.collected = true;
        if (!object.persistentId.empty()) collected.insert(object.persistentId);
        ++count;
    }
    return count;
}
void LevelScene::ApplyCollected(const std::unordered_set<std::string>& collected) {
    for (auto& object : sprites_)
        if (object.currencyOrb && collected.count(object.persistentId)) object.collected = true;
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
void LevelScene::UpdateVisuals(float dt, float playerX, float playerY, float cameraX, float cameraY) {
    visualTime_ += dt;
    // Game::Draw shuttles this effect clock at strict 10/5 s thresholds;
    // the XML parameters are reapplied on each sprite draw.
    pixelWobbleTime_ += pixelWobbleReverse_ ? -dt : dt;
    if (!pixelWobbleReverse_ && pixelWobbleTime_ > 10.f) pixelWobbleReverse_ = true;
    else if (pixelWobbleReverse_ && pixelWobbleTime_ < 5.f) pixelWobbleReverse_ = false;
    for (auto& spray : particleSprays_) {
        if (spray.alwaysUpdate || (spray.updateable &&
            spray.x + spray.width*.5f > cameraX - 2000.f && spray.x - spray.width*.5f < cameraX + 2000.f &&
            spray.y + spray.height*.5f > cameraY - 2000.f && spray.y - spray.height*.5f < cameraY + 2000.f))
            spray.Update(dt);
    }
    for (auto& sprite : sprites_) {
        if (sprite.collected) continue;
        if (sprite.scrolling && sprite.updateable && sprite.scrollTime > 0.f) {
            // Match GameObjectManager's world-space update zone. Use the current
            // position: a scrolling sprite can enter or leave it mid-cycle.
            const bool inUpdateZone = sprite.alwaysUpdate ||
                (sprite.x + sprite.width * .5f > cameraX - 2000.f &&
                 sprite.x - sprite.width * .5f < cameraX + 2000.f &&
                 sprite.y + sprite.height * .5f > cameraY - 2000.f &&
                 sprite.y - sprite.height * .5f < cameraY + 2000.f);
            if (inUpdateZone) {
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
        sprite.animationElapsed += dt;
        const float frameDuration = 1.f / sprite.animationFrameRate;
        // AnimationPart::AnimateLooped advances at most one frame per update.
        if (sprite.animationElapsed >= frameDuration) {
            sprite.animationElapsed -= frameDuration;
            sprite.animationFrame = (sprite.animationFrame + 1) % sprite.animationFrames.size();
        }
    }
}
void LevelScene::UpdatePlayerVisual(float dt, float vx, float vy, bool grounded, int wallSide,
                                    bool rolling, bool dashing, bool inWater, float meleeElapsed, int facing,
                                    bool projectileFired, bool eligibleFireAttempt) {
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
    if (eligibleFireAttempt) aimLineHold_ = .05f;
    if (aimLineHold_ > 0.f) {
        aimLineHold_ -= dt;
        aimLineAlpha_ = 1.f;
    } else if (aimLineAlpha_ > 0.f) {
        aimLineAlpha_ = std::max(0.f, aimLineAlpha_ - 10.f * dt);
    }
    if (playerSequences_.empty()) return;
    playerFacing_ = facing;
    std::string target = "Still";
    if (meleeElapsed > 0.f) {
        // Combat's existing 38-tick phase model comes from the four source phases.
        if (meleeElapsed < 7.f/60.f) target = "Melee";
        else if (meleeElapsed < 22.f/60.f) target = "Melee2";
        else if (meleeElapsed < 31.f/60.f) target = "Melee3";
        else target = "MeleeFinish";
    } else if (rolling) target = "Roll";
    else if (inWater) target = std::abs(vx) > 3.f ? "SwimForward" : "SwimIdle";
    else if (dashing) target = "JumpingDown"; // no dedicated authored dash body sequence
    else if (!grounded && wallSide != 0 && vy < 0.f) target = "SlidingDown";
    else if (!grounded && vy > -0.5f * 60.f) target = "JumpingUp";
    else if (!grounded) target = "JumpingDown";
    else if (std::abs(vx) > 3.f) target = "Running";
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
    if (target != playerSequence_) {
        playerSequence_ = target; playerFrame_ = 0; playerFrameElapsed_ = 0;
    }
    auto& sequence = it->second;
    float rate = sequence.rate;
    if (target == "Running")
        rate = std::clamp(std::abs(vx / 60.f) * 1.8f * 2.65f, sequence.minRate, sequence.maxRate);
    if (!holdFallingBody && rate > 0 && sequence.frames.size() >= 2) {
        playerFrameElapsed_ += dt;
        if (playerFrameElapsed_ >= 1.f / rate) {
            playerFrameElapsed_ -= 1.f / rate; // AnimationPart advances at most one frame per update.
            if (target == "Still" || target == "Running" || target == "SlidingDown" ||
                target == "SwimIdle" || target == "SwimForward")
                playerFrame_ = (playerFrame_ + 1) % sequence.frames.size();
            else if (playerFrame_ + 1 < sequence.frames.size()) ++playerFrame_;
        }
    }
    // Player::Fire restarts/advances once; Character::UpdateAnimations advances
    // again before switching/finishing when the body sequence changes.
    auto advanceArm = [&]() {
        auto arm = playerArmSequences_.find(playerArmSequence_);
        if (arm == playerArmSequences_.end() || playerArmFrame_ >= arm->second.frames.size() || arm->second.rate <= 0) return;
        playerArmFrameElapsed_ += dt;
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
void LevelScene::Draw(render::Renderer& renderer, float cameraX, float cameraY, float playerX, float playerY, std::vector<PainterItem> dynamic, float playerDamageBurstGain) const {
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
    auto drawDynamicBefore = [&](int depth) {
        while (nextDynamic < dynamic.size() && dynamic[nextDynamic].depth > depth)
            dynamic[nextDynamic++].draw();
    };
    auto drawSpraysBefore = [&](int depth, size_t order) {
        while (nextSpray < particleSprays_.size()) {
            const auto& spray = particleSprays_[nextSpray];
            if (spray.depth < depth || (spray.depth == depth && spray.order >= order)) break;
            drawDynamicBefore(spray.depth);
            if (filter.Draw(spray.xmlId)) spray.Draw(renderer, cameraX, cameraY);
            ++nextSpray;
        }
    };
    for (const auto& object : sprites_) {
        drawSpraysBefore(object.depth, object.order);
        drawDynamicBefore(object.depth);
        if (object.collected || (object.playerShadow && !playerGrounded_) ||
            (object.playerAimLine && aimLineAlpha_ <= 0.f) ||
            !filter.Draw(object.xmlId)) continue;
        // ParallaxLayer follows the camera before its authored UV scroll. Static
        // background/foreground sprites use GameObject depth parallax instead.
        const float worldX = object.playerAimLine ? playerX + (playerFacing_ < 0 ? -5.f : 5.f) :
            object.player || object.playerShadow ? playerX :
            object.parallaxLayer && object.followCameraX ? cameraX : object.x;
        const float worldY = object.playerAimLine ? playerY + 25.f :
            object.playerShadow ? playerY - 150.f : object.player ? playerY :
            object.parallaxLayer && object.followCameraY ? cameraY : object.y;
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
            draw.uvShiftY = -cameraY * object.cameraParallaxY;
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
        draw.foliageSway.time = visualTime_;
        draw.pixelWobble = object.pixelWobble;
        draw.pixelWobble.time = pixelWobbleTime_;
        draw.flipY = (draw.foliageSway.enabled || draw.pixelWobble.enabled)
            ? object.flipY : (object.playerAimLine ? playerFacing_ >= 0 : !object.flipY);
        // XML rotation is radians in D3D world coordinates (Y up). SDL's
        // positive degrees rotate clockwise in screen coordinates (Y down),
        // so the sign is preserved when converting radians to degrees.
        draw.rotationDegrees = object.rotation * (180.f / 3.14159265358979323846f);
        if (draw.destination.x + draw.destination.width < 0 || draw.destination.x > 1280 ||
            draw.destination.y + draw.destination.height < 0 || draw.destination.y > 720) continue;
        if (object.player && object.xmlId == "2") {
            auto arm = playerArmSequences_.find(playerArmSequence_);
            if (arm != playerArmSequences_.end() && playerArmFrame_ < arm->second.frames.size()) {
                auto armDraw = draw;
                armDraw.texture = arm->second.frames[playerArmFrame_];
                renderer.DrawSprite(armDraw);
            }
        }
        renderer.DrawSprite(draw);
    }
    drawSpraysBefore(-1, 0);
    drawDynamicBefore(-2147483647);
}
}

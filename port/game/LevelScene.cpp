#include "LevelScene.h"
#include "../../Game/tinyxml/tinyxml.h"
#include <algorithm>
#include <cmath>
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
}
bool LevelScene::Load(const std::string& path, render::Renderer& renderer, std::string& error,
                      const std::string& fromLevel, const std::string& entryDoorId) {
    TiXmlDocument document(path.c_str());
    if (!document.LoadFile()) { error = "Cannot parse level: " + path + ": " + document.ErrorDesc(); return false; }
    auto* level = document.RootElement();
    if (!level || std::string(level->Value()) != "level") { error = "Not a Death Leak level: " + path; return false; }
    playerVisualWidth_ = playerVisualHeight_ = 0;
    spawnedOrbTexture_ = render::InvalidTexture;
    sprites_.clear(); ground_.clear(); solidBoxes_.clear(); triggers_.clear(); encounters_.clear(); hazards_.clear(); doors_.clear(); boats_.clear(); keys_.clear(); papers_.clear(); healthDevils_.clear(); smashables_.clear(); spawnedOrbs_.clear(); scrolls_.clear(); skipped_ = 0;
    if (auto* props = level->FirstChildElement("LevelProperties")) {
        auto* camera = props->FirstChildElement("camera_properties");
        cameraX_ = Attr(camera, "posx"); cameraY_ = Attr(camera, "posy");
    }
    std::unordered_map<std::string, render::TextureId> textures;
    for (auto* object = level->FirstChildElement(); object; object = object->NextSiblingElement()) {
        auto* position = object->FirstChildElement("position");
        auto* dimensions = object->FirstChildElement("dimensions");
        auto* asset = object->FirstChildElement("texture");
        if (!position || !dimensions) continue;
        const std::string kind = object->Value();
        const float x = Attr(position, "x"), y = Attr(position, "y");
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
                scrolls_.push_back({x,y,Attr(dimensions,"width"),Attr(dimensions,"height"),feature->Attribute("value"),std::filesystem::path(path).lexically_normal().string() + "#" + object->Attribute("id")});
        }
        if (kind == "KeyPickup") {
            auto* props = object->FirstChildElement("key_props");
            if (props && props->Attribute("id") && *props->Attribute("id"))
                keys_.push_back({x,y,Attr(dimensions,"width"),Attr(dimensions,"height"),props->Attribute("id")});
        }
        if (kind == "HealthDevil") healthDevils_.push_back({x,y,Attr(dimensions,"width"),Attr(dimensions,"height")});
        if (kind == "PaperPickup") {
            auto* loc = object->FirstChildElement("loc_id");
            if (loc && loc->Attribute("desc") && *loc->Attribute("desc"))
                papers_.push_back({x,y,Attr(dimensions,"width"),Attr(dimensions,"height"),loc->Attribute("desc"),loc->Attribute("title") ? loc->Attribute("title") : ""});
        }
        if (kind == "Boat" && object->Attribute("id"))
            boats_.push_back({x,y,Attr(dimensions,"width"),Attr(dimensions,"height"),object->Attribute("id")});
        if (kind == "Door") {
            auto* to = object->FirstChildElement("to_level_file");
            auto* key = object->FirstChildElement("required_key");
            auto* id = object->FirstChildElement("door_id");
            auto destination = NormalizeAsset(to ? to->Attribute("value") : nullptr);
            if (!destination.empty() && std::filesystem::exists(destination))
                doors_.push_back({x,y,Attr(dimensions,"width"),Attr(dimensions,"height"),destination,
                    key && key->Attribute("value") ? key->Attribute("value") : "",
                    id && id->Attribute("value") ? id->Attribute("value") : ""});
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
        if (!asset) continue;
        std::string filename = NormalizeAsset(asset->Attribute("filename"));
        std::string animationPath;
        if (Flag(object->FirstChildElement("isanimated"), "value")) {
            auto* animation = object->FirstChildElement("animationfile");
            animationPath = NormalizeAsset(animation ? animation->Attribute("value") : nullptr);
            if (!animationPath.empty()) filename = FirstAnimationFrame(animationPath);
        }
        if (filename.empty() || !std::filesystem::exists(filename)) { ++skipped_; continue; }
        auto found = textures.find(filename);
        render::TextureId texture = render::InvalidTexture;
        if (found == textures.end()) {
            std::string textureError;
            texture = renderer.LoadTexture(filename, textureError);
            textures.emplace(filename, texture);
        } else texture = found->second;
        if (!texture) { ++skipped_; continue; }
        SceneSprite sprite;
        sprite.texture = texture;
        sprite.x = Attr(position, "x"); sprite.y = Attr(position, "y");
        sprite.width = Attr(dimensions, "width"); sprite.height = Attr(dimensions, "height");
        sprite.rotation = Attr(position, "rotation");
        sprite.depth = Depth(position->Attribute("depth_layer"));
        sprite.player = kind == "Player";
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
        sprite.flipX = Flag(object->FirstChildElement("horizontalflip"), "value");
        sprite.flipY = Flag(object->FirstChildElement("verticalflip"), "value");
        auto alpha = std::clamp(Attr(object->FirstChildElement("alpha"), "value", 1), 0.f, 1.f);
        sprite.tint.a = static_cast<std::uint8_t>(std::lround(alpha * 255));
        if (sprite.width > 0 && sprite.height > 0) sprites_.push_back(sprite);
    }
    if (!smashables_.empty()) {
        const auto frame = AnimationFrame("Game/XmlFiles/orb_anim.xml", "Still");
        if (!frame.empty()) spawnedOrbTexture_ = renderer.LoadTexture(frame, error);
    }
    std::stable_sort(sprites_.begin(), sprites_.end(), [](const auto& a, const auto& b) { return a.depth > b.depth; });
    if (sprites_.empty()) { error = "Level has no renderable assets: " + path; return false; }
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
void LevelScene::Draw(render::Renderer& renderer, float cameraX, float cameraY, float playerX, float playerY) const {
    constexpr float scale = 2.f / 3.f; // Original world is authored for 1920x1080.
    for (const auto& orb : spawnedOrbs_) if (spawnedOrbTexture_) {
        render::Sprite draw;
        draw.texture = spawnedOrbTexture_;
        draw.destination = {640.f + (orb.x-cameraX-23.84865f)*scale,
                            360.f + (cameraY-orb.y-36.415604f)*scale,
                            47.6973f*scale, 72.831207f*scale};
        draw.tint = {255,255,255,204};
        renderer.DrawSprite(draw);
    }
    for (const auto& object : sprites_) {
        if (object.collected) continue;
        const float worldX = object.player ? playerX : object.x;
        const float worldY = object.player ? playerY : object.y;
        render::Sprite draw;
        draw.texture = object.texture;
        draw.destination = {640.f + (worldX - cameraX - object.width / 2) * scale,
                            360.f + (cameraY - worldY - object.height / 2) * scale,
                            object.width * scale, object.height * scale};
        draw.tint = object.tint; draw.flipX = object.flipX; draw.flipY = object.flipY;
        draw.rotationDegrees = -object.rotation;
        if (draw.destination.x + draw.destination.width < 0 || draw.destination.x > 1280 ||
            draw.destination.y + draw.destination.height < 0 || draw.destination.y > 720) continue;
        renderer.DrawSprite(draw);
    }
}
}

#include "Combat.h"
#include "../render/OrdinarySpriteRotation.h"
#include "../../Game/tinyxml/tinyxml.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace deathleak::game {
namespace {
bool Inside(float x, float y, float cx, float cy, float w, float h) {
    return std::abs(x-cx) <= w*0.5f && std::abs(y-cy) <= h*0.5f;
}
// First contact of a moving projectile center with an authored segment.
bool SegmentContact(float ax, float ay, float bx, float by,
                    float cx, float cy, float dx, float dy, float& contact) {
    const float rx = bx-ax, ry = by-ay, sx = dx-cx, sy = dy-cy;
    const float divisor = rx*sy-ry*sx;
    if (std::abs(divisor) < 0.00001f) return false;
    const float qx = cx-ax, qy = cy-ay;
    const float t = (qx*sy-qy*sx)/divisor;
    const float u = (qx*ry-qy*rx)/divisor;
    if (t < 0.f || t > 1.f || u < 0.f || u > 1.f) return false;
    contact = t;
    return true;
}
// Deterministic center sweep, including collinear overlap and endpoints.
bool BloodSegmentContact(float ax, float ay, float bx, float by,
                         float cx, float cy, float dx, float dy, float& contact) {
    if (SegmentContact(ax, ay, bx, by, cx, cy, dx, dy, contact)) return true;
    const float rx=bx-ax, ry=by-ay, sx=dx-cx, sy=dy-cy;
    if (std::abs(rx*sy-ry*sx)>0.00001f || std::abs((cx-ax)*ry-(cy-ay)*rx)>0.00001f) return false;
    const float travel=rx*rx+ry*ry;
    if (travel<0.0000001f) {
        const float length=sx*sx+sy*sy;
        if (length<0.0000001f) return false;
        const float u=((ax-cx)*sx+(ay-cy)*sy)/length;
        if (u<0.f || u>1.f) return false;
        contact=0.f; return true;
    }
    const float a=((cx-ax)*rx+(cy-ay)*ry)/travel;
    const float b=((dx-ax)*rx+(dy-ay)*ry)/travel;
    if (std::max(a,b)<0.f || std::min(a,b)>1.f) return false;
    contact=std::clamp(std::min(a,b),0.f,1.f);
    return true;
}
bool SweptBoxContact(float ax, float ay, float bx, float by,
                     float left, float right, float bottom, float top, float& contact) {
    const float dx=bx-ax, dy=by-ay;
    float enter=0.f, leave=1.f;
    auto slab=[&](float start,float travel,float low,float high) {
        if (std::abs(travel)<0.00001f) return start>low && start<high;
        float a=(low-start)/travel, b=(high-start)/travel;
        if (a>b) std::swap(a,b);
        enter=std::max(enter,a); leave=std::min(leave,b);
        return enter<=leave;
    };
    if (!slab(ax,dx,left,right) || !slab(ay,dy,bottom,top)) return false;
    contact=enter;
    return true;
}
bool BoxContact(float ax, float ay, float bx, float by, const SolidBox& box, float& contact) {
    if (ax > box.left && ax < box.right && ay > box.bottom && ay < box.top) { contact = 0.f; return true; }
    bool found = false;
    auto edge = [&](float cx, float cy, float dx, float dy) {
        float t;
        if (SegmentContact(ax,ay,bx,by,cx,cy,dx,dy,t) && t < contact) { contact=t; found=true; }
    };
    edge(box.left,box.bottom,box.right,box.bottom);
    edge(box.right,box.bottom,box.right,box.top);
    edge(box.right,box.top,box.left,box.top);
    edge(box.left,box.top,box.left,box.bottom);
    return found;
}
// NinjaSpawner and SkeletonEnemySpawner both give NPCs a 100x200 collision box.
// Grounded NPCs use MovingSprite's default gravity (1.0) and their spawner's
// vertical resistance (1.4), scaled from the original 60 Hz target update.
void MoveGroundEnemy(Enemy& enemy, const LevelScene& scene, float desiredX, float dt) {
    constexpr float halfW = 50.f, halfH = 100.f;
    const float oldX = enemy.x, oldY = enemy.y;
    enemy.x = desiredX;
    for (const auto& line : scene.Ground()) {
        if (std::abs(line.x2-line.x1) > 1.f) continue;
        const float wallX = (line.x1+line.x2)*0.5f;
        if (oldY+halfH <= std::min(line.y1,line.y2) || oldY-halfH >= std::max(line.y1,line.y2)) continue;
        if (desiredX > oldX && oldX+halfW <= wallX && enemy.x+halfW > wallX) enemy.x=std::min(enemy.x,wallX-halfW);
        if (desiredX < oldX && oldX-halfW >= wallX && enemy.x-halfW < wallX) enemy.x=std::max(enemy.x,wallX+halfW);
    }
    for (const auto& box : scene.SolidBoxes()) {
        if (oldY+halfH <= box.bottom || oldY-halfH >= box.top) continue;
        if (desiredX > oldX && oldX+halfW <= box.left && enemy.x+halfW > box.left) enemy.x=std::min(enemy.x,box.left-halfW);
        if (desiredX < oldX && oldX-halfW >= box.right && enemy.x-halfW < box.right) enemy.x=std::max(enemy.x,box.right+halfW);
    }
    constexpr float gravity = (1.f/1.4f)*60.f*60.f;
    if (!enemy.grounded) {
        const float acceleration = enemy.vy < 0 && enemy.vy > -15.f*60.f ? gravity*3.f : gravity;
        enemy.y += enemy.vy*dt - 0.5f*acceleration*dt*dt;
        enemy.vy = std::max(-34.f*60.f, enemy.vy-acceleration*dt);
    }
    const bool wasGrounded = enemy.grounded;
    enemy.grounded = false;
    float floor = -1e9f;
    for (const auto& line : scene.Ground()) {
        const float dx = line.x2-line.x1;
        if (std::abs(dx) <= 1.f) continue;
        const float t=(enemy.x-line.x1)/dx;
        if (t < 0.f || t > 1.f) continue;
        const float surface=line.y1+t*(line.y2-line.y1);
        if ((wasGrounded && std::abs(surface-(oldY-halfH)) <= 20.f) ||
            (enemy.vy <= 0.f && oldY-halfH >= surface-4.f && enemy.y-halfH <= surface)) floor=std::max(floor,surface);
    }
    for (const auto& box : scene.SolidBoxes()) {
        if (enemy.x+halfW <= box.left || enemy.x-halfW >= box.right) continue;
        if ((wasGrounded && std::abs(box.top-(oldY-halfH)) <= 20.f) ||
            (enemy.vy <= 0.f && oldY-halfH >= box.top-4.f && enemy.y-halfH <= box.top)) floor=std::max(floor,box.top);
        if (enemy.vy > 0.f && oldY+halfH <= box.bottom && enemy.y+halfH > box.bottom) {
            enemy.y=box.bottom-halfH; enemy.vy=0.f;
        }
    }
    if (floor > -1e8f) { enemy.y=floor+halfH; enemy.vy=0.f; enemy.grounded=true; }
}
}
void Combat::SetArt(render::Renderer& renderer) {
    std::string error;
    ninjaArt_ = renderer.LoadTexture("Game/Media/characters/enemy_1/idle_default/idle_default_000.png", error);
    ghostArt_ = renderer.LoadTexture("Game/Media/characters/banshee.png", error);
    skeletonArt_ = renderer.LoadTexture("Game/Media/characters/skeleton/idle_default/idle_default_000.png", error);
    if (!skeletonArt_) skeletonArt_ = ninjaArt_; // source skeleton PNGs are absent from this checkout
    knifeArt_ = renderer.LoadTexture("Game/Media/knife.png", error);
    upgradedKnifeArt_ = renderer.LoadTexture("Game/Media/knife_2.png", error);
    ninjaShotArt_ = renderer.LoadTexture("Game/Media/ninjastar.png", error);
    ghostShotArt_ = renderer.LoadTexture("Game/Media/puke_temp.png", error);
    skeletonShotArt_ = renderer.LoadTexture("Game/Media/fireball.png", error);
    if (!skeletonShotArt_) skeletonShotArt_ = ghostShotArt_; // source fireball PNG is absent
    constexpr const char* deathPaths[] = {
        "Game/Media/characters/enemy_1/exploded/ricehat.png",
        "Game/Media/characters/enemy_1/exploded/head.png",
        "Game/Media/characters/enemy_1/exploded/forearm_back.png",
        "Game/Media/characters/enemy_1/exploded/hand_front.png",
        "Game/Media/orb.png", "Game/Media/orb2.png", "Game/Media/orb3.png"};
    for (size_t i = 0; i < 7; ++i) ninjaDeathArt_[i] = renderer.LoadTexture(deathPaths[i], error);
    explosionLinesArt_ = renderer.LoadTexture("Game/Media/explosion_lines.png", error);
    bloodParticleArt_ = renderer.LoadTexture("Game/Media/bloodparticle.png", error);
    bloodProjectileArt_ = renderer.LoadTexture("Game/Media/blood_projectile.png", error);
    bloodImpactArt_[0] = renderer.LoadTexture("Game/Media/blood_impact.png", error);
    bloodImpactArt_[1] = renderer.LoadTexture("Game/Media/blood_impact_2.png", error);
    bloodMaterialPierceable_.clear();
    TiXmlDocument materials("Game/XmlFiles/materials.xml");
    if (materials.LoadFile() && materials.RootElement())
        for (auto* material = materials.RootElement()->FirstChildElement("material"); material;
             material = material->NextSiblingElement("material")) {
            const char* name = material->Attribute("name");
            auto* flag = material->FirstChildElement("is_pierceable");
            const char* value = flag ? flag->Attribute("value") : nullptr;
            if (name && value) bloodMaterialPierceable_[name] = std::string(value) == "true";
        }
    auto load = [&](const char* xml, VisualSet& visual) {
        TiXmlDocument document(xml);
        if (!document.LoadFile()) return;
        auto* body = document.RootElement() ? document.RootElement()->FirstChildElement("body") : nullptr;
        for (auto* sequence = body ? body->FirstChildElement() : nullptr; sequence; sequence = sequence->NextSiblingElement()) {
            const std::string name = sequence->Value();
            if (name != "Still" && name != "Running" && name != "JumpingUp" &&
                name != "JumpingDown" && name != "Roll" && name != "Crouch" &&
                name != "ImpactLandSoft" && name != "ImpactLandHard") continue;
            VisualSequence frames;
            sequence->QueryFloatAttribute("FrameRate", &frames.rate);
            frames.loop = name == "Still" || name == "Running";
            for (auto* frame = sequence->FirstChildElement("frame"); frame; frame = frame->NextSiblingElement("frame")) {
                const char* filename = frame->Attribute("filename");
                if (!filename) continue;
                std::string path(filename);
                std::replace(path.begin(), path.end(), '\\', '/');
                if (path.empty() || path.find("..") != std::string::npos || path.front() == '/') continue;
                path = "Game/" + path;
                if (!std::filesystem::exists(path)) continue;
                const auto texture = renderer.LoadTexture(path, error);
                if (texture) frames.frames.push_back(texture);
            }
            if (!frames.frames.empty()) visual.emplace(name, std::move(frames));
        }
    };
    load("Game/XmlFiles/animation/ninjaAnimation.xml", ninjaVisuals_);
    load("Game/XmlFiles/animation/ghost_enemy_anim.xml", ghostVisuals_);
    load("Game/XmlFiles/animation/skeleton_animation.xml", skeletonVisuals_);
}
size_t Combat::DeathDebrisCount() const {
    return std::count_if(deathEffects_.begin(), deathEffects_.end(),
                         [](const auto& effect) { return !effect.spray; });
}
size_t Combat::DeathSprayCount() const {
    return std::count_if(deathEffects_.begin(), deathEffects_.end(),
                         [](const auto& effect) { return effect.spray; });
}
void Combat::SpawnNinjaDeath(float x, float y) {
    // NPC::DoNinjaExplosion supplies this exact texture/multiplier sequence.
    // Debris random speeds are 5..24 source units/tick, direction (±1,.5).
    constexpr int art[] = {0,1,2,2,3,4,5,5,5,6};
    constexpr float multiplier[] = {1.3f,.8f,.95f,1.1f,.65f,1.25f,1.2f,.94f,.79f,1.12f};
    constexpr float width[] = {74,58,59,59,32,64,32,32,32,64};
    constexpr float height[] = {35,75,30,30,32,64,32,32,32,64};
    for (int i = 0; i < 10; ++i) {
        const float speed = (5.f + static_cast<float>((i*13+7)%20))*multiplier[i]*60.f;
        const float sign = i%2 ? -1.f : 1.f;
        deathEffects_.push_back({ninjaDeathArt_[art[i]], x, y+50.f, sign*speed,
                                 .5f*speed, width[i], height[i], 0.f, 4.2f, 0.f, false});
    }
    // Five source explosion_lines particles; deterministic samples within the
    // authored .3-.7 s lifetime and 32..128 size envelope.
    for (int i = 0; i < 5; ++i) {
        const float size = 40.f+20.f*i;
        const float spread = static_cast<float>(i-2)*.025f;
        deathEffects_.push_back({explosionLinesArt_, x, y, (spread+.04f)*60.f*4.f,
                                 -spread*60.f*4.f, size, size, 0.f, .3f+.1f*i, 0.f, true});
    }
}
void Combat::SpawnNinjaMaterialSpray(float x, float y) {
    // Character's common gruesome branch runs before NPC::DoNinjaExplosion.
    // Bottom() uses the NinjaSpawner visual height, not its collision box.
    ParticleSpray spray;
    spray.texture = bloodParticleArt_;
    spray.x = x;
    spray.y = y - 231.714279f * .5f + 30.f;
    spray.width = 3200.f;
    spray.height = 2000.f;
    spray.depth = 3000; // GameObject::kPlayer
    spray.looping = false;
    spray.warmed = true; // Runtime emission does not take authored-load Warm().
    spray.fadeIn = .15f;
    spray.fadeOut = .8f;
    spray.scalesByLifetime = true;
    spray.scaleTo = 8.f;
    spray.randomState = 0x4e494e4au ^ static_cast<std::uint32_t>(kills_);
    spray.particles.reserve(20);
    for (int i = 0; i < 20; ++i) {
        ParticleSpray::Particle particle;
        const float spread = static_cast<float>(spray.Random(30)) * .01f;
        particle.directionX = .2f + spread;
        particle.directionY = .8f - spread;
        const int offsetX = static_cast<int>(spray.Random(51));
        const int signX = spray.Random(2) ? -1 : 1;
        const int offsetY = static_cast<int>(spray.Random(101));
        const int signY = spray.Random(2) ? -1 : 1;
        particle.x = particle.startX = spray.x + signX * offsetX;
        particle.y = particle.startY = spray.y + signY * offsetY;
        particle.drawX = particle.x;
        particle.drawY = particle.y;
        particle.size = particle.startSize = 11.f + static_cast<float>(spray.Random(30));
        particle.drawSize = particle.size;
        particle.speed = particle.startSpeed = 7.01f + static_cast<float>(spray.Random(500)) * .01f;
        particle.gravity = 1.4f;
        particle.brightness = .76f + static_cast<float>(spray.Random(25)) * .01f;
        // Update may spawn this spray after the emitter-update pass. Start at
        // fade-in time zero so that frame cannot render fully opaque.
        particle.alpha = 0.f;
        particle.lifetime = 1.5f - .035f * static_cast<float>(i);
        spray.particles.push_back(particle);
    }
    materialSprays_.push_back(std::move(spray));
}
void Combat::SpawnKnifeLineSpray(const LevelScene& scene, size_t segment, const Shot& shot,
                                float rayEndX, float rayEndY) {
    const auto texture = scene.GroundParticleTexture(scene.Ground()[segment].material);
    if (!texture) return;
    ParticleSpray spray;
    spray.texture = texture;
    // SolidLineStrip::GetProjectileCollisionData returns line.Position() -
    // projectile.GetCollisionRayEnd(); Projectile subtracts that result from
    // line.Position(), so its emitter origin is the *ray end*, not intersection.
    // The port retains only segment endpoints and center-sweep contact. Map the
    // source ray end to the untruncated new center + half blade width along
    // travel; contact timing/geometry is therefore not exact Windows parity.
    spray.x = rayEndX;
    spray.y = rayEndY;
    spray.width = 3200.f;
    spray.height = 1200.f;
    spray.depth = shot.depth;
    spray.looping = false;
    spray.loopTime = 10.f;
    spray.warmed = true;
    spray.fadeIn = .15f;
    spray.fadeOut = .6f;
    spray.scalesByLifetime = true;
    spray.scaleTo = 3.5f;
    spray.randomState = 0x4b4e4946u ^ static_cast<std::uint32_t>(fired_);
    // Source m_direction is launch direction; gravity changes velocity, not
    // direction. Reverse that launch direction for the contact spray.
    const float directionX = -shot.launchDirectionX;
    const float directionY = -shot.launchDirectionY;
    spray.particles.reserve(10);
    for (int i = 0; i < 10; ++i) {
        ParticleSpray::Particle particle;
        const float spread = static_cast<float>(spray.Random(40)) * .01f;
        particle.directionX = directionX + spread;
        particle.directionY = directionY - spread;
        const int offsetX = static_cast<int>(spray.Random(16));
        const int signX = spray.Random(2) ? -1 : 1;
        const int offsetY = static_cast<int>(spray.Random(31));
        const int signY = spray.Random(2) ? -1 : 1;
        particle.x = particle.startX = spray.x + signX * offsetX;
        particle.y = particle.startY = spray.y + signY * offsetY;
        particle.drawX = particle.x; particle.drawY = particle.y;
        particle.lifetime = .75f - .04f * static_cast<float>(i);
        particle.size = particle.startSize = 11.f + static_cast<float>(spray.Random(20));
        particle.drawSize = particle.size;
        particle.speed = particle.startSpeed = 1.01f + static_cast<float>(spray.Random(300)) * .01f;
        particle.gravity = .7f;
        particle.brightness = .71f + static_cast<float>(spray.Random(30)) * .01f;
        particle.alpha = 0.f;
        spray.particles.push_back(particle);
    }
    materialSprays_.push_back(std::move(spray));
}
bool Combat::DamageEnemy(size_t index, float damage, float projectileDirectionX) {
    if (index >= enemies_.size() || enemies_[index].health <= 0.f ||
        enemies_[index].teleportTime > 0.f || damage <= 0.f) return false;
    auto& enemy = enemies_[index];
    enemy.health -= damage;
    enemy.damageBurstAge = 0.f;
    // Character::OnDamage applies 30 world units of X kickback only when a
    // projectile damages a non-player character; direct/melee hits do not.
    if (projectileDirectionX != 0.f) enemy.x += std::copysign(30.f,projectileDirectionX);
    ++hits_;
    // NPC::OnDamage counts consecutive hits while the 0.5 s window is live.
    enemy.comboWindow=0.5f;
    ++enemy.comboHits;
    if (enemy.stunTime <= 0.f && enemy.grounded && enemy.comboHits >= 6) {
        enemy.stunTime=2.f;
        enemy.windup=0.f;
    }
    if (enemy.health <= 0.f) {
        ++kills_;
        if (enemy.type == "kNinja") {
            SpawnNinjaMaterialSpray(enemy.x, enemy.y);
            SpawnNinjaDeath(enemy.x, enemy.y);
        }
    }
    return true;
}
void Combat::ApplyPlayerDamage(float damage) {
    if (damage <= 0.f || health_ <= 0.f) return;
    health_ = std::max(0.f, health_ - damage);
    playerDamageBurstAge_ = 0.f;
}
void Combat::SpawnPlayerBloodFlight(float x, float y) {
    SpawnBloodFlight(x, y, 0.f, 0.f);
}
void Combat::SpawnIntroBloodFlight(float x, float y) {
    SpawnBloodFlight(x, y, -1.6f, 1.4f);
}
void Combat::SpawnBloodFlight(float x, float y, float directionX, float directionY) {
    // Character::FireBloodSpatter: one blood projectile. Linux keeps a
    // private deterministic RNG; Windows uses the process-global rand().
    bloodRandomState_ = bloodRandomState_ * 1664525u + 1013904223u;
    const float size = 256.f * (.5f + static_cast<float>(bloodRandomState_ % 50u) * .01f);
    bloodRandomState_ = bloodRandomState_ * 1664525u + 1013904223u;
    const unsigned variant = bloodRandomState_ % 2u;
    BloodFlight blood{};
    blood.x=x; blood.y=y; blood.vx=directionX*3.5f; blood.vy=directionY*3.5f;
    blood.launchDirectionX=directionX; blood.launchDirectionY=directionY;
    blood.size=size; blood.variant=variant;
    bloodFlights_.push_back(blood);
}
void Combat::EnableSimulatedWallClock() {
    simulatedWallClock_ = true;
    simulatedClockOriginMs_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    simulatedClockElapsedMs_ = 0;
}
void Combat::UpdateBloodFlights(const LevelScene& scene, float dt) {
    // Blood collision is independent of knife's enemy-dependent line skip.
    for (auto& blood : bloodFlights_) {
        if (blood.impacted || blood.blocked) {
            blood.inactiveAge += std::max(0.f, dt);
            if (blood.blocked) {
                // Projectile::Update's inactive branch keeps moving and adds a
                // fixed 0.09-degree spin and -1.15 Y per update, not per tick.
                const float ticks = std::min(2.f, std::max(0.f, dt) * 60.f);
                blood.x += blood.vx * ticks;
                blood.y += (blood.vy - 1.15f) * ticks;
                blood.rotation += .09f;
            }
            continue;
        }
        const float ticks = std::min(2.f, std::max(0.f, dt) * 60.f);
        const float oldX = blood.x, oldY = blood.y;
        blood.vy -= .5f * ticks;
        const float nextX = blood.x + blood.vx * ticks;
        const float nextY = blood.y + blood.vy * ticks;
        const bool directed = blood.launchDirectionX != 0.f || blood.launchDirectionY != 0.f;
        // The source ray extends from the previous center to the new center
        // plus half the 64-unit collision width along immutable direction.
        const float rayEndX = nextX + (directed ? blood.launchDirectionX * 32.f : 0.f);
        const float rayEndY = nextY + (directed ? blood.launchDirectionY * 32.f : 0.f);
        float first = 2.f;
        size_t winner = scene.Ground().size();
        for (size_t i = 0; i < scene.Ground().size(); ++i) {
            const auto& line = scene.Ground()[i];
            float t;
            if (BloodSegmentContact(oldX, oldY, rayEndX, rayEndY,
                                    line.x1, line.y1, line.x2, line.y2, t) && t < first) {
                first = t; winner = i;
            }
        }
        if (winner < scene.Ground().size()) {
            // The directed source projectile has already advanced to its full
            // next center before line collision. Preserve the ordinary
            // zero-direction center-sweep behavior separately.
            blood.x = directed ? nextX : oldX + (nextX-oldX)*first;
            blood.y = directed ? nextY : oldY + (nextY-oldY)*first;
            blood.vy = 0.f;
            blood.inactiveAge = 0.f;
            const auto& line = scene.Ground()[winner];
            if (!line.material.empty()) {
                const auto material = bloodMaterialPierceable_.find(line.material);
                if (material == bloodMaterialPierceable_.end() || !material->second) {
                    // SolidLineStrip builds the normal from authored point order.
                    // GroundSegment preserves that order, so recover it here.
                    const float dx = line.x2-line.x1, dy = line.y2-line.y1;
                    const float length = std::hypot(dx, dy);
                    if (length > 0.f) {
                        float directionX = -dy/length, directionY = dx/length;
                        // Source adds separate integer X/Y offsets before
                        // normalizing, then scales X by 2 and Y by 6.
                        bloodRandomState_ = bloodRandomState_ * 1664525u + 1013904223u;
                        const int offsetY = static_cast<int>(bloodRandomState_ % 20u);
                        bloodRandomState_ = bloodRandomState_ * 1664525u + 1013904223u;
                        int offsetX = static_cast<int>(bloodRandomState_ % 20u);
                        bloodRandomState_ = bloodRandomState_ * 1664525u + 1013904223u;
                        if (bloodRandomState_ % 2u == 0) offsetX = -offsetX;
                        directionX += static_cast<float>(offsetX);
                        directionY += static_cast<float>(offsetY);
                        const float magnitude = std::hypot(directionX, directionY);
                        if (magnitude > 0.f) {
                            blood.vx = directionX/magnitude * 2.f;
                            blood.vy = directionY/magnitude * 6.f;
                        }
                    }
                    // The same material-directed spray is emitted for either
                    // pierceability branch; its origin is the source ray end.
                    Shot emitter{};
                    emitter.depth = 3000;
                    emitter.launchDirectionX = blood.launchDirectionX;
                    emitter.launchDirectionY = blood.launchDirectionY;
                    SpawnKnifeLineSpray(scene, winner, emitter, directed ? rayEndX : blood.x,
                                        directed ? rayEndY : nextY-32.f);
                    blood.blocked = true;
                    blood.age += std::max(0.f, dt);
                    continue;
                }
            }
            blood.impacted = true;
            bloodRandomState_ = bloodRandomState_ * 1664525u + 1013904223u;
            const float jitter = static_cast<float>(bloodRandomState_ % 100u) * .2f;
            bloodRandomState_ = bloodRandomState_ * 1664525u + 1013904223u;
            blood.y += (bloodRandomState_ & 1u ? -jitter : jitter) - 20.f;
            if (!scene.Ground()[winner].material.empty()) {
                Shot emitter{};
                emitter.depth = 3000; // material spray precedes ground-blood depth
                emitter.launchDirectionX = blood.launchDirectionX;
                emitter.launchDirectionY = blood.launchDirectionY;
                SpawnKnifeLineSpray(scene, winner, emitter, directed ? rayEndX : blood.x,
                                    directed ? rayEndY : nextY);
            }
        } else { blood.x = nextX; blood.y = nextY; }
        blood.age += std::max(0.f, dt);
    }
    // Port-only orphan bound; source has no active-flight TTL.
    bloodFlights_.erase(std::remove_if(bloodFlights_.begin(), bloodFlights_.end(),
        [](const BloodFlight& blood) { return blood.blocked ? blood.inactiveAge > .25f : blood.impacted ? blood.inactiveAge > 5.5f : blood.age >= 10.f; }), bloodFlights_.end());
    for (auto& spray : materialSprays_) spray.Update(dt);
    materialSprays_.erase(std::remove_if(materialSprays_.begin(), materialSprays_.end(),
        [](const ParticleSpray& spray) {
            return std::all_of(spray.particles.begin(), spray.particles.end(),
                               [](const ParticleSpray::Particle& p) { return p.dead; });
        }), materialSprays_.end());
}
void Combat::Update(const LevelScene& scene, float x, float y, float dt, bool fire, bool melee, bool grounded, bool rolling, int facing, render::Renderer& renderer, float aimX, float aimY, PlayerShotLaunchState launchState) {
    UpdateBloodFlights(scene, dt);
    for (auto& effect : deathEffects_) {
        effect.age += std::max(0.f, dt);
        effect.x += effect.vx*dt;
        effect.y += effect.vy*dt;
        if (!effect.spray) {
            effect.vy -= .25f*60.f*60.f*dt;
            effect.vx *= std::pow(.99f, dt*60.f);
            effect.vy *= std::pow(.95f, dt*60.f);
            effect.rotation -= effect.vx*.02f*dt;
        }
    }
    deathEffects_.erase(std::remove_if(deathEffects_.begin(), deathEffects_.end(),
        [](const auto& effect) { return effect.age >= effect.lifetime; }), deathEffects_.end());
    playerDamageBurstAge_ = std::min(0.25f, playerDamageBurstAge_ + std::max(0.f, dt));
    for (auto& enemy : enemies_)
        enemy.damageBurstAge = std::min(0.25f, enemy.damageBurstAge + std::max(0.f, dt));
    if (simulatedWallClock_) simulatedClockElapsedMs_ += static_cast<double>(dt) * 1000.0;
    fireCooldown_ = std::max(0.f, fireCooldown_-dt);
    burstCooldown_ = std::max(0.f, burstCooldown_-dt);
    hurtCooldown_ = std::max(0.f, hurtCooldown_-dt);
    bool inWater = false;
    for (const auto& hazard : scene.Hazards())
        if (hazard.water && Inside(x,y,hazard.x,hazard.y,hazard.width,hazard.height)) inWater = true;
    // Player::DoMeleeAttack requires solid footing, 4 focus and dry ground.
    // The four sequences in player_anim_default.xml contain 7/15/9/7 frames
    // at 60 FPS. Character::DoMeleeCollisions damages only in phase 3.
    const bool meleePressed = melee && !meleeHeld_;
    meleeHeld_ = melee;
    if (meleePressed && meleeTime_ <= 0.f && grounded && !rolling && !inWater && focus_ >= 4.f && health_ > 0.f) {
        meleeTime_ = 0.000001f;
        focus_ -= 4.f;
    }
    const bool meleeStrikePhase = meleeTime_ >= 22.f/60.f && meleeTime_ < 31.f/60.f;
    for (const auto& trigger : scene.Encounters()) {
        if (!Inside(x,y,trigger.x,trigger.y,trigger.width,trigger.height) || !activeTriggers_.insert(trigger.id).second) continue;
        // NPCTrigger::Initialise removes a trigger if it last fired less than
        // 120000 ms ago. Keep wall-clock history across levels and saves.
        const auto nowMs = simulatedWallClock_ ?
            simulatedClockOriginMs_ + static_cast<std::int64_t>(std::llround(simulatedClockElapsedMs_)) :
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        const auto previous = triggerHistory_.find(trigger.id);
        const auto elapsedMs = previous == triggerHistory_.end() ? -1 : nowMs - previous->second;
        const bool onCooldown = previous != triggerHistory_.end() && elapsedMs < 120000;
        const bool traceEncounter = std::getenv("DEATHLEAK_TRACE_ENCOUNTERS") != nullptr;
        if (traceEncounter) std::cout << "Encounter trigger id=" << trigger.id << " elapsed_ms=" << elapsedMs
            << " cooldown=" << onCooldown
            << " count=" << trigger.count << " type=" << trigger.type << std::endl;
        if (onCooldown) continue;
        triggerHistory_[trigger.id] = nowMs;
        if (!ninjaArt_) SetArt(renderer);
        for (int i=0; i<std::clamp(trigger.count,0,20); ++i) {
            // Original spawners use a 1200x1200 box around the player. Fixed offsets make scripted runs reproducible.
            float offset = (i%2 ? -1.f : 1.f) * (150.f + 85.f*(i/2));
            auto art = trigger.type == "kGhost" ? ghostArt_ : trigger.type == "kSkeleton" ? skeletonArt_ : ninjaArt_;
            // Source SpawnMultiple uses x within +/-600, y between +300 and +600.
            const float spawnY = y + 300.f + 60.f*(i%5);
            enemies_.push_back({x+offset, spawnY, trigger.type == "kGhost" ? 50.f : trigger.type == "kSkeleton" ? 500.f : 20.f,
                1.3f+0.1f*i, trigger.type, art, 0.f, x + (offset < 0 ? -250.f : 250.f), y+300.f});
            enemies_.back().lastVisualX = enemies_.back().x;
            ++spawned_;
        }
    }
    // Player::FireWeapon blocks shots in water and fires at most five knives
    // before its 0.5-second burst pause. Source's 0.25 s is an inactive,
    // post-contact retirement interval, not an active-flight lifetime.
    if (fire && !rolling && meleeTime_ <= 0.f && fireCooldown_ <= 0 && burstCooldown_ <= 0 && !inWater && health_ > 0) {
        if (!knifeArt_) SetArt(renderer);
        // The port removes on contact. Ten seconds only bounds an unimpeded
        // offscreen knife in this simplified collection; it is not source TTL.
        // The active stick supplies a unit aim vector; legacy callers and
        // keyboard-only input retain the horizontal facing fallback.
        const float aimLength = std::hypot(aimX, aimY);
        const float knifeX = aimLength > 0.f ? aimX / aimLength : static_cast<float>(facing);
        const float knifeY = aimLength > 0.f ? aimY / aimLength : 0.f;
        // Player::FireWeapon starts at +25, removes 25 when crouched, then
        // applies the Character air flag (up takes precedence over down).
        const float launchY = y + (launchState.crouching ? 0.f : 25.f) +
            (launchState.ascending ? 65.f : launchState.descending ? 40.f : 0.f);
        shots_.push_back({x+(knifeX > 0.f ? 10.f : -10.f),launchY,knifeX*1200.f,knifeY*1200.f,10.f,projectileDamageUpgrade_ ? 4.f : 1.5f,false});
        shots_.back().texture = projectileDamageUpgrade_ ? upgradedKnifeArt_ : knifeArt_;
        shots_.back().playerKnife = true;
        shots_.back().launchDirectionX = knifeX;
        shots_.back().launchDirectionY = knifeY;
        fireCooldown_ = projectileSpeedUpgrade_ ? 0.220f*0.75f : 0.220f;
        ++fired_;
        if (!projectileSpeedUpgrade_ && ++burstCount_ == 5) { burstCount_ = 0; burstCooldown_ = 0.5f; }
    }
    for (auto& enemy : enemies_) {
        if (enemy.health <= 0) continue;
        enemy.meleeStrikeCooldown = std::max(0.f, enemy.meleeStrikeCooldown-dt);
        // NPC::Update expires the rolling damage combo after 0.5 s; Character
        // counts down stun while the range-attack AI remains inactive.
        enemy.comboWindow = std::max(0.f, enemy.comboWindow-dt);
        if (enemy.comboWindow == 0.f) enemy.comboHits = 0;
        enemy.stunTime = std::max(0.f, enemy.stunTime-dt);
        const float dx=x-enemy.x, dy=y-enemy.y;
        if (std::abs(dx) > 1.f) enemy.facing = dx > 0.f ? 1 : -1;
        enemy.fireCooldown = std::max(0.f, enemy.fireCooldown-dt);
        const bool ghost = enemy.type == "kGhost";
        if (enemy.teleportTime > 0.f) {
            enemy.teleportTime = std::max(0.f,enemy.teleportTime-dt);
            if (enemy.teleportTime > 0.f) continue;
            enemy.x=enemy.teleportX;
            enemy.y=enemy.teleportY;
            enemy.vy=0.f;
            enemy.grounded=false;
            ++enemy.teleports;
        }
        if (enemy.rollTime > 0.f) {
            enemy.rollTime = std::max(0.f, enemy.rollTime-dt);
            // Character::Roll starts at 12 units/tick and suspends ranged AI.
            MoveGroundEnemy(enemy,scene,enemy.x+std::copysign(720.f*dt,enemy.targetX-enemy.x),dt);
            continue;
        }
        if (enemy.stunTime > 0.f) {
            enemy.windup = 0.f;
            if (!ghost) MoveGroundEnemy(enemy,scene,enemy.x,dt);
            continue;
        }
        if (ghost) {
            // AIStateFlying: seek 250-400 above the player, holding a 150-400 X offset.
            const float side = enemy.targetX < x ? -1.f : 1.f;
            enemy.targetX = x + side*250.f;
            enemy.targetY = y + 300.f;
            if (enemy.x < x-250.f) enemy.x += std::min(x-250.f-enemy.x,720.f*dt);
            else if (enemy.x > x+250.f) enemy.x -= std::min(enemy.x-x-250.f,720.f*dt);
            enemy.y += std::clamp(enemy.targetY-enemy.y,-210.f*dt,210.f*dt);
        } else {
            // AIStateRangeAttack teleports a distant NPC after 8-12 s;
            // SkeletonEnemy::CanTeleport is false. Fixed in-range values replace rand().
            enemy.teleportCooldown=std::max(0.f,enemy.teleportCooldown-dt);
            if (enemy.type != "kSkeleton" && std::abs(dx)>600.f && enemy.teleportCooldown<=0.f) {
                enemy.teleportX=x-facing*550.f;
                enemy.teleportY=y+250.f;
                enemy.teleportTime=0.8f; // Character::kTeleportTime
                enemy.teleportCooldown=8.f;
                enemy.windup=0.f;
                enemy.vy=0.f;
                continue;
            }
            // AIStateRangeAttack follows beyond 500, retreats inside 200, and holds the band.
            const float range = std::abs(dx);
            const float speed = enemy.type == "kSkeleton" ? 240.f : 480.f;
            float desiredX=enemy.x;
            if (range > 500.f) desiredX += std::copysign(std::min(range-500.f,speed*dt),dx);
            else if (range < 200.f && range > 0.01f) desiredX -= std::copysign(std::min(200.f-range,speed*dt),dx);
            // AIStateRangeAttack chooses a jump if the player is >50 above,
            // otherwise a roll, only while grounded and accelerating.
            // Use the source delay's 2 s lower bound and a fixed 18-unit/tick
            // ninja jump (NinjaSpawner randomizes 16..20) for repeatability.
            enemy.jumpRollDelay = std::max(0.f,enemy.jumpRollDelay-dt);
            if (enemy.type != "kSkeleton" && enemy.grounded &&
                desiredX-enemy.x > 0.01f && enemy.jumpRollDelay <= 0.f) {
                if (y > enemy.y+50.f) {
                    enemy.vy=18.f*60.f;
                    enemy.grounded=false;
                    ++enemy.jumps;
                } else {
                    enemy.targetX=enemy.x+10000.f;
                    enemy.rollTime=0.5f; // bounded stand-in for Roll animation duration
                    ++enemy.rolls;
                }
                enemy.jumpRollDelay=2.f;
            }
            MoveGroundEnemy(enemy,scene,desiredX,dt);
            if (enemy.rollTime > 0.f) continue;
        }
        if (ghost) {
            // AIStateFlying fires only when the player is below and horizontally near.
            if (enemy.fireCooldown > 0 || y >= enemy.y || std::abs(x-enemy.x) > 45.f) continue;
        } else {
            if (enemy.windup > 0) {
                enemy.windup = std::max(0.f,enemy.windup-dt);
                if (enemy.windup > 0) continue;
            } else {
                if (enemy.fireCooldown > 0 || std::abs(dx)>700.f || std::abs(dy)>500.f) continue;
                enemy.windup=0.25f; // AIStateRangeAttack fire warmup
                continue;
            }
        }
        const float aimX=x-enemy.x, aimY=y-enemy.y;
        const float length=std::max(1.f,std::hypot(aimX,aimY));
        const bool skeleton = enemy.type == "kSkeleton";
        const float speed = skeleton ? 300.f : ghost ? 510.f : 540.f;
        shots_.push_back({enemy.x,enemy.y+(skeleton?50.f:0.f),aimX/length*speed,aimY/length*speed,
            ghost || skeleton ? 2.f : 0.25f, skeleton ? 35.f : ghost ? 2.f : 3.f,true,
            ghost ? 0.9f : 1.f, !skeleton});
        shots_.back().depth = ghost || skeleton ? 2530 : 2510;
        shots_.back().deflectable=!ghost && !skeleton;
        shots_.back().baseSpeed=speed;
        shots_.back().texture = skeleton ? skeletonShotArt_ : ghost ? ghostShotArt_ : ninjaShotArt_;
        shots_.back().visualWidth = ghost ? 40.f : 25.f;
        shots_.back().visualHeight = ghost ? 25.f : skeleton ? 8.f : 13.f;
        enemy.fireCooldown=1.3f; // fixed within NPC's original 1.0..1.6 s randomized reload
    }
    // CollisionManager dispatches each strict-AABB character contact in both
    // directions. Character::OnCollision deliberately skips SolidMovingSprite
    // displacement for another Character, so contact alone must not move,
    // ground, knock back, or damage either participant. Record the pair for
    // deterministic diagnostics and the melee callback below.
    playerContacts_ = 0;
    for (size_t index = 0; index < enemies_.size(); ++index) {
        auto& enemy = enemies_[index];
        if (enemy.health <= 0.f || enemy.teleportTime > 0.f) continue;
        const float playerHalfW = (meleeStrikePhase ? 1.5f : 1.f)*128.959793f*0.5f;
        const float playerHalfH = (meleeTime_ > 0.f ? 1.1201479f : 1.f)*214.898743f*0.5f;
        const bool overlap = std::abs(x-enemy.x) < playerHalfW+50.f &&
                             std::abs(y-enemy.y) < playerHalfH+100.f;
        if (!overlap) continue;
        ++playerContacts_;
        // Character::OnCollision calls DoMeleeCollisions on each overlapping
        // damageable NPC. Its edge check rejects targets behind the player;
        // SolidMovingSprite allows one melee strike per target per 0.5 s.
        if (meleeStrikePhase && enemy.meleeStrikeCooldown <= 0.f &&
            ((facing > 0 && enemy.x-50.f > x-playerHalfW) ||
             (facing < 0 && enemy.x+50.f < x+playerHalfW))) {
            if (DamageEnemy(index,500.f)) {
                enemy.meleeStrikeCooldown = 0.5f;
                ++meleeStrikes_;
            }
        }
    }
    if (playerContacts_ > 0) ++contactFrames_;
    if (meleeTime_ > 0.f) {
        meleeTime_ += dt;
        if (meleeTime_ >= 38.f/60.f) meleeTime_ = 0.f;
    }
    for (auto& shot : shots_) {
        if (shot.life<=0) continue;
        shot.meleeCooldown=std::max(0.f,shot.meleeCooldown-dt);
        const float oldX=shot.x, oldY=shot.y;
        // Projectile::Update uses 60-Hz-equivalent gravity and displacement resistance.
        const float ticks=std::min(2.f,dt*60.f);
        if (shot.gravity) shot.vy -= (shot.enemy ? (shot.resistance < 1.f ? 0.5f : 0.01f) : (projectileSpeedUpgrade_ ? 0.1f : 0.25f))*60.f*ticks;
        const float newX=shot.x+shot.vx*dt*shot.resistance;
        const float newY=shot.y+shot.vy*dt*shot.resistance;
        float terrainT=1.f;
        size_t winningLine = scene.Ground().size();
        // Original blades pass line strips while an enemy is alive, but still hit solid boxes.
        if (shot.enemy || enemies_.empty()) for (size_t index = 0; index < scene.Ground().size(); ++index) {
            const auto& line = scene.Ground()[index];
            float t;
            if (SegmentContact(oldX,oldY,newX,newY,line.x1,line.y1,line.x2,line.y2,t) && t < terrainT) {
                terrainT=t;
                winningLine=index;
            }
        }
        for (const auto& box : scene.SolidBoxes())
            if (BoxContact(oldX,oldY,newX,newY,box,terrainT)) winningLine=scene.Ground().size();
        if (shot.playerKnife && !shot.enemy && winningLine < scene.Ground().size() && terrainT < 1.f) {
            const float halfBlade = shot.visualWidth * .5f;
            SpawnKnifeLineSpray(scene, winningLine, shot,
                               newX + (shot.vx < 0.f ? -halfBlade : halfBlade), newY);
        }
        shot.x=oldX+(newX-oldX)*terrainT;
        shot.y=oldY+(newY-oldY)*terrainT;
        shot.life-=dt;
        if (shot.playerKnife) {
            shot.activeVisualAge += dt;
            shot.knifeRotationRadians -= 20.f * dt;
        }
        if (terrainT < 1.f) shot.life=0;
        if (shot.enemy) {
            const float playerHalfW=(meleeStrikePhase?1.5f:1.f)*128.959793f*0.5f;
            const float playerHalfH=(meleeTime_>0.f?1.1201479f:1.f)*214.898743f*0.5f;
            const float shotHalfW=shot.resistance<1.f?20.f:12.5f;
            const float shotHalfH=shot.resistance<1.f?12.5f:(shot.gravity?6.5f:4.f);
            // Original ghost 40x25, ninja star 25x13, skeleton fireball 25x8.
            float contact=0.f;
            if (deflectionUnlocked_ && shot.deflectable && shot.life>0.f &&
                shot.meleeCooldown<=0.f && meleeTime_>0.f &&
                (meleeTime_<7.f/60.f || meleeStrikePhase) &&
                SweptBoxContact(oldX,oldY,shot.x,shot.y,
                                x-playerHalfW-shotHalfW,x+playerHalfW+shotHalfW,
                                y-playerHalfH-shotHalfH,y+playerHalfH+shotHalfH,contact)) {
                const float contactX=oldX+(shot.x-oldX)*contact;
                const bool front=(facing>0 && shot.vx<0.f && contactX-shotHalfW>x-playerHalfW) ||
                    (facing<0 && shot.vx>0.f && contactX+shotHalfW<x+playerHalfW);
                if (front) {
                    const float speed=shot.baseSpeed>0.f?shot.baseSpeed:std::hypot(shot.vx,shot.vy);
                    shot.x=contactX; shot.y=oldY+(shot.y-oldY)*contact;
                    shot.vx=(shot.vx<0.f?1.f:-1.f)*speed*2.5f;
                    shot.vy=0.f; // deterministic representative of source random y offset
                    shot.enemy=false; shot.damage=9999.f; shot.wasDeflected=true;
                    shot.meleeCooldown=0.5f;
                    ++deflections_;
                }
            }
            // Retain the existing prototype hitbox outside successful melee
            // deflection; upgrading all projectile collision is separate work.
            if (shot.enemy && hurtCooldown_<=0.f) {
                const float vx=shot.x-oldX, vy=shot.y-oldY;
                const float t=std::clamp(((x-oldX)*vx+(y-oldY)*vy)/(vx*vx+vy*vy+0.0001f),0.f,1.f);
                if (Inside(oldX+t*vx,oldY+t*vy,x,y,70.f,180.f)) {
                    if (std::getenv("DEATHLEAK_TRACE_ENCOUNTERS"))
                        std::cout << "Encounter projectile hit player=(" << x << ',' << y
                                  << ") shot_from=(" << oldX << ',' << oldY
                                  << ") shot_to=(" << shot.x << ',' << shot.y
                                  << ") damage=" << shot.damage << " health_before=" << health_
                                  << " deflectable=" << shot.deflectable << std::endl;
                    const float healthBefore = health_;
                    ApplyPlayerDamage(shot.damage);
                    if (health_ < healthBefore) {
                        if (!bloodProjectileArt_) SetArt(renderer);
                        SpawnPlayerBloodFlight(x, y);
                    }
                    hurtCooldown_=0.5f; shot.life=0; ++hits_;
                }
            }
        } else for (size_t enemyIndex=0; enemyIndex<enemies_.size(); ++enemyIndex) {
            auto& enemy=enemies_[enemyIndex];
            if (enemy.health<=0 || enemy.teleportTime>0.f) continue;
            // Swept segment avoids tunneling at the knife's 1200 world-units/s speed.
            const float vx=shot.x-oldX, vy=shot.y-oldY;
            const float t=std::clamp(((enemy.x-oldX)*vx+(enemy.y-oldY)*vy)/(vx*vx+vy*vy+0.0001f),0.f,1.f);
            if (Inside(oldX+t*vx,oldY+t*vy,enemy.x,enemy.y,80.f,190.f)) {
                DamageEnemy(enemyIndex,shot.damage,shot.vx); shot.life=0;
                break;
            }
        }
    }
    shots_.erase(std::remove_if(shots_.begin(),shots_.end(),[](const auto& p){return p.life<=0;}),shots_.end());
    enemies_.erase(std::remove_if(enemies_.begin(),enemies_.end(),[](const auto& e){return e.health<=0;}),enemies_.end());
    for (const auto& hazard : scene.Hazards()) {
        if (!Inside(x,y,hazard.x,hazard.y,hazard.width,hazard.height)) continue;
        if (!hazard.water) ApplyPlayerDamage(hazard.damage*dt);
    }
    if (inWater) {
        focus_ = std::max(0.f, focus_-0.5f*dt); // Player::kWaterFocusUseRate
        if (focus_ <= 0) focusCooldown_ = 5.f;
        if (focus_ <= 0) ApplyPlayerDamage(3.f*dt); // drowning follows focus depletion
    } else {
        if (focusCooldown_ > 0) focusCooldown_ = std::max(0.f,focusCooldown_-dt);
        else focus_ = std::min(100.f,focus_+10.f*dt); // Player::kFocusRechargeRate
    }
}
void Combat::UpdateVisuals(float dt) {
    for (auto& enemy : enemies_) {
        const auto& visuals = enemy.type == "kGhost" ? ghostVisuals_ :
            enemy.type == "kSkeleton" ? skeletonVisuals_ : ninjaVisuals_;
        // AI's target and current vertical velocity drive the same broad body
        // categories as Character::UpdateAnimations; exact source state ordering
        // remains outside this bounded renderer slice.
        std::string pose = "Still";
        if (enemy.rollTime > 0.f) pose = "Roll";
        else if (!enemy.grounded && enemy.type != "kGhost") pose = enemy.vy > 0.f ? "JumpingUp" : "JumpingDown";
        else if (enemy.type == "kGhost") pose = "Running";
        else if (enemy.windup > 0.f || enemy.stunTime > 0.f) pose = "Still";
        else if (std::abs(enemy.x-enemy.lastVisualX) > 0.5f) pose = "Running";
        enemy.lastVisualX = enemy.x;
        if (!visuals.count(pose)) pose = "Still";
        const auto found = visuals.find(pose);
        if (found == visuals.end()) continue;
        if (enemy.visualPose != pose) { enemy.visualPose = pose; enemy.visualFrame = 0; enemy.visualElapsed = 0.f; }
        const auto& sequence = found->second;
        if (sequence.rate <= 0.f || sequence.frames.size() < 2) continue;
        enemy.visualElapsed += dt;
        if (enemy.visualElapsed >= 1.f / sequence.rate) {
            enemy.visualElapsed -= 1.f / sequence.rate;
            if (sequence.loop) enemy.visualFrame = (enemy.visualFrame + 1) % sequence.frames.size();
            else if (enemy.visualFrame + 1 < sequence.frames.size()) ++enemy.visualFrame;
        }
    }
}
void Combat::AppendPainterItems(std::vector<PainterItem>& items, render::Renderer& renderer, float cameraX, float cameraY) const {
    constexpr float scale=2.f/3.f;
    for (const auto& spray : materialSprays_) {
        if (!spray.texture) continue;
        const auto* stableSpray = &spray; // Valid until the next Combat mutation.
        items.push_back({spray.depth, [stableSpray, &renderer, cameraX, cameraY] {
            stableSpray->Draw(renderer, cameraX, cameraY);
        }});
    }
    for (const auto& enemy:enemies_) if (enemy.teleportTime<=0.f) {
        const auto& visuals = enemy.type == "kGhost" ? ghostVisuals_ :
            enemy.type == "kSkeleton" ? skeletonVisuals_ : ninjaVisuals_;
        auto selected = visuals.find(enemy.visualPose);
        render::TextureId texture = enemy.texture;
        if (selected != visuals.end() && !selected->second.frames.empty())
            texture = selected->second.frames[std::min(enemy.visualFrame, selected->second.frames.size()-1)];
        if (!texture) continue;
        const float width = enemy.type == "kGhost" ? 113.f : 183.f;
        const float height = enemy.type == "kGhost" ? 208.f : 231.f;
        render::Sprite sprite{texture, {}, {640.f+(enemy.x-cameraX-width/2)*scale,
            360.f+(cameraY-enemy.y-height/2)*scale,width*scale,height*scale}};
        sprite.flipX = enemy.facing < 0;
        sprite.rgbGain = DamageBurstGain(enemy.damageBurstAge);
        items.push_back({3500, [&, sprite] { renderer.DrawSprite(sprite); }});
    }
    for (const auto& effect : deathEffects_) {
        if (!effect.texture) continue;
        const float remaining = effect.lifetime-effect.age;
        const float fade = effect.spray ? std::clamp(remaining/.1f, 0.f, 1.f) :
                           std::clamp(remaining/.2f, 0.f, 1.f);
        const float w = effect.width*fade, h = effect.height*fade;
        render::Sprite sprite{effect.texture, {}, {640.f+(effect.x-cameraX-w/2.f)*scale,
            360.f+(cameraY-effect.y-h/2.f)*scale,w*scale,h*scale}};
        sprite.rotationDegrees = effect.rotation;
        sprite.tint.a = static_cast<std::uint8_t>(std::lround(255.f*fade));
        items.push_back({effect.spray ? 1750 : 3500, [&, sprite] { renderer.DrawSprite(sprite); }});
    }
    for (const auto& blood : bloodFlights_) {
        const auto texture = blood.impacted && !blood.blocked ? bloodImpactArt_[blood.variant] : bloodProjectileArt_;
        if (!texture) continue;
        const float fade = blood.impacted && !blood.blocked ? std::clamp(blood.inactiveAge / (.05f * 5.5f), 0.f, 1.f)
                                                             : std::clamp(blood.age / .3f, 0.f, 1.f);
        const float alpha = blood.impacted && !blood.blocked ? std::clamp((5.5f-blood.inactiveAge)/(5.5f*.2f), 0.f, 1.f)
                                                              : fade;
        const float size = blood.size * fade;
        render::Sprite sprite{texture, {},
            {640.f+(blood.x-cameraX-size/2.f)*scale,
             360.f+(cameraY-blood.y-size/2.f)*scale, size*scale, size*scale}};
        sprite.tint.a = static_cast<std::uint8_t>(std::lround(255.f*alpha));
        sprite.rotationDegrees = blood.rotation;
        items.push_back({blood.impacted && !blood.blocked ? 4000 : 3000, [&, sprite] { renderer.DrawSprite(sprite); }});
    }
    for (const auto& shot:shots_) {
        const float visualScale = shot.playerKnife ? std::clamp(shot.activeVisualAge / 0.3f, 0.f, 1.f) : 1.f;
        if (visualScale <= 0.f) continue;
        const auto texture = shot.texture ? shot.texture : knifeArt_;
        if (!texture) continue;
        const float w = shot.visualWidth * visualScale, h = shot.visualHeight * visualScale;
        render::Sprite sprite{texture,{}, {640.f+(shot.x-cameraX-w/2)*scale,
            360.f+(cameraY-shot.y-h/2)*scale,w*scale,h*scale}};
        sprite.flipX = shot.vx < 0.f;
        // Deliberate tumbling animation requested for the player knife.
        // The Windows player knife only pitches with velocity; ninja stars
        // supply this 20-rad/s spin rate. Keep the existing launch flip.
        if (shot.playerKnife)
            sprite.rotationDegrees = render::OrdinarySpriteRotationDegrees(
                shot.knifeRotationRadians, sprite.flipX, false);
        sprite.tint.a = static_cast<std::uint8_t>(std::lround(255.f * visualScale));
        items.push_back({shot.depth, [&, sprite] { renderer.DrawSprite(sprite); }});
    }
}
void Combat::Draw(render::Renderer& renderer, float cameraX, float cameraY) const {
    std::vector<PainterItem> items;
    AppendPainterItems(items, renderer, cameraX, cameraY);
    std::stable_sort(items.begin(), items.end(),
                     [](const PainterItem& a, const PainterItem& b) { return a.depth > b.depth; });
    for (const auto& item : items) item.draw();
}
}

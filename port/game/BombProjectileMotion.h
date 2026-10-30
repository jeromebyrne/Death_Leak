#pragma once
#include <cmath>

namespace deathleak::game {
// Renderer-neutral translation of Player::FireBomb, Projectile's constructor,
// and BombProjectile::{Update,OnCollision,HandleSolidLineStripCollision}.
// Coordinates are world Y-up; velocity is ORIGINAL units / 60-Hz tick (NOT
// units/second). Rotation is source radians. Use once per fixed gameplay tick.
class BombProjectileMotion {
public:
    static constexpr float Width = 45.f, Height = 45.f;
    static constexpr float ColliderWidth = 30.f, ColliderHeight = 30.f;
    static constexpr float LaunchSpeed = 15.f, SprintLaunchSpeed = 20.f;
    static constexpr double InactiveFuseSeconds = 5.0;
    static constexpr float ExplosionRadius = 1000.f, ExplosionDamage = 500.f;
    struct Point { float x, y; };
    struct Environment {
        bool inWater = false;
        bool wasInWaterLastFrame = false;
        bool onSolidLine = false;   // suppresses gravity, unlike ordinary ground
        bool onSolidSurface = false; // grounded spin (includes solid lines)
    };
    enum class Contact {
        Terrain, SolidLine, Blade, OtherProjectile,
        Character, Debris, CurrencyOrb, Water, Breakable, Pickup, Bomb, Foliage
    };

    float x = 0, y = 0, vx = 0, vy = 0, rotation = 0;
    bool active = true;
    bool pendingExplosion = false;

    BombProjectileMotion() = default;
    BombProjectileMotion(float originX, float originY, float directionX,
                         float directionY, bool sprinting = false) {
        Launch(originX, originY, directionX, directionY, sprinting);
    }
    // Input direction is used VERBATIM, not normalized, as in Projectile.
    // origin is already the muzzle position, NOT the player's position.
    void Launch(float originX, float originY, float directionX,
                float directionY, bool sprinting = false) {
        x = originX; y = originY; rotation = 0;
        const float speed = sprinting ? SprintLaunchSpeed : LaunchSpeed;
        vx = directionX * speed; vy = directionY * speed;
        active = true; pendingExplosion = false; exploded_ = false;
        applyGravity_ = true; inactiveAt_ = 0;
    }
    // FireBomb applies the horizontal offset TWICE; vertical direction takes
    // the left branch. Gate unlock and WasInWaterLastFrame in the caller.
    static Point FireBombOrigin(float playerX, float playerY, float directionX,
                                float offsetX = 5.f, float offsetY = 25.f) {
        return {playerX + (directionX > 0 ? 2.f : -2.f) * offsetX,
                playerY + offsetY};
    }
    void Update(float dt, double now) { Update(dt, now, Environment{}); }
    void Update(float dt, double now, const Environment& environment) {
        if (exploded_ || !(dt >= 0) || !std::isfinite(dt)) return;
        const float ticks = dt * 60.f;
        if (applyGravity_ && !environment.onSolidLine)
            vy -= (environment.inWater ? .06f : .3f) * ticks;
        // Source applies .92 ONCE per Update, not pow(.92, ticks).
        if (environment.inWater) vx *= .92f;
        x += vx * ticks; y += vy * ticks;
        const bool grounded = environment.onSolidSurface || environment.onSolidLine;
        const float spin = grounded ? (environment.inWater ? -.009f : -.04f)
                                    : (environment.inWater ? -.005f : -.01f);
        rotation += vx * spin * ticks;
        // No active-flight TTL: the fuse begins only at first valid contact.
        // Exactly inactiveAt + 5 remains alive; source uses strict >.
        if (!active && now > inactiveAt_ + InactiveFuseSeconds) Explode();
    }
    // Call only after verified collision, using the collision pass's water
    // state. Caller owns narrow-phase, position correction, surface identities,
    // and any SolidMovingSprite bounce response (source dampening is .7).
    // SolidLine means GetBombProjectileCollisionData returned true.
    // Returns true only when this contact deactivates or detonates the bomb.
    bool OnContact(Contact contact, double now) {
        return OnContact(contact, now, Environment{});
    }
    bool OnContact(Contact contact, double now, const Environment& environment) {
        if (exploded_ || environment.inWater || environment.wasInWaterLastFrame) return false;
        switch (contact) {
        case Contact::Blade:
            Explode(); return true;
        case Contact::Terrain:
        case Contact::SolidLine:
        case Contact::OtherProjectile:
            // Unlike blade projectiles, other projectiles fall through to the
            // source's ordinary terrain-style deactivation.
            if (active) {
                active = false; applyGravity_ = false; vx = vy = 0;
                inactiveAt_ = now; return true;
            }
            // Solid-line handler zeros velocity even on repeated contacts.
            if (contact == Contact::SolidLine) { applyGravity_ = false; vx = vy = 0; }
            return false;
        default: return false;
        }
    }
    bool HasExploded() const { return exploded_; }
    double InactiveSince() const { return inactiveAt_; }
    // Deliver once, at current x/y; then remove the bomb. Consuming the event
    // does NOT reactivate motion or permit a second explosion.
    bool TakeExplosion() {
        const bool result = pendingExplosion; pendingExplosion = false; return result;
    }
private:
    void Explode() { exploded_ = true; active = false; pendingExplosion = true; }
    bool applyGravity_ = true, exploded_ = false;
    double inactiveAt_ = 0;
};
}

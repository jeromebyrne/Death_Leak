#include "../game/BombProjectileMotion.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
using Bomb = deathleak::game::BombProjectileMotion;
static constexpr float tick = 1.f / 60.f;
static void Near(float a, float b) { assert(std::abs(a - b) < .0001f); }
static void LaunchAndMotion() {
    static_assert(Bomb::Width == 45 && Bomb::Height == 45);
    static_assert(Bomb::ColliderWidth == 30 && Bomb::ColliderHeight == 30);
    static_assert(Bomb::ExplosionRadius == 1000 && Bomb::ExplosionDamage == 500);
    auto p = Bomb::FireBombOrigin(100,200,1); Near(p.x,110); Near(p.y,225);
    p = Bomb::FireBombOrigin(100,200,0); Near(p.x,90);
    Bomb b(10,20,.6f,.8f); Near(b.vx,9); Near(b.vy,12);
    b.Update(tick,100); Near(b.x,19); Near(b.y,31.7f); Near(b.vy,11.7f); Near(b.rotation,-.09f);
    b.Launch(0,0,-1,0,true); Near(b.vx,-20); b.Update(tick,0); Near(b.rotation,.2f);
    b.Launch(0,0,2,0); Near(b.vx,30); // source does not normalize
    b.Update(tick * 2,0); Near(b.vy,-.6f); Near(b.x,60); Near(b.y,-1.2f); Near(b.rotation,-.6f);
    b.Launch(0,0,0,0); b.Update(tick,99999); assert(b.active && !b.pendingExplosion); // no TTL
    Near(b.y,-.3f); assert(std::isfinite(b.rotation));
}
static void WaterAndSpin() {
    Bomb::Environment e; e.inWater = true;
    Bomb b(0,0,1,0); b.Update(tick,0,e);
    Near(b.vx,13.8f); Near(b.vy,-.06f); Near(b.x,13.8f); Near(b.rotation,-.069f);
    b.Launch(0,0,1,0); b.Update(tick*2,0,e); Near(b.vx,13.8f); Near(b.vy,-.12f); Near(b.x,27.6f);
    b.Launch(0,0,1,0); e.onSolidSurface = true; b.Update(tick,0,e); Near(b.rotation,-.1242f);
    b.Launch(0,0,1,0); e.inWater = false; b.Update(tick,0,e); Near(b.rotation,-.6f); Near(b.vy,-.3f);
    b.Launch(0,0,1,0); e.onSolidLine = true; e.onSolidSurface = false;
    b.Update(tick,0,e); Near(b.vy,0); Near(b.rotation,-.6f);
    e.inWater = true; b.Launch(0,0,1,0); b.Update(tick,0,e); Near(b.vy,0); Near(b.rotation,-.1242f);
}
static void ContactsAndFuse() {
    using C = Bomb::Contact;
    for (C c : {C::Character,C::Debris,C::CurrencyOrb,C::Water,C::Breakable,C::Pickup,C::Bomb,C::Foliage}) {
        Bomb b(0,0,1,1); assert(!b.OnContact(c,10)); assert(b.active); Near(b.vx,15);
    }
    for (C c : {C::Terrain,C::SolidLine,C::OtherProjectile}) {
        Bomb b(0,0,1,1); assert(b.OnContact(c,10)); assert(!b.active); Near(b.vx,0); Near(b.vy,0);
        assert(b.InactiveSince() == 10); assert(!b.OnContact(c,14)); assert(b.InactiveSince() == 10);
        b.Update(tick,15); assert(!b.pendingExplosion && !b.HasExploded()); Near(b.y,0);
        b.Update(tick,std::nextafter(15.0, std::numeric_limits<double>::infinity()));
        assert(b.pendingExplosion && b.HasExploded()); assert(b.TakeExplosion()); assert(!b.TakeExplosion());
        const float x = b.x; b.vx = 100; b.Update(tick,100); Near(b.x,x);
        assert(!b.OnContact(C::Blade,100)); assert(!b.pendingExplosion);
    }
    for (bool previous : {false,true}) {
        Bomb::Environment e; e.inWater = !previous; e.wasInWaterLastFrame = previous;
        for (C c : {C::Terrain,C::SolidLine,C::Blade}) {
            Bomb b(0,0,1,1); assert(!b.OnContact(c,10,e)); assert(b.active && !b.pendingExplosion);
        }
    }
    Bomb b(12,34,1,0); assert(b.OnContact(C::Blade,0)); assert(b.pendingExplosion); Near(b.x,12); Near(b.y,34);
    assert(b.TakeExplosion()); b.Launch(1,2,-1,0); assert(b.active && !b.HasExploded() && !b.pendingExplosion);
    assert(b.OnContact(C::Terrain,0)); assert(b.OnContact(C::Blade,.1)); assert(b.pendingExplosion);
    // Repeated line contacts stop external explosion impulses without resetting fuse.
    b.Launch(0,0,1,0); b.OnContact(C::SolidLine,0); b.vx = 10;
    b.Update(tick,1); Near(b.x,10); assert(!b.OnContact(C::SolidLine,1)); Near(b.vx,0); assert(b.InactiveSince() == 0);
}
int main() {
    LaunchAndMotion(); WaterAndSpin(); ContactsAndFuse();
    std::cout << "bomb projectile motion: PASS (launch, gravity, water/spin, contacts, strict inactive fuse, one-shot detonation)\n";
}

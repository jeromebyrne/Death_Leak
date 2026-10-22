#include "../game/ParticleSpray.h"
#include <cassert>
#include <cmath>

using deathleak::game::ParticleSpray;

int main() {
    ParticleSpray spray;
    spray.fadeIn = .05f;
    spray.fadeOut = .98f;
    spray.warmed = true; // Isolate per-tick motion/fade from deferred loading warm-up.
    ParticleSpray::Particle leaf;
    leaf.x = leaf.startX = 100;
    leaf.y = leaf.startY = 200;
    leaf.directionY = -1;
    leaf.speed = leaf.startSpeed = 4;
    leaf.lifetime = 10;
    spray.particles.push_back(leaf);

    spray.Update(1.f/60.f);
    assert(spray.particles[0].drawY == 200); // source draws pre-step position
    assert(std::abs(spray.particles[0].y - 196) < .001f);
    assert(std::abs(spray.particles[0].alpha - 1.f/30.f) < .001f);

    spray.particles[0].elapsed = 9.99f;
    spray.Update(1.f/60.f);
    assert(spray.particles[0].elapsed == 0);
    assert(spray.particles[0].y == 200);
    assert(spray.particles[0].speed == 4);

    spray.looping = false;
    spray.particles[0].elapsed = 9.99f;
    spray.Update(1.f/60.f);
    assert(spray.particles[0].dead);
}

#include "../game/WeatherRain.h"
#include <cassert>
#include <cmath>
using deathleak::game::WeatherRain;
int main() {
    WeatherRain rain;
    rain.Update(1.f/60, false, 0, 0);
    assert(!rain.VisibleParticles());
    rain.Update(1.f/60, true, 0, 0);
    assert(rain.Active() && rain.Spray().particles.size() == 300);
    assert(rain.Spray().particles[0].x > 98000); // source placeholder
    for (int i = 0; i < 65; ++i) rain.Update(1.f/60, true, 500, 100);
    int entered = 0;
    for (const auto& p : rain.Spray().particles) {
        if (p.x < 3000 && p.y < 3000) ++entered;
        assert(p.directionX == -.2f && p.directionY == -.8f);
        assert(p.gravity == -1.f);
    }
    assert(entered == 300);
    rain.Update(1.f/60, false, 500, 100);
    assert(!rain.Active() && rain.VisibleParticles());
    for (int i = 0; i < 65; ++i) rain.Update(1.f/60, false, 500, 100);
    assert(!rain.VisibleParticles());
}

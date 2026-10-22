#include "../game/WeatherLightning.h"
#include <cassert>
#include <cmath>
#include <iostream>
using deathleak::game::WeatherLightning;
static bool Near(float a, float b, float e = .002f) { return std::fabs(a-b) < e; }
int main() {
    WeatherLightning w(7);
    for (int i = 0; i < 4000; ++i) w.Update(.05f);
    assert(Near(w.Elapsed(), 0.f));
    w.SetAllowed(true);
    w.Update(180.01f);
    assert(!w.Raining() && w.Alpha() == 0.f); // pre-update strict >180
    w.Update(.05f);
    assert(w.Raining() && w.Alpha() == 0.f); // rain asset created; no flash yet
    w.Update(.05f);
    assert(w.Alpha() == 0.f); // first flash scheduled, next update sets alpha
    w.Update(.05f);
    assert(Near(w.Alpha(), 1.f - .05f/.4f, .01f));
    auto a = w.SpriteForCamera(12, -600.f, 3400.f);
    auto b = w.SpriteForCamera(12, 900.f, -250.f);
    assert(a.texture == 12 && Near(a.destination.x, b.destination.x));
    assert(Near(a.destination.y, b.destination.y) && Near(a.destination.width, 2048.f*2.f/3.f));
    assert(Near(a.destination.height, a.destination.width) && a.flipY && a.tint.a == b.tint.a);
    w.SetAllowed(false);
    const float paused = w.Elapsed(), held = w.Alpha();
    for (int i = 0; i < 20; ++i) w.Update(.05f);
    assert(Near(w.Elapsed(), paused) && Near(w.Alpha(), held));
    w.SetAllowed(true);
    for (int i = 0; i < 9; ++i) w.Update(.05f);
    assert(w.Alpha() == 0.f);
    // Seeded next-flash intervals are within 10-30 s; no immediate repeat.
    for (int i = 0; i < 2500 && w.Raining(); ++i) w.Update(.05f);
    assert(!w.Raining() && w.Alpha() == 0.f); // 70-120 s plus 3 s outro
    std::cout << "weather lightning timing/camera/alpha PASS\n";
}

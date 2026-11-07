#include "../game/CameraShake.h"

#include <cassert>
#include <cmath>

using deathleak::game::CameraShake;

static bool Near(float a, float b) { return std::fabs(a - b) < 0.0001f; }

int main() {
    CameraShake shake;
    float x = 100.f, y = 200.f;

    // Intro's sub-threshold request: intensity is sampled at the start of each
    // window, and repeated held-frame requests cannot restart an active window.
    assert(shake.Start(1.25f, 0.1f, 1.f));
    assert(!shake.Start(2.5f, 0.1f, 1.04f));
    auto first = shake.Update(1.f, 1.f, true, true, 1, -1, x, y);
    assert(first.active && Near(first.remaining, 0.1f));
    assert(Near(first.viewX, 100.f) && Near(first.viewY, 200.f));
    assert(Near(x, 101.25f) && Near(y, 198.75f));

    // A later view snapshots the incoming position; intensity remains 1.25,
    // not the rejected 2.5. The active interval is [start, start+duration).
    x = 100.f; y = 200.f;
    auto middle = shake.Update(1.05f, 2.f, true, true, -1, 1, x, y);
    assert(middle.active && Near(middle.remaining, 0.05f));
    assert(Near(middle.viewX, 100.f) && Near(middle.viewY, 200.f));
    assert(Near(x, 98.75f) && Near(y, 201.25f));
    assert(shake.WillJitter(1.09f));
    assert(!shake.WillJitter(1.1f));
    auto expired = shake.Update(1.1f, 1.f, true, true, 1, 1, x, y);
    assert(!expired.active && Near(expired.remaining, 0.f));
    assert(Near(expired.viewX, x) && Near(expired.viewY, y));

    // Once expired, a stronger next intro window can begin. Axis follow gates
    // and the source view-before-jitter contract remain intact.
    x = 100.f; y = 200.f;
    assert(shake.Start(5.f, 0.1f, 1.1f));
    auto next = shake.Update(1.15f, 1.f, false, true, 1, 1, x, y);
    assert(next.active && Near(next.viewX, 100.f) && Near(next.viewY, 200.f));
    assert(Near(x, 100.f) && Near(y, 202.5f));
    shake.Reset();
    assert(!shake.Active() && !shake.WillJitter(1.15f));

    // Existing landing entry point retains its 15 / 0.30 parameters.
    x = 100.f; y = 200.f;
    assert(shake.StartMedium(2.f));
    auto landing = shake.Update(2.1f, 1.f, true, false, 1, 1, x, y);
    assert(landing.active && Near(landing.remaining, 0.2f));
    assert(Near(x, 110.f) && Near(y, 200.f));
}

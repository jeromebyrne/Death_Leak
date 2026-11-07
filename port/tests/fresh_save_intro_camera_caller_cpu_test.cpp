// Focused CPU contract for the fresh-save held-pull CameraShake caller.
#include "../game/CameraShake.h"
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

using deathleak::game::CameraShake;
static bool Near(float a, float b) { return std::fabs(a - b) < .0001f; }

int main() {
    // Check placement in the actual Linux intro branch, including the strict
    // threshold and the independent release path; no renderer is required.
    std::ifstream input("port/linux/main.cpp");
    assert(input);
    const std::string source(std::istreambuf_iterator<char>{input}, {});
    const auto intro = source.find("} else if (!hasPulledSword) {");
    const auto ordinary = source.find("} else {\n            const float direction", intro);
    assert(intro != std::string::npos && ordinary != std::string::npos);
    const auto branch = source.substr(intro, ordinary - intro);
    const auto finished = branch.find("if (scene.IntroBodyFinished())");
    const auto looped = branch.find("IntroBodyPhase::Looped");
    const auto held = branch.find("if (held) {");
    const auto increment = branch.find("introHeldTime += dt;");
    const auto threshold = branch.find("if (introHeldTime > 2.75f)");
    const auto phase2 = branch.find("IntroBodyPhase::Pull");
    const auto subthreshold = branch.find("} else {\n                            // Source requests", phase2);
    const auto start = branch.find("cameraShake.Start((introHeldTime / 2.75f) * 5.f, 0.1f, cameraShakeTime - dt);");
    const auto zoom = branch.find("introEffectiveZoom = std::max", start);
    const auto releasePath = branch.find("introHeldTime = 0.f;\n                        introEffectiveZoom = std::min", zoom);
    assert(finished < looped && looped < held && held < increment && increment < threshold);
    assert(threshold < phase2 && phase2 < subthreshold && subthreshold < start && start < zoom && zoom < releasePath);
    assert(branch.find("cameraShake.Start(", start + 1) == std::string::npos);

    CameraShake shake;
    float heldTime = 0.f;
    float clock = 0.f;
    float x = 100.f, y = 200.f;
    int starts = 0, refusals = 0;
    auto tick = [&](float dt, bool heldInput, bool loopedPhase, bool finishedBody) {
        clock += dt;
        // Match main's caller: completion, phase-2, release, and threshold
        // cannot schedule a new window. Clock offset is the draw-time clock.
        if (!finishedBody && loopedPhase) {
            if (heldInput) {
                heldTime += dt;
                if (heldTime > 2.75f) heldTime = 0.f;
                else if (shake.Start((heldTime / 2.75f) * 5.f, .1f, clock - dt)) ++starts;
                else ++refusals;
            } else heldTime = 0.f;
        }
        const auto sample = shake.Update(clock - dt, 1.f, true, true, 1, -1, x, y);
        x = sample.viewX; y = sample.viewY; // next camera follow restores view
        return sample;
    };
    auto first = tick(.055f, true, true, false);
    assert(starts == 1 && refusals == 0 && first.active);
    assert(Near(first.remaining, .1f));
    assert(Near(first.storedX - first.viewX, .1f)); // (.055/2.75)*5
    auto refused = tick(.05f, true, true, false);
    assert(starts == 1 && refusals == 1 && refused.active);
    assert(Near(refused.remaining, .045f));
    assert(Near(refused.storedX - refused.viewX, .045f)); // retained first intensity
    auto released = tick(.05f, false, true, false);
    assert(starts == 1 && refusals == 1 && !released.active);
    auto resumed = tick(.05f, true, true, false);
    assert(starts == 2 && Near(heldTime, .05f) && resumed.active);
    tick(.11f, false, true, false);
    tick(.01f, false, true, false); // expire the second window
    heldTime = 2.75f;
    auto equality = tick(0.f, true, true, false);
    assert(starts == 3 && equality.active); // strict > means equality is held
    auto thresholdFrame = tick(.001f, true, true, false);
    assert(starts == 3 && Near(heldTime, 0.f)); // no threshold restart
    assert(thresholdFrame.active); // prior active window may finish afterward
    tick(.01f, true, false, false);
    tick(.01f, true, false, true);
    assert(starts == 3 && refusals == 1); // no phase-2/completion request
    std::cout << "fresh-save intro camera caller CPU test passed\n";
}

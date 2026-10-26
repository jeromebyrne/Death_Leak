#include "../game/PresentationPose.h"
#include <cassert>
#include <cstring>
#include <limits>
using deathleak::game::PresentationPose;
using deathleak::game::PresentationPoseHistory;
static bool EqualBits(const PresentationPose& a, const PresentationPose& b) {
    return std::memcmp(&a.playerX, &b.playerX, sizeof(float)) == 0 &&
        std::memcmp(&a.playerY, &b.playerY, sizeof(float)) == 0 &&
        std::memcmp(&a.cameraX, &b.cameraX, sizeof(float)) == 0 &&
        std::memcmp(&a.cameraY, &b.cameraY, sizeof(float)) == 0 &&
        a.sceneGeneration == b.sceneGeneration;
}
int main() {
    PresentationPoseHistory history;
    const PresentationPose a{100.f, -20.f, 10.f, 8.f, 1};
    const PresentationPose b{120.f, -10.f, 20.f, 18.f, 1};
    history.Seal(a);
    assert(EqualBits(history.Sample(0.5), a));
    history.Seal(b);
    assert(history.HasContinuousPair());
    assert(EqualBits(history.Sample(1.0), b));
    const auto half = history.Sample(0.5);
    assert(half.playerX == 110.f && half.playerY == -15.f);
    assert(half.cameraX == 15.f && half.cameraY == 13.f);
    assert(EqualBits(history.Sample(0.5), half));
    assert(EqualBits(history.Sample(std::numeric_limits<double>::quiet_NaN()), b));
    const PresentationPose sceneChange{1.f, 2.f, 3.f, 4.f, 2};
    history.Seal(sceneChange);
    assert(!history.HasContinuousPair());
    assert(EqualBits(history.Sample(0.5), sceneChange));
    history.Seal(a, false); // explicit save restoration discontinuity
    assert(!history.HasContinuousPair());
    assert(EqualBits(history.Sample(0.5), a));
    const PresentationPose teleport{500.f, 0.f, 5.f, 8.f, 1};
    history.Seal(teleport); // same-scene positional discontinuity
    assert(!history.HasContinuousPair());
    assert(EqualBits(history.Sample(0.5), teleport));
}

#include "../linux/InterpolationTopologyOracle.h"
#include "../linux/FixedStepPacer.h"
#include "../game/LevelScene.h"
#include <type_traits>
#include <utility>
#include <cassert>
#include <chrono>
#include <iostream>
using namespace deathleak::linux_port;
template<class T, class = void> struct HasStableIdentity : std::false_type {};
template<class T> struct HasStableIdentity<T, std::void_t<decltype(std::declval<T>().stableIdentity)>> : std::true_type {};
static_assert(!HasStableIdentity<deathleak::game::SceneSprite>::value,
              "current authored/runtime sprite has no semantic stableIdentity");
static_assert(!HasStableIdentity<deathleak::game::ParticleSpray::Particle>::value,
              "current attached particle has no semantic stableIdentity");
static TopologySnapshot Scene() {
    TopologySnapshot s;
    s.sceneEpoch = 9;
    s.items = {{{1,0},100,true,100,20,1,1}, // body
               {{2,0},101,true,100,20,1,1}, // arm
               {{3,0},102,true,105,15,1,1}, // spray particle
               {{4,0},103,true,200,0,.5f,.97f}}; // parallax background
    return s;
}
int main() {
    auto a = Scene(), b = Scene();
    b.cameraX = 20;
    b.items[0].worldX = b.items[1].worldX = 120;
    b.items[2].worldX = 125;
    const auto x0 = SampleTopology(a,b,0), xhalf = SampleTopology(a,b,.5f), x1 = SampleTopology(a,b,1);
    assert(x0.decision == TopologyDecision::Interpolate);
    assert(xhalf.decision == TopologyDecision::Interpolate);
    assert(xhalf.sample[0].worldX == 100); // actor and camera move together
    assert(xhalf.sample[2].worldX == 105);
    assert(xhalf.sample[3].worldX == 195); // distinct parallax factor
    assert(x0.sample[0].worldX == x1.sample[0].worldX);
    const auto sameAlpha = SampleTopology(a,b,.5f);
    assert(sameAlpha.sample[3].worldX == xhalf.sample[3].worldX); // ordinary/repeat purity
    auto changed = b; changed.items.push_back({{5,0},104,true,130,20,1,1});
    assert(SampleTopology(a,changed,.5f).decision == TopologyDecision::HoldCurrent); // birth
    assert(SampleTopology(changed,b,.5f).decision == TopologyDecision::HoldCurrent); // death
    changed = b; changed.items[2].visible = false;
    assert(SampleTopology(a,changed,.5f).decision == TopologyDecision::HoldCurrent);
    changed = b; changed.items[2].key.generation = 1;
    assert(SampleTopology(a,changed,.5f).decision == TopologyDecision::HoldCurrent); // reused slot
    changed = b; changed.items[1].painterOrdinal = 99;
    assert(SampleTopology(a,changed,.5f).decision == TopologyDecision::Invalid); // order inversion
    changed = b; changed.items[2].painterOrdinal = 103;
    assert(SampleTopology(a,changed,.5f).decision == TopologyDecision::HoldCurrent);
    changed = b; changed.cameraX = 1000;
    assert(SampleTopology(a,changed,.5f).decision == TopologyDecision::HoldCurrent);
    assert(SampleTopology(a,changed,.5f).sample[0].worldX == -880);
    changed = b; changed.items[0].worldX = 1000;
    assert(SampleTopology(a,changed,.5f).decision == TopologyDecision::HoldCurrent);
    changed = b; changed.sceneEpoch++;
    assert(SampleTopology(a,changed,.5f).decision == TopologyDecision::HoldCurrent);
    changed = b; changed.discontinuity = true;
    assert(SampleTopology(a,changed,.5f).decision == TopologyDecision::HoldCurrent);
    changed = b; changed.items[0].key.object = 0;
    assert(SampleTopology(a,changed,.5f).decision == TopologyDecision::Invalid);
    changed = b; changed.items[1].key = changed.items[0].key;
    assert(SampleTopology(a,changed,.5f).decision == TopologyDecision::Invalid);
    using Clock = FixedStepPacer::Clock;
    for (int hz : {30,60,75,120,144,240}) {
        FixedStepPacer p(Clock::time_point{});
        int updates=0, repeats=0;
        for (int slot=0; slot<=hz*10; ++slot) {
            p.Accumulate(Clock::time_point{}+std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(double(slot)/hz)));
            if (!p.Due()) ++repeats;
            while (p.Due()) { p.Consume(); ++updates; }
            const auto ordinaryOrRepeat = SampleTopology(a,b,float(p.PresentationAlpha()));
            const auto secondSameAlpha = SampleTopology(a,b,float(p.PresentationAlpha()));
            assert(ordinaryOrRepeat.sample[3].worldX == secondSameAlpha.sample[3].worldX);
        }
        assert(updates == 601 && p.DroppedSteps() == 0);
        assert((repeats > 0) == (hz > 60));
    }
    std::cout << "topology and 30/60/75/120/144/240 Hz cadence oracle passed\n";
}

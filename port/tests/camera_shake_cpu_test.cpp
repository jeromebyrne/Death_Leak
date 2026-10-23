#include "../game/CameraShake.h"
#include <cassert>
#include <cmath>
#include <iostream>
using deathleak::game::CameraShake;
static bool near(float a, float b) { return std::fabs(a-b) < 0.0001f; }
int main() {
    CameraShake shake;
    float x=100, y=200;
    auto log = [&](const char* event, int frame, float clock, int sign) {
        const float baseX=x, baseY=y;
        const auto sample=shake.Update(clock,1.f,true,false,sign,1,x,y);
        std::cout << event << " frame=" << frame << " base=(" << baseX << ',' << baseY
                  << ") active=" << sample.active << " remaining=" << sample.remaining
                  << " stored=(" << sample.storedX << ',' << sample.storedY
                  << ") view=(" << sample.viewX << ',' << sample.viewY << ") hud=(12,34)\n";
        assert(near(sample.viewX,baseX) && near(sample.viewY,baseY));
        assert(near(sample.storedY,200));
        assert(std::fabs(sample.storedX-baseX)<=15.0001f);
        return sample;
    };
    auto control=log("control",0,0.f,1); assert(!control.active && near(x,100));
    assert(shake.StartMedium(1.f));
    auto first=log("large-drop",60,1.f,1); assert(first.active && near(first.viewX,100) && near(x,115));
    assert(!shake.StartMedium(1.05f));
    // Source follow/clamp precedes camera Update; a later view sees the prior stored perturbation.
    x=100+(x-100)/10.f;
    auto second=log("active",61,1.1f,-1);
    assert(near(second.viewX,101.5f) && near(second.storedX,91.5f));
    auto final=log("expiry",78,1.3f,1); assert(!final.active && near(final.storedX,final.viewX));
    assert(shake.StartMedium(1.4f));
    x=100; y=200;
    auto noFollow=shake.Update(1.4f,1.f,false,false,1,-1,x,y);
    assert(near(noFollow.storedX,100) && near(noFollow.storedY,200));
    shake.Reset(); assert(!shake.Active());
}

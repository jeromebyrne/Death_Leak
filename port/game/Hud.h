#pragma once
#include "../render/Renderer.h"

namespace deathleak::game {
// Renderer-neutral meter subset of XmlFiles/UI/game_hud.xml.
class Hud {
public:
    void Load(render::Renderer& renderer);
    void Draw(render::Renderer& renderer, float health, float maxHealth, float focus, int orbBalance, float preRegistrationProcessTotal);
private:
    struct MeterProgress {
        float displayed = 1.f;
        float target = 0.f;
        float previousFill = 0.f; // also the in-flight movement baseline
        float startProcessTotal = 0.f;
        bool moving = false;
        void Update(float processTotal, bool snapDecrease);
        void SetProgress(float newTarget, float processTotal);
    };
    MeterProgress healthProgress_, focusProgress_;
    render::TextureId edge_ = 0, bar_ = 0, backing_ = 0, focusBar_ = 0, overlay_ = 0, orb_ = 0;
    // UIMeter starts at the XML width; UIGameHudScreen schedules a cap
    // change for the next update, then shrinks immediately or grows by 0.20.
    float healthMeterLength_ = 300.f; // original 1920-wide UI units
    float lastMaxHealth_ = 0.f;
    bool healthMeterUpgrade_ = false;
};
}

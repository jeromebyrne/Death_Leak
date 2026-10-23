#include "Hud.h"
#include "BitmapText.h"
#include <algorithm>
#include <string>

namespace deathleak::game {
void Hud::MeterProgress::Update(float processTotal, bool snapDecrease) {
    if (!moving) {
        if (previousFill > displayed) previousFill -= .005f;
        else previousFill = displayed;
        return;
    }
    const float difference = target - previousFill;
    if (snapDecrease && difference < 0.f) {
        displayed = target;
        moving = false;
        return;
    }
    const float elapsed = processTotal - startProcessTotal;
    if (elapsed > 0.f) {
        const float fraction = std::min(elapsed, .25f) / .25f;
        displayed = previousFill + difference * fraction;
        if (fraction >= 1.f) {
            displayed = target;
            moving = false;
        }
    }
}
void Hud::MeterProgress::SetProgress(float newTarget, float processTotal) {
    // Original UIMeter compares with displayed, even while a move is active.
    if (newTarget != displayed) {
        if (!moving) {
            previousFill = displayed;
            startProcessTotal = processTotal;
        }
        target = newTarget;
        moving = true;
    }
}
void Hud::Load(render::Renderer& renderer) {
    std::string error;
    edge_ = renderer.LoadTexture("Game/Media/UI/meter_edge_2.png", error);
    bar_ = renderer.LoadTexture("Game/Media/UI/meter_bar_middle.png", error);
    backing_ = renderer.LoadTexture("Game/Media/UI/meter_bar_middle_back.png", error);
    focusBar_ = renderer.LoadTexture("Game/Media/UI/xp_meter_bar_middle.png", error);
    overlay_ = renderer.LoadTexture("Game/Media/UI/meter_overlay.png", error);
    orb_ = renderer.LoadTexture("Game/Media/UI/orb_count.png", error);
}
void Hud::Draw(render::Renderer& renderer, float health, float maxHealth, float focus, int orbBalance, float preRegistrationProcessTotal) {
    // UIScreen advances children before UIGameHudScreen registers new targets.
    healthProgress_.Update(preRegistrationProcessTotal, false);
    focusProgress_.Update(preRegistrationProcessTotal, true);
    // Source UI: health meter x=-900,y=460,w=300; orb x=-900,y=360.
    // Translate from 1920x1080 centered UI to the 1280x720 logical viewport.
    if (healthMeterUpgrade_) {
        const float targetLength = maxHealth * 3.f;
        if (healthMeterLength_ < targetLength) {
            healthMeterLength_ += 0.20f;
        } else {
            healthMeterLength_ = targetLength;
            healthMeterUpgrade_ = false;
        }
    }
    if (maxHealth != lastMaxHealth_) {
        lastMaxHealth_ = maxHealth;
        healthMeterUpgrade_ = true;
    }
    const float healthWidth = healthMeterLength_ * (2.f / 3.f);
    // Original UIMeter bar: (length - 0.8 * 4px edge) * progress.
    const float healthBarWidth = (healthMeterLength_ - 3.2f) * (2.f / 3.f);
    if (orb_) renderer.DrawSprite({orb_, {}, {40, 80, 67.3f, 42.7f}, {255,255,255,191}});
    // UIGameHudScreen draws the spendable SaveManager count at (105,135)
    // in 1920x1080 coordinates; scale to this 1280x720 viewport.
    DrawBitmapText(renderer, std::to_string(orbBalance), 70, 90, 3, {179,179,179,204});
    const float healthProgress = maxHealth > 0.f ? std::clamp(health / maxHealth, 0.f, 1.f) : 0.f;
    healthProgress_.SetProgress(healthProgress, preRegistrationProcessTotal);
    focusProgress_.SetProgress(std::clamp(focus / 100.f, 0.f, 1.f), preRegistrationProcessTotal);
    if (backing_ && healthProgress_.previousFill > healthProgress_.displayed &&
        healthBarWidth * healthProgress_.previousFill > 0.f)
        renderer.DrawSprite({backing_, {}, {43, 45, healthBarWidth * healthProgress_.previousFill, 9}, {255,255,255,220}});
    if (bar_) renderer.DrawSprite({bar_, {}, {43, 45, healthBarWidth * healthProgress_.displayed, 9}, {255,255,255,220}});
    if (overlay_) renderer.DrawSprite({overlay_, {}, {40, 43, healthWidth, 13}, {255,255,255,178}});
    if (edge_) {
        renderer.DrawSprite({edge_, {}, {39, 42, 4, 15}, {255,255,255,220}});
        renderer.DrawSprite({edge_, {}, {37 + healthWidth, 42, 4, 15}, {255,255,255,220}, 1.f, 0, true});
    }
    // The original 91_xp_meter is the focus meter (200 units wide at
    // x=-900,y=440 in the 1920x1080 UI), directly below health.
    if (focusBar_) renderer.DrawSprite({focusBar_, {}, {43, 66, 127.f * focusProgress_.displayed, 9}, {255,255,255,220}});
    if (overlay_) renderer.DrawSprite({overlay_, {}, {40, 64, 133, 13}, {255,255,255,178}});
    if (edge_) {
        renderer.DrawSprite({edge_, {}, {39, 63, 4, 15}, {255,255,255,220}});
        renderer.DrawSprite({edge_, {}, {170, 63, 4, 15}, {255,255,255,220}, 0, true});
    }
}
}

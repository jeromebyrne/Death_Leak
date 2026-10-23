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
    // UIMeter forces every component to the 19px edge height, then draws
    // from the same bottom-left XML origin. Project each complete stack.
    constexpr float meterHeight = 19.f * (2.f / 3.f);
    constexpr float healthTop = (540.f - 460.f - 19.f) * (2.f / 3.f);
    constexpr float focusTop = (540.f - 440.f - 19.f) * (2.f / 3.f);
    const float healthWidth = healthMeterLength_ * (2.f / 3.f);
    // UIMeter offsets bars by 0.9 edge and middle by 1.0 edge.
    constexpr float meterX = 40.f;
    constexpr float edgeWidth = 4.f;
    constexpr float uiScale = 2.f / 3.f;
    const float barX = meterX + edgeWidth * 0.9f * uiScale;
    const float overlayX = meterX + edgeWidth * uiScale;
    const float healthBarWidth = (healthMeterLength_ - edgeWidth * 0.8f) * uiScale;
    const float healthOverlayWidth = (healthMeterLength_ - edgeWidth * 0.95f) * uiScale;
    const float focusBarWidth = (200.f - edgeWidth * 0.8f) * uiScale;
    const float focusOverlayWidth = (200.f - edgeWidth * 0.95f) * uiScale;
    // XML orb bottom-left (-900, 360), 101x64 in Y-up 1920x1080 UI.
    // Project its top-left and size into the Y-down 1280x720 viewport.
    if (orb_) renderer.DrawSprite({orb_, {}, {40.f, (540.f - (360.f + 64.f)) * (2.f / 3.f),
                                                101.f * (2.f / 3.f), 64.f * (2.f / 3.f)},
                                   {255,255,255,191}});
    // UIGameHudScreen draws the spendable SaveManager count at (105,135)
    // in 1920x1080 coordinates; scale to this 1280x720 viewport.
    DrawBitmapText(renderer, std::to_string(orbBalance), 70, 90, 3, {179,179,179,204});
    const float healthProgress = maxHealth > 0.f ? std::clamp(health / maxHealth, 0.f, 1.f) : 0.f;
    healthProgress_.SetProgress(healthProgress, preRegistrationProcessTotal);
    focusProgress_.SetProgress(std::clamp(focus / 100.f, 0.f, 1.f), preRegistrationProcessTotal);
    if (backing_ && healthProgress_.previousFill > healthProgress_.displayed &&
        healthBarWidth * healthProgress_.previousFill > 0.f)
        renderer.DrawSprite({backing_, {}, {barX, healthTop, healthBarWidth * healthProgress_.previousFill, meterHeight}, {255,255,255,220}});
    if (bar_) renderer.DrawSprite({bar_, {}, {barX, healthTop, healthBarWidth * healthProgress_.displayed, meterHeight}, {255,255,255,220}});
    // meter_edge_2.png is 4x19 native pixels; UIMeter draws both caps at
    // that native size. Project their geometry from 1920x1080 to 1280x720.
    constexpr float capWidth = 4.f * (2.f / 3.f);
    constexpr float capHeight = meterHeight;
    if (edge_) {
        renderer.DrawSprite({edge_, {}, {40.f, healthTop, capWidth, capHeight}, {255,255,255,220}});
        renderer.DrawSprite({edge_, {}, {40.f + healthWidth, healthTop, capWidth, capHeight}, {255,255,255,220}, 1.f, 0, true});
    }
    if (overlay_) renderer.DrawSprite({overlay_, {}, {overlayX, healthTop, healthOverlayWidth, meterHeight}, {255,255,255,178}});
    // The original 91_xp_meter is the focus meter (200 units wide at
    // x=-900,y=440 in the 1920x1080 UI), directly below health.
    if (focusBar_) renderer.DrawSprite({focusBar_, {}, {barX, focusTop, focusBarWidth * focusProgress_.displayed, meterHeight}, {255,255,255,220}, 1.f, 0, false, true});
    if (edge_) {
        renderer.DrawSprite({edge_, {}, {40.f, focusTop, capWidth, capHeight}, {255,255,255,220}});
        renderer.DrawSprite({edge_, {}, {40.f + 200.f * (2.f / 3.f), focusTop, capWidth, capHeight}, {255,255,255,220}, 1.f, 0, true, true});
    }
    if (overlay_) renderer.DrawSprite({overlay_, {}, {overlayX, focusTop, focusOverlayWidth, meterHeight}, {255,255,255,178}});
}
}

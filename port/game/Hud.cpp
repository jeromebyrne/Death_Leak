#include "Hud.h"
#include <algorithm>
#include <string>

namespace deathleak::game {
void Hud::Load(render::Renderer& renderer) {
    std::string error;
    edge_ = renderer.LoadTexture("Game/Media/UI/meter_edge_2.png", error);
    bar_ = renderer.LoadTexture("Game/Media/UI/meter_bar_middle.png", error);
    focusBar_ = renderer.LoadTexture("Game/Media/UI/xp_meter_bar_middle.png", error);
    overlay_ = renderer.LoadTexture("Game/Media/UI/meter_overlay.png", error);
    orb_ = renderer.LoadTexture("Game/Media/UI/orb_count.png", error);
}
void Hud::Draw(render::Renderer& renderer, float health, float focus) const {
    // Source UI: health meter x=-900,y=460,w=300; orb x=-900,y=360.
    // Translate from 1920x1080 centered UI to the 1280x720 logical viewport.
    if (orb_) renderer.DrawSprite({orb_, {}, {40, 80, 67.3f, 42.7f}, {255,255,255,191}});
    if (bar_) renderer.DrawSprite({bar_, {}, {43, 45, 194 * std::clamp(health / 25.f, 0.f, 1.f), 9}, {255,255,255,220}});
    if (overlay_) renderer.DrawSprite({overlay_, {}, {40, 43, 200, 13}, {255,255,255,178}});
    if (edge_) {
        renderer.DrawSprite({edge_, {}, {39, 42, 4, 15}, {255,255,255,220}});
        renderer.DrawSprite({edge_, {}, {237, 42, 4, 15}, {255,255,255,220}, 0, true});
    }
    // The original 91_xp_meter is the focus meter (200 units wide at
    // x=-900,y=440 in the 1920x1080 UI), directly below health.
    if (focusBar_) renderer.DrawSprite({focusBar_, {}, {43, 66, 127.f * std::clamp(focus / 100.f, 0.f, 1.f), 9}, {255,255,255,220}});
    if (overlay_) renderer.DrawSprite({overlay_, {}, {40, 64, 133, 13}, {255,255,255,178}});
    if (edge_) {
        renderer.DrawSprite({edge_, {}, {39, 63, 4, 15}, {255,255,255,220}});
        renderer.DrawSprite({edge_, {}, {170, 63, 4, 15}, {255,255,255,220}, 0, true});
    }
}
}

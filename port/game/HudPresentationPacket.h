#pragma once
#include "Hud.h"

namespace deathleak::game {
// An immutable copy of the fixed-update HUD state. Repeated presentation of
// this packet submits the same HUD commands without advancing simulation.
struct HudPresentationPacket {
    const Hud preparedHud;
    const int orbBalance;
    HudPresentationPacket(const Hud& hud, int balance)
        : preparedHud(hud), orbBalance(balance) {}
    void Draw(render::Renderer& renderer) const { preparedHud.Draw(renderer, orbBalance); }
};
}

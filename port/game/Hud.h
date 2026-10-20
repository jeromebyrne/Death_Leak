#pragma once
#include "../render/Renderer.h"

namespace deathleak::game {
// Small renderer-neutral subset of XmlFiles/UI/game_hud.xml.
class Hud {
public:
    void Load(render::Renderer& renderer);
    void Draw(render::Renderer& renderer, float health, float focus) const;
private:
    render::TextureId edge_ = 0, bar_ = 0, overlay_ = 0, orb_ = 0;
};
}

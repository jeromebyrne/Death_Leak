// Forest-1 CPU draw spy: source SetAimLineDirection quadrants and strafe hold.
#include "../game/LevelScene.h"
#include "../game/Combat.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace deathleak;
class Spy final : public render::Renderer {
public:
    std::unordered_map<std::string, render::TextureId> textures;
    std::vector<render::Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path, std::string&) override {
        return textures.try_emplace(path, static_cast<render::TextureId>(textures.size()+1)).first->second;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& sprite) override { drawn.push_back(sprite); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
int main() {
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "2,__player_aim_line", 1);
    Spy spy;
    game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", spy, error));
    const auto texture = spy.textures.at("Game/Media/aim_line.png");
    const float px = scene.PlayerX(), py = scene.PlayerY(), scale = 2.f/3.f;
    auto update = [&](float dt, int facing, bool attempt, game::PlayerStrafeVisual strafe,
                      bool active, float x, float y) {
        scene.UpdatePlayerVisual(dt, 0.f, 0.f, true, 0, false, false, false, 0.f,
                                 facing, false, attempt, strafe, active, x, y);
    };
    auto draw = [&]() {
        spy.drawn.clear(); scene.Draw(spy, px, py, px, py);
        std::vector<render::Sprite> lines;
        for (const auto& sprite : spy.drawn) if (sprite.texture == texture) lines.push_back(sprite);
        assert(lines.size() <= 1);
        return lines;
    };
    assert(draw().empty());
    // Before the first active stick, neutral input must retain the authored
    // +5 X attachment and unflipped, zero-angle line even when body faces left.
    update(1.f/60.f, -1, true, {}, false, 0, 0);
    const auto neutralLeft = draw(); assert(neutralLeft.size() == 1);
    assert(neutralLeft.front().tint.a == 255);
    assert(std::abs(neutralLeft.front().destination.x - (640.f + (5.f-200.f)*scale)) < .005f);
    assert(!neutralLeft.front().flipX && neutralLeft.front().flipY);
    assert(neutralLeft.front().rotationDegrees == 0.f);
    // Right-stick Y is already converted from SDL screen-down to XInput/world-up.
    struct Case { float x, y; int facing; };
    for (const auto row : std::vector<Case>{{9000,0,-1},{-9000,0,1},
             {9000,9000,-1},{9000,-9000,-1},{-9000,9000,1},{-9000,-9000,1},
             {0,9000,1},{0,-9000,1}}) {
        const auto strafe = game::PlayerStrafeVisual{true, row.x > 0 ? 1 : -1, row.facing};
        update(1.f/60.f, row.facing, true, strafe, true, row.x, row.y);
        const auto lines = draw(); assert(lines.size() == 1);
        const auto& line = lines.front();
        const int side = row.x > 0 ? 1 : -1;
        const float angle = std::acos(row.x / std::hypot(row.x,row.y)) * 180.f / 3.14159265358979323846f;
        const float authoredSign = ((row.y < 0) == (row.x > 0)) ? -1.f : 1.f;
        // OrdinarySpriteRotationDegrees reverses the authored angle when both
        // axes are unflipped; a left aim flips Y and thus keeps its sign.
        const float expectedSDL = angle * authoredSign * (side > 0 ? -1.f : 1.f);
        assert(line.tint.a == 255 && !line.flipX && line.flipY == (side > 0));
        assert(std::abs(line.rotationDegrees - expectedSDL) < .005f);
        assert(std::abs(line.destination.x - (640.f + (side*5.f-200.f)*scale)) < .005f);
        assert(std::abs(line.destination.y - (360.f + (-25.f-200.f)*scale)) < .005f);
        assert(std::abs(line.destination.width - 400.f*scale) < .005f);
        // Fresh combat state records the same normalized source stick vector.
        game::Combat combat;
        combat.Update(scene, px, py, 1.f/60.f, true, false, true, false, row.facing,
                      spy, row.x, row.y);
        assert(combat.Fired() == 1 && !combat.Shots().empty());
        const auto& shot = combat.Shots().back();
        assert(std::abs(shot.launchDirectionX - row.x/std::hypot(row.x,row.y)) < .0001f);
        assert(std::abs(shot.launchDirectionY - row.y/std::hypot(row.x,row.y)) < .0001f);
    }
    // Opposed-stick strafe holds a line even with an intentionally suppressed
    // fire attempt. This isolates Player::Update's independent hold path.
    const game::PlayerStrafeVisual opposed{true,1,-1};
    update(1.f/60.f, -1, false, opposed, true, 9000, 9000);
    assert(draw().front().tint.a == 255);
    update(1.f/60.f, -1, false, opposed, true, 9000, 9000);
    assert(draw().front().tint.a == 255);
    // Neutral keeps the source's last angle/attachment, then fades at 10/s.
    const auto held = draw().front();
    for (int i=0; i<2; ++i) update(1.f/60.f, -1, false, {}, false, 0, 0);
    assert(draw().front().tint.a == 255);
    update(1.f/60.f, -1, false, {}, false, 0, 0);
    const auto faded = draw().front();
    assert(faded.tint.a == static_cast<unsigned char>(std::lround((1.f-10.f/60.f)*255.f)));
    assert(faded.destination.x == held.destination.x && faded.rotationDegrees == held.rotationDegrees && faded.flipY == held.flipY);
    for (int i=0; i<6; ++i) update(1.f/60.f, -1, false, {}, false, 0, 0);
    assert(draw().empty());
    std::cout << "PASS: forest-1 neutral-first parity, right-stick quadrants, knife vectors, strafe hold and neutral fade\n";
}

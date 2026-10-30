#include "../game/Combat.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::vector<deathleak::render::Sprite> sprites;
    bool Initialize(const char*, int, int, bool, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(deathleak::render::Color) override {}
    void DrawSprite(const deathleak::render::Sprite& sprite) override { sprites.push_back(sprite); }
    void FillRect(deathleak::render::Rect, deathleak::render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    deathleak::render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};

int main() {
    using namespace deathleak::game;
    CaptureRenderer renderer;
    LevelScene empty;
    Combat combat;
    auto tick = [&](float dt, bool fire) {
        combat.Update(empty, 100000.f, 100000.f, dt, fire, false, true, false, -1, renderer);
    };
    auto expect = [&](float age, float fraction) {
        assert(combat.ShotCount() == 1);
        const auto& shot = combat.Shots().front();
        assert(shot.playerKnife && std::abs(shot.activeVisualAge - age) < .0001f);
        renderer.sprites.clear();
        combat.Draw(renderer, shot.x, shot.y);
        if (fraction == 0.f) { assert(renderer.sprites.empty()); return; }
        assert(renderer.sprites.size() == 1);
        const auto& sprite = renderer.sprites.front();
        assert(std::abs(sprite.destination.width - 84.5f * fraction * 2.f/3.f) < .001f);
        assert(std::abs(sprite.destination.height - 25.f * fraction * 2.f/3.f) < .001f);
        assert(sprite.tint.a == std::lround(255.f * fraction));
        assert(sprite.flipX);
    };
    tick(0.f, true); expect(0.f, 0.f);
    tick(.15f, false); expect(.15f, .5f);
    tick(.15f, false); expect(.30f, 1.f);
    tick(.05f, false); expect(.35f, 1.f); // Must survive beyond old 0.25 s active TTL.
    for (int i = 0; i < 9; ++i) tick(1.f, false);
    assert(combat.ShotCount() == 1);
    tick(1.f, false);
    assert(combat.ShotCount() == 0); // Port-only offscreen collection bound.
    LevelScene solid;
    std::string error;
    assert(solid.Load("port/tests/fixtures/solid_box.xml", renderer, error));
    assert(!solid.SolidBoxes().empty());
    Combat contact;
    contact.Update(solid, 0.f, 200.f, 0.f, true, false, true, false, 1, renderer);
    assert(contact.ShotCount() == 1);
    contact.Update(solid, 0.f, 200.f, .15f, false, false, true, false, 1, renderer);
    assert(contact.ShotCount() == 0); // Existing solid contact still retires immediately.
    // Original Projectile::DoRotateToDirection pitches with current velocity:
    // gravity turns a horizontal throw downward, while a left throw preserves
    // the launch reflection. Its texture tip must point along velocity.
    for (int facing : {1, -1}) {
        Combat trajectory;
        auto step = [&](float dt, bool fire) {
            trajectory.Update(empty, 100000.f, 100000.f, dt, fire, false,
                              true, false, facing, renderer);
        };
        step(0.f, true);
        auto drawKnife = [&]() {
            renderer.sprites.clear();
            const auto& shot = trajectory.Shots().front();
            trajectory.Draw(renderer, shot.x, shot.y);
            assert(renderer.sprites.size() == 1);
            return renderer.sprites.front();
        };
        step(.15f, false);
        const auto early = drawKnife();
        step(.15f, false);
        const auto late = drawKnife();
        assert(early.flipX == (facing < 0) && early.flipY == (facing > 0));
        const auto& shot = trajectory.Shots().front();
        const float radians = late.rotationDegrees * 3.14159265358979323846f / 180.f;
        const float sign = late.flipX ? -1.f : 1.f;
        const float tipX = sign*std::cos(radians), tipY = sign*std::sin(radians);
        const float speed = std::hypot(shot.vx,shot.vy);
        assert((tipX*shot.vx-tipY*shot.vy)/speed > .9999f);
        assert(late.flipX == early.flipX && late.flipY == early.flipY);
        assert(std::abs(late.rotationDegrees - early.rotationDegrees) > .1f);
        assert(std::abs(late.rotationDegrees) <= 180.f);
        assert(std::abs(early.rotationDegrees) <= 180.f);
    }
    for (const auto aim : {std::pair<float,float>{1,0}, {-1,0}, {1,1},
                           {-1,1}, {1,-1}, {-1,-1}, {0,1}, {0,-1}}) {
        Combat directed;
        directed.Update(empty,100000,100000,0,true,false,true,false,1,renderer,aim.first,aim.second);
        directed.Update(empty,100000,100000,.15f,false,false,true,false,1,renderer);
        assert(directed.ShotCount()==1);
        const auto& shot=directed.Shots().front(); renderer.sprites.clear();
        directed.Draw(renderer,shot.x,shot.y); assert(renderer.sprites.size()==1);
        const auto& sprite=renderer.sprites.front();
        const float angle=sprite.rotationDegrees*3.14159265358979323846f/180.f;
        const float sign=sprite.flipX?-1.f:1.f;
        assert(sign*(std::cos(angle)*shot.vx-std::sin(angle)*shot.vy)/std::hypot(shot.vx,shot.vy)>.9999f);
    }
    LevelScene encounter;
    assert(encounter.Load("port/tests/fixtures/encounter_open.xml",renderer,error));
    Combat actor; actor.Update(encounter,0,107.44937f,0,false,false,true,false,1,renderer);
    actor.SetArt(renderer); assert(actor.EnemyCount()==1);
    const auto& enemy=actor.Enemies().front(); renderer.sprites.clear();
    actor.Draw(renderer,enemy.x,enemy.y); assert(renderer.sprites.size()==1);
    const auto& body=renderer.sprites.front(); assert(!body.flipY); // spawners FlipVertical -> inverse SDL V
    assert(std::abs(body.destination.x+body.destination.width/2-640.f)<.001f);
    assert(std::abs(body.destination.y+body.destination.height/2-360.f)<.001f);
    Combat blood; blood.SetArt(renderer); blood.SpawnIntroBloodFlight(0,0);
    blood.UpdateBloodFlights(empty,.15f); renderer.sprites.clear(); blood.Draw(renderer,0,0);
    assert(renderer.sprites.size()==1 && renderer.sprites.front().flipY);
    std::cout << "PASS: player knife fade-in, eight-direction forward tips, actor/blood V, trajectory pitch, active-flight bound and solid contact\n";
}

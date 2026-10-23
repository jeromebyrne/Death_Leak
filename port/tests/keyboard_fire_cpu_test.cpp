#include "../render/Renderer.h"
#include "../game/Combat.h"
#include <cassert>
#include <iostream>
#include <string>

using namespace deathleak;
class InputSpy final : public render::Renderer {
public:
    bool down = false, fire = false;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string&, std::string&) override { return ++texture; }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite&) override {}
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int key) const override {
        return key == render::Down ? down : key == render::Fire && fire;
    }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
private:
    render::TextureId texture = 0;
};
int main() {
    InputSpy input;
    auto resolve = [&](bool scripted = false, char command = '.', bool grounded = true,
                       bool stick = false) {
        return render::ResolveGameplayAttackInput(scripted, command, input,
            grounded, false, false, false, stick);
    };
    const game::LevelScene scene;
    input.fire = true;
    auto attack = resolve();
    assert(!attack.crouch && attack.fire);
    game::Combat combat;
    const int before = combat.Fired();
    combat.Update(scene, 0, 0, 1.f/60.f, attack.fire, false, true, false,
                  1, input);
    assert(combat.Fired() == before + 1 && combat.ShotCount() == 1);

    input.fire = false; input.down = true;
    attack = resolve();
    assert(attack.crouch && !attack.fire); // S/Down never shoots on ground alone.
    attack = resolve(false, '.', false);
    assert(!attack.crouch && attack.fire); // Existing airborne Down shot.
    input.down = false;
    attack = resolve(false, '.', true, true);
    assert(!attack.crouch && attack.fire); // Right-stick fire.
    attack = resolve(true, 'F');
    assert(!attack.crouch && attack.fire); // Scripted F remains independent.
    input.down = true; input.fire = true;
    attack = resolve();
    assert(attack.crouch && attack.fire); // Crouched keyboard fire is possible.
    std::cout << "keyboard fire CPU: grounded knife fired, crouch/air/stick/script passed\n";
}

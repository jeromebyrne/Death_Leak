// Run once per XML ID with DEATHLEAK_RENDER_ONLY_XML_IDS set to that ID.
#include "../game/LevelScene.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::vector<deathleak::render::Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(deathleak::render::Color) override {}
    void DrawSprite(const deathleak::render::Sprite& sprite) override { drawn.push_back(sprite); }
    void FillRect(deathleak::render::Rect, deathleak::render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    deathleak::render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};

int main(int argc, char** argv) {
    assert(argc == 6);
    const char* selected = std::getenv("DEATHLEAK_RENDER_ONLY_XML_IDS");
    assert(selected && std::string(selected) == argv[1]);
    CaptureRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    const float x = std::stof(argv[2]), y = std::stof(argv[3]);
    scene.Draw(renderer, x, y, x, y);
    assert(renderer.drawn.size() == 1);
    const auto& sprite = renderer.drawn.front();
    assert(sprite.flipX == (std::string(argv[4]) == "true"));
    assert(sprite.flipY); // Authored verticalflip=false maps to SDL's inverted V convention.
    assert(std::fabs(sprite.rotationDegrees - std::stof(argv[5])) < .002f);
    assert(std::fabs(sprite.destination.x + sprite.destination.width / 2 - 640.f) < .01f);
    std::cout << "PASS: XML ID " << selected << " horizontal reflection and centered rotation\n";
}

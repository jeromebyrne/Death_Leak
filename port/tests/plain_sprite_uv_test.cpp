// D3D's upward-Y Sprite vertices put UV V=0 on the lower world edge.
#include "../game/LevelScene.h"
#include <cassert>
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

int main() {
    CaptureRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", renderer, error));
    auto sample = [&](const char* id, bool expected) {
        renderer.drawn.clear();
        scene.Draw(renderer, 0.f, 3400.f, 0.f, 3400.f);
        assert(renderer.drawn.size() == 1);
        assert(renderer.drawn[0].flipY == expected);
    };
    const char* id = std::getenv("DEATHLEAK_RENDER_ONLY_XML_IDS");
    assert(id);
    sample(id, std::string(id) == "17" || std::string(id) == "19" ||
           std::string(id) == "37" || std::string(id) == "122");
    std::cout << "PASS: forest sprite UV orientation ID " << id << '\n';
}

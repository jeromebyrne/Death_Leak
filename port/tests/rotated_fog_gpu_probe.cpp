// Physical-display diagnostic: invoke with capture path and camera world Y.
#include "../game/LevelScene.h"
#include <cstdlib>
#include <iostream>
#include <memory>

int main(int argc, char** argv) {
    if (argc != 3) return 2;
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "117", 1);
    auto renderer = deathleak::render::CreateRenderer();
    std::string error;
    if (!renderer->Initialize("Rotated fog cull probe", 1280, 720, error)) {
        std::cerr << error << '\n'; return 1;
    }
    deathleak::game::LevelScene scene;
    if (!scene.Load("Game/XmlFiles/levels/waterfall_top.xml", *renderer, error)) {
        std::cerr << error << '\n'; return 1;
    }
    constexpr float x = -1675.014648f, y = 2873.f;
    for (int i = 0; i < 6; ++i) scene.UpdateVisuals(1.f, x, y, x, y);
    renderer->BeginFrame({0, 0, 0, 255});
    scene.Draw(*renderer, x, std::atof(argv[2]), x, y);
    if (!renderer->SaveFrame(argv[1], error)) {
        std::cerr << error << '\n'; return 1;
    }
    renderer->EndFrame();
    std::cout << "Captured ID117 at cameraY=" << argv[2] << " to " << argv[1] << '\n';
}

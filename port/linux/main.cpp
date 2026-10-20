#include "../render/Renderer.h"
#include "../game/LevelScene.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace deathleak::render;

int main(int argc, char** argv) {
    int frameLimit = 0;
    std::string capture;
    std::string levelPreview;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--frames" && i + 1 < argc) frameLimit = std::atoi(argv[++i]);
        else if (arg == "--capture" && i + 1 < argc) capture = argv[++i];
        else if (arg == "--level-preview" && i + 1 < argc) levelPreview = argv[++i];
        else { std::cerr << "Usage: deathleak-linux [--frames N] [--capture output.bmp] [--level-preview level.xml]\n"; return 2; }
    }
    auto renderer = CreateRenderer();
    std::string error;
    if (!renderer->Initialize("Death Leak - Linux renderer slice", 1280, 720, error)) {
        std::cerr << "Renderer initialization failed: " << error << '\n'; return 1;
    }
    deathleak::game::LevelScene scene;
    if (!levelPreview.empty() && !scene.Load(levelPreview, *renderer, error)) {
        std::cerr << error << '\n'; return 1;
    }
    if (!levelPreview.empty()) {
        std::cout << "Loaded " << scene.SpriteCount() << " original level sprites; "
                  << scene.SkippedCount() << " missing or unsupported assets\n";
    }
    auto background = levelPreview.empty() ? renderer->LoadTexture("Game/Media/UI/main_menu_2.png", error) : InvalidTexture;
    if (levelPreview.empty() && !background) { std::cerr << "Background load failed: " << error << '\n'; return 1; }
    auto player = levelPreview.empty() ? renderer->LoadTexture("Game/Media/UI/player_silhouette.png", error) : InvalidTexture;
    if (levelPreview.empty() && !player) { std::cerr << "Player art load failed: " << error << '\n'; return 1; }
    float playerX = 590, playerY = 430;
    auto last = std::chrono::steady_clock::now();
    for (int frame = 0; !frameLimit || frame < frameLimit; ++frame) {
        if (renderer->PollQuit()) break;
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;
        if (dt > 0.05f) dt = 0.05f;
        if (renderer->KeyDown(Left)) playerX -= 250 * dt;
        if (renderer->KeyDown(Right)) playerX += 250 * dt;
        if (renderer->KeyDown(Up)) playerY -= 250 * dt;
        if (renderer->KeyDown(Down)) playerY += 250 * dt;
        renderer->BeginFrame({8, 12, 20, 255});
        if (levelPreview.empty()) {
            renderer->DrawSprite({background, {}, {0, 0, 1280, 720}});
            renderer->DrawSprite({player, {}, {playerX, playerY, 100, 170}});
        } else {
            scene.Draw(*renderer, scene.CameraX() + (playerX - 590) * 1.5f,
                       scene.CameraY() + (430 - playerY) * 1.5f);
        }
        if (!capture.empty() && frame == frameLimit - 1 && !renderer->SaveFrame(capture, error)) {
            std::cerr << "Capture failed: " << error << '\n'; return 1;
        }
        renderer->EndFrame();
    }
    return 0;
}

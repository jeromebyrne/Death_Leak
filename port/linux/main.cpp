#include "../render/Renderer.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace deathleak::render;

int main(int argc, char** argv) {
    int frameLimit = 0;
    std::string capture;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--frames" && i + 1 < argc) frameLimit = std::atoi(argv[++i]);
        else if (arg == "--capture" && i + 1 < argc) capture = argv[++i];
        else { std::cerr << "Usage: deathleak-linux [--frames N] [--capture output.bmp]\n"; return 2; }
    }
    auto renderer = CreateRenderer();
    std::string error;
    if (!renderer->Initialize("Death Leak - Linux renderer slice", 1280, 720, error)) {
        std::cerr << "Renderer initialization failed: " << error << '\n'; return 1;
    }
    auto background = renderer->LoadTexture("Game/Media/UI/main_menu_2.png", error);
    if (!background) { std::cerr << "Background load failed: " << error << '\n'; return 1; }
    auto player = renderer->LoadTexture("Game/Media/UI/player_silhouette.png", error);
    if (!player) { std::cerr << "Player art load failed: " << error << '\n'; return 1; }
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
        renderer->DrawSprite({background, {}, {0, 0, 1280, 720}});
        renderer->DrawSprite({player, {}, {playerX, playerY, 100, 170}});
        if (!capture.empty() && frame == frameLimit - 1 && !renderer->SaveFrame(capture, error)) {
            std::cerr << "Capture failed: " << error << '\n'; return 1;
        }
        renderer->EndFrame();
    }
    return 0;
}

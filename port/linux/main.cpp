#include "../render/Renderer.h"
#include "../game/LevelScene.h"
#include <chrono>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace deathleak::render;

int main(int argc, char** argv) {
    int frameLimit = 0;
    std::string capture;
    std::string levelPreview;
    std::string playLevel;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--frames" && i + 1 < argc) frameLimit = std::atoi(argv[++i]);
        else if (arg == "--capture" && i + 1 < argc) capture = argv[++i];
        else if (arg == "--level-preview" && i + 1 < argc) levelPreview = argv[++i];
        else if (arg == "--play" && i + 1 < argc) playLevel = argv[++i];
        else { std::cerr << "Usage: deathleak-linux [--frames N] [--capture output.bmp] [--level-preview level.xml | --play level.xml]\n"; return 2; }
    }
    if (!playLevel.empty() && !levelPreview.empty()) { std::cerr << "Choose either --play or --level-preview\n"; return 2; }
    auto renderer = CreateRenderer();
    std::string error;
    if (!renderer->Initialize("Death Leak - Linux renderer slice", 1280, 720, error)) {
        std::cerr << "Renderer initialization failed: " << error << '\n'; return 1;
    }
    deathleak::game::LevelScene scene;
    const std::string levelPath = !playLevel.empty() ? playLevel : levelPreview;
    if (!levelPath.empty() && !scene.Load(levelPath, *renderer, error)) {
        std::cerr << error << '\n'; return 1;
    }
    if (!levelPath.empty()) {
        std::cout << "Loaded " << scene.SpriteCount() << " original level sprites; "
                  << scene.SkippedCount() << " missing or unsupported assets; "
                  << scene.Ground().size() << " ground segments; " << scene.Triggers().size() << " level triggers\n";
    }
    auto background = levelPath.empty() ? renderer->LoadTexture("Game/Media/UI/main_menu_2.png", error) : InvalidTexture;
    if (levelPath.empty() && !background) { std::cerr << "Background load failed: " << error << '\n'; return 1; }
    auto player = levelPath.empty() ? renderer->LoadTexture("Game/Media/UI/player_silhouette.png", error) : InvalidTexture;
    if (levelPath.empty() && !player) { std::cerr << "Player art load failed: " << error << '\n'; return 1; }
    float playerX = 590, playerY = 430;
    float worldX = scene.PlayerX(), worldY = scene.PlayerY(), velocityX = 0, velocityY = 0;
    float cameraX = scene.CameraX(), cameraY = scene.CameraY();
    bool grounded = false, jumpHeld = false;
    std::string currentLevel = playLevel;
    auto start = std::chrono::steady_clock::now();
    auto last = std::chrono::steady_clock::now();
    for (int frame = 0; !frameLimit || frame < frameLimit; ++frame) {
        if (renderer->PollQuit()) break;
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;
        if (dt > 0.05f) dt = 0.05f;
        if (renderer->KeyDown(Escape)) break;
        if (playLevel.empty()) {
            if (renderer->KeyDown(Left)) playerX -= 250 * dt;
            if (renderer->KeyDown(Right)) playerX += 250 * dt;
            if (renderer->KeyDown(Up)) playerY -= 250 * dt;
            if (renderer->KeyDown(Down)) playerY += 250 * dt;
        } else {
            const float direction = static_cast<float>(renderer->KeyDown(Right)) - static_cast<float>(renderer->KeyDown(Left));
            velocityX = std::clamp(velocityX + direction * 1700.f * dt, -470.f, 470.f);
            if (direction == 0) velocityX *= std::max(0.f, 1.f - 9.f * dt);
            const bool jump = renderer->KeyDown(Up);
            if (jump && !jumpHeld && grounded) { velocityY = 790.f; grounded = false; }
            jumpHeld = jump;
            float oldFeet = worldY - 107.f;
            worldX += velocityX * dt;
            worldY += velocityY * dt - 0.5f * 1900.f * dt * dt;
            velocityY -= 1900.f * dt;
            grounded = false;
            float bestGround = -1e9f;
            for (const auto& line : scene.Ground()) {
                const float dx = line.x2 - line.x1;
                if (std::abs(dx) < 0.01f) continue;
                const float t = (worldX - line.x1) / dx;
                if (t < 0.f || t > 1.f) continue;
                const float surface = line.y1 + t * (line.y2 - line.y1);
                if (velocityY <= 0 && oldFeet >= surface - 4.f && worldY - 107.f <= surface && surface > bestGround)
                    bestGround = surface;
            }
            if (bestGround > -1e8f) { worldY = bestGround + 107.f; velocityY = 0; grounded = true; }
            cameraX += (worldX - cameraX) * std::min(1.f, 8.f * dt);
            cameraY += (worldY - cameraY) * std::min(1.f, 4.f * dt);
            for (const auto& trigger : scene.Triggers()) {
                if (std::abs(worldX - trigger.x) < trigger.width * 0.5f &&
                    std::abs(worldY - trigger.y) < trigger.height * 0.5f && trigger.destination != currentLevel) {
                    if (scene.Load(trigger.destination, *renderer, error, currentLevel)) {
                        currentLevel = trigger.destination;
                        worldX = scene.PlayerX(); worldY = scene.PlayerY();
                        cameraX = scene.CameraX(); cameraY = scene.CameraY();
                        velocityX = velocityY = 0; grounded = false;
                        std::cout << "Entered " << currentLevel << '\n';
                    } else std::cerr << "Transition failed: " << error << '\n';
                    break;
                }
            }
        }
        renderer->BeginFrame({8, 12, 20, 255});
        if (levelPath.empty()) {
            renderer->DrawSprite({background, {}, {0, 0, 1280, 720}});
            renderer->DrawSprite({player, {}, {playerX, playerY, 100, 170}});
        } else if (playLevel.empty())
            scene.Draw(*renderer, scene.CameraX() + (playerX - 590) * 1.5f,
                       scene.CameraY() + (430 - playerY) * 1.5f, scene.PlayerX(), scene.PlayerY());
        else scene.Draw(*renderer, cameraX, cameraY, worldX, worldY);
        if (!capture.empty() && frame == frameLimit - 1 && !renderer->SaveFrame(capture, error)) {
            std::cerr << "Capture failed: " << error << '\n'; return 1;
        }
        renderer->EndFrame();
    }
    if (!playLevel.empty()) {
        const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        std::cout << "Simulation ended at " << currentLevel << " player=(" << worldX << "," << worldY
                  << ") elapsed=" << elapsed << "s\n";
    }
    return 0;
}

// Actual forest XML render-command regression for AnimationPart's prior-render clock.
#include "../game/LevelScene.h"
#include <array>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class Spy final : public deathleak::render::Renderer {
public:
    std::unordered_map<std::string, deathleak::render::TextureId> ids;
    std::unordered_map<deathleak::render::TextureId, std::string> paths;
    std::vector<std::string> commands;
    int aimAlpha = -1;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string& path, std::string&) override {
        auto [it, inserted] = ids.try_emplace(path, static_cast<deathleak::render::TextureId>(ids.size()+1));
        paths[it->second] = path;
        return it->second;
    }
    void BeginFrame(deathleak::render::Color) override {}
    void DrawSprite(const deathleak::render::Sprite& sprite) override {
        const auto& path = paths.at(sprite.texture);
        commands.push_back(path);
        if (path == "Game/Media/aim_line.png") aimAlpha = sprite.tint.a;
    }
    void FillRect(deathleak::render::Rect, deathleak::render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    deathleak::render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};

int main() {
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "2,68,__player_aim_line", 1);
    Spy spy;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml", spy, error));
    const float x = scene.PlayerX(), y = scene.PlayerY();
    constexpr std::array<float,20> dt{.016f,.016f,.016f,.016f,.016f,.016f,.016f,.016f,.016f,.012f,.008f,.012f,.012f,.012f,.012f,.016f,.016f,.016f,.008f,.012f};
    constexpr std::array<int,20> body{0,0,0,0,1,1,1,2,2,2,3,0,0,0,1,0,0,0,1,1};
    constexpr std::array<int,20> orb{0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2};
    float previous = 0.f;
    for (int tick=0; tick<20; ++tick) {
        const bool running = tick>=11 && tick<=14;
        const float camera = (tick==6 || tick==7) ? 10000.f : x;
        scene.UpdateVisuals(dt[tick], x,y,camera,y,false,previous);
        scene.UpdatePlayerVisual(dt[tick],running ? 360.f : 0.f,0.f,true,0,false,false,false,0.f,1,false,false,{},false,0.f,0.f,previous);
        spy.commands.clear();
        scene.Draw(spy,x,y,x,y);
        std::string actualBody, actualOrb;
        for (const auto& path : spy.commands) {
            if (path.find("/idle_default/idle_default_") != std::string::npos || path.find("/run_default/run_default_") != std::string::npos) actualBody=path;
            if (path.find("Game/Media/orb/") == 0) actualOrb=path;
        }
        const auto index = body[tick];
        const std::string expectedBody = "Game/Media/characters/player/" + std::string(running ? "run_default/run_default_" : "idle_default/idle_default_") + (index < 10 ? "00" : "0") + std::to_string(index) + ".png";
        const std::string expectedOrb = "Game/Media/orb/" + std::to_string(1+10*orb[tick]) + ".png";
        if (actualBody != expectedBody || actualOrb != expectedOrb) {
            std::cerr << "tick " << tick << " dt=" << dt[tick] << " prior=" << previous << " body " << actualBody << " expected " << expectedBody << " orb " << actualOrb << " expected " << expectedOrb << '\n';
            return 1;
        }
        spy.EndFrame();
        previous = dt[tick]; // process slot changes only at the completed-render boundary
    }
    // A successful shot advances its finite 30-fps arm twice using the same prior slot.
    scene.UpdatePlayerVisual(.001f,0,0,true,0,false,false,false,0,1,false,false,{},false,0,0,.001f);
    spy.commands.clear(); scene.Draw(spy,x,y,x,y); spy.EndFrame();
    scene.UpdatePlayerVisual(.0166667f,0,0,true,0,false,false,false,0,1,true,false,{},false,0,0,.001f);
    spy.commands.clear(); scene.Draw(spy,x,y,x,y);
    bool arm000=false;
    for (const auto& path : spy.commands) if (path == "Game/Media/characters/player/back_arm_attack/back_arm_attack_000.png") arm000=true;
    assert(arm000);
    // Non-animation aim-line hold/fade must continue consuming current dt.
    for (int i=0; i<4; ++i) {
        scene.UpdatePlayerVisual(.02f,0,0,true,0,false,false,false,0,1,false,i==0,{},false,0,0,0.f);
        spy.commands.clear(); spy.aimAlpha=-1; scene.Draw(spy,x,y,x,y);
        assert(spy.aimAlpha == (i==3 ? 204 : 255));
    }
    std::cout << "PASS: forest ID2 body/arm and ID68 preceding-rendered-delta command clock\n";
}

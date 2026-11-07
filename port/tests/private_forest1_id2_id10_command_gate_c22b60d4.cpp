// CPU-only production LevelScene draw-command gate. Run from source/.
#include "../game/LevelScene.h"
#include "../../Game/tinyxml/tinyxml.h"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
namespace render = deathleak::render;
struct Spy final : render::Renderer {
    struct Command { std::string path; render::Sprite sprite; };
    std::unordered_map<std::string, render::TextureId> ids;
    std::unordered_map<render::TextureId, std::string> paths;
    std::vector<Command> commands;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path, std::string&) override {
        auto [it, inserted] = ids.try_emplace(path, static_cast<render::TextureId>(ids.size() + 1));
        paths[it->second] = path; return it->second;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite& sprite) override { commands.push_back({paths.at(sprite.texture), sprite}); }
    void FillRect(render::Rect, render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
int main() {
    constexpr char level[] = "Game/XmlFiles/levels/forest_exploration_1.xml";
    TiXmlDocument document(level);
    assert(document.LoadFile());
    const TiXmlElement *player=nullptr, *tree=nullptr;
    for (auto* e=document.RootElement()->FirstChildElement(); e; e=e->NextSiblingElement()) {
        if (!e->Attribute("id")) continue;
        if (std::string(e->Attribute("id"))=="2") player=e;
        if (std::string(e->Attribute("id"))=="10") tree=e;
    }
    assert(player && tree);
    assert(std::string(player->FirstChildElement("position")->Attribute("depth_layer"))=="kPlayer"); // 3000
    assert(std::string(tree->FirstChildElement("position")->Attribute("depth_layer"))=="kMiddleForeground"); // 1000
    assert(std::string(tree->FirstChildElement("texture")->Attribute("filename"))=="Media/objects/tree2_blur.png");
    assert(std::string(tree->FirstChildElement("horizontalflip")->Attribute("value"))=="true");
    assert(std::string(tree->FirstChildElement("verticalflip")->Attribute("value"))=="false");
    assert(std::string(tree->FirstChildElement("fade_when_player_occluded")->Attribute("value"))=="false");
    double authoredAlpha=0;
    assert(tree->FirstChildElement("alpha")->QueryDoubleAttribute("value",&authoredAlpha)==TIXML_SUCCESS);
    assert(std::fabs(authoredAlpha-.7)<1e-6);
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS","2,10",1);
    Spy spy; deathleak::game::LevelScene scene; std::string error;
    if (!scene.Load(level,spy,error)) { std::cerr<<error<<'\n'; return 1; }
    constexpr float x=2171.985352f,y=3375.f,dt=1.f/60.f;
    scene.UpdateVisuals(dt,x,y,x,y); // player center inside ID10 rectangle
    scene.Draw(spy,x,y,x,y);
    size_t bodyIndex=spy.commands.size(),treeIndex=spy.commands.size();
    for (size_t i=0;i<spy.commands.size();++i) {
        const auto& path=spy.commands[i].path;
        if (path.find("Game/Media/characters/player/idle_default/idle_default_")==0) bodyIndex=i;
        if (path=="Game/Media/objects/tree2_blur.png") treeIndex=i;
    }
    assert(bodyIndex<spy.commands.size() && treeIndex<spy.commands.size());
    assert(bodyIndex<treeIndex);
    const auto& command=spy.commands[treeIndex].sprite;
    assert(command.flipX && command.flipY); // SDL V convention inverts XML flipY=false.
    assert(std::fabs(command.opacity-.7f)<1e-6f);
    assert(command.tint.a==179); // round(.7*255); ordinary SDL copy uses tint alpha once.
    std::cout<<"PASS: Forest-1 ID2 before ID10; texture/flip; no occlusion fade; ID10 opacity=.7 tint.a=179\n";
}

#include "../game/LevelScene.h"
#include "../game/PlayerMotion.h"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <unordered_map>

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::unordered_map<std::string, deathleak::render::TextureId> textures;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string& path, std::string&) override {
        return textures.try_emplace(path, static_cast<deathleak::render::TextureId>(textures.size()+1)).first->second;
    }
    void BeginFrame(deathleak::render::Color) override {}
    void DrawSprite(const deathleak::render::Sprite&) override {}
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
    constexpr float dt = 1.f/60.f;
    auto run = [&](LevelScene& scene, bool expectsSpray) {
        const auto& line = scene.Ground().front();
        const float x = (line.x1+line.x2)*.5f;
        const float surface = (line.y1+line.y2)*.5f;
        const float y = surface + 214.898743f*.5f;
        PlayerMotion motion;
        motion.Reset(x,y,true);
        motion.Update(scene,dt,0,false);
        assert(motion.Grounded());
        const float beforeX = motion.X(), beforeY = motion.Y();
        assert(scene.JumpTakeoffSprays().empty());
        motion.Update(scene,dt,0,true);
        assert(!motion.Grounded());
        assert((scene.JumpTakeoffSprays().size()==1)==expectsSpray);
        if (expectsSpray) {
            const auto& spray = scene.JumpTakeoffSprays().front();
            assert(spray.texture==renderer.textures.at("Game/Media/rock_particle.png"));
            assert(spray.depth==3000 && spray.particles.size()==10 && !spray.looping);
            assert(std::abs(spray.x-(beforeX+10.f))<.01f);
            assert(std::abs(spray.y-(beforeY+10.f-214.898743f*.5f+35.f))<.01f);
            assert(spray.width==1200.f && spray.height==720.f && spray.fadeIn==.15f && spray.fadeOut==.9f);
            assert(spray.scalesByLifetime && spray.scaleTo==5.f && spray.loopTime==1.f);
            for (int i=0;i<10;++i) {
                const auto& p=spray.particles[i];
                assert(p.directionX>=0 && p.directionX<.8f && std::abs(p.directionX+p.directionY-1.f)<.0001f);
                assert(std::abs(p.lifetime-(1.2f-.04f*i))<.0001f);
                assert(std::abs(p.startX-spray.x)<=50 && std::abs(p.startY-spray.y)<=2);
                assert(p.size==32.f && p.gravity==.8f && p.speed>=1.01f && p.speed<=1.5f);
                assert(p.brightness>=.81f && p.brightness<=1.f);
            }
        }
        for (int i=0;i<12;++i) motion.Update(scene,dt,0,true);
        assert(scene.JumpTakeoffSprays().size()==(expectsSpray?1u:0u));
    };
    LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/forest_exploration_1.xml",renderer,error));
    assert(scene.Ground().front().material=="grassground");
    run(scene,true);
    // Airborne reset has no retained solid-line pointer even when jump input is pressed.
    PlayerMotion airborne;
    const auto& line=scene.Ground().front();
    airborne.Reset((line.x1+line.x2)*.5f,(line.y1+line.y2)*.5f+400.f);
    airborne.Update(scene,dt,0,true);
    assert(scene.JumpTakeoffSprays().size()==1);
    // Leave the end of the same solid line, then jump during coyote time:
    // motion succeeds, but the source no longer has a retained line pointer.
    PlayerMotion coyote;
    const float edgeX=line.x2-.1f;
    const float edgeSurface=line.y2+(line.y1-line.y2)*(.1f/(line.x2-line.x1));
    coyote.Reset(edgeX,edgeSurface+214.898743f*.5f,true);
    coyote.Update(scene,dt,0,false);
    assert(coyote.Grounded());
    for (int i=0;i<12 && coyote.Grounded();++i) coyote.Update(scene,dt,1,false);
    assert(!coyote.Grounded());
    const float beforeCoyoteY=coyote.Y();
    coyote.Update(scene,dt,1,true);
    assert(coyote.Y()>beforeCoyoteY && scene.JumpTakeoffSprays().size()==1);
    // An authored line with no material texture still accepts a physical jump.
    const std::filesystem::path source("Game/XmlFiles/levels/forest_exploration_1.xml");
    std::ifstream input(source);
    assert(input);
    std::string xml((std::istreambuf_iterator<char>(input)),{});
    const auto at=xml.find("<material value=\"grassground\" />");
    assert(at!=std::string::npos);
    xml.replace(at,std::string("<material value=\"grassground\" />").size(),"<material value=\"untextured_jump_test\" />");
    const auto fixture=std::filesystem::temp_directory_path()/"deathleak_jump_no_texture.xml";
    { std::ofstream output(fixture); assert(output); output<<xml; }
    LevelScene untextured;
    assert(untextured.Load(fixture.string(),renderer,error));
    std::filesystem::remove(fixture);
    assert(untextured.Ground().front().material=="untextured_jump_test");
    run(untextured,false);
    // A retained material can be water without inheriting the Running-footstep
    // material-name veto. Motion's separate submerged gate decides jump acceptance.
    auto waterXml=xml;
    const auto waterAt=waterXml.find("<material value=\"untextured_jump_test\" />");
    assert(waterAt!=std::string::npos);
    waterXml.replace(waterAt,std::string("<material value=\"untextured_jump_test\" />").size(),
                     "<material value=\"water\" />");
    { std::ofstream output(fixture); assert(output); output<<waterXml; }
    LevelScene waterMaterial;
    assert(waterMaterial.Load(fixture.string(),renderer,error));
    std::filesystem::remove(fixture);
    waterMaterial.EmitJumpTakeoff(0,100.f,200.f,1);
    assert(waterMaterial.JumpTakeoffSprays().size()==1);
    assert(waterMaterial.JumpTakeoffSprays().front().texture==renderer.textures.at("Game/Media/large_splash.png"));
    // Put a WaterBlock over the starting line. A submerged held jump uses swim
    // acceleration, not Character::Jump, and cannot emit a takeoff burst.
    auto submergedXml=waterXml;
    const auto end=submergedXml.rfind("</level>");
    assert(end!=std::string::npos);
    submergedXml.insert(end,"<WaterBlock id=\"999999\"><position x=\""+
        std::to_string((line.x1+line.x2)*.5f)+"\" y=\""+
        std::to_string((line.y1+line.y2)*.5f+214.898743f*.5f)+
        "\" /><dimensions width=\"500\" height=\"500\" /></WaterBlock>\n");
    { std::ofstream output(fixture); assert(output); output<<submergedXml; }
    LevelScene submerged;
    assert(submerged.Load(fixture.string(),renderer,error));
    std::filesystem::remove(fixture);
    assert(!submerged.Hazards().empty() && submerged.Hazards().back().water);
    PlayerMotion swimming;
    swimming.Reset((line.x1+line.x2)*.5f,(line.y1+line.y2)*.5f+214.898743f*.5f,true);
    swimming.Update(submerged,dt,0,false);
    assert(swimming.Grounded() && swimming.InWater());
    swimming.Update(submerged,dt,0,true);
    assert(submerged.JumpTakeoffSprays().empty());
    std::cout<<"PASS: grounded takeoff, hold, coyote/airborne, no-texture, water material, submerged controls\n";
}

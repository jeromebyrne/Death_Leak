#include "../game/LevelScene.h"
#include "../game/PlayerMotion.h"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <unordered_map>
using namespace deathleak;
class CaptureRenderer final : public render::Renderer {
public:
    std::unordered_map<std::string,render::TextureId> textures;
    bool Initialize(const char*,int,int,std::string&) override { return true; }
    render::TextureId LoadTexture(const std::string& path,std::string&) override {
        return textures.try_emplace(path,static_cast<render::TextureId>(textures.size()+1)).first->second;
    }
    void BeginFrame(render::Color) override {}
    void DrawSprite(const render::Sprite&) override {}
    void FillRect(render::Rect,render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&,std::string&) override { return false; }
};
static constexpr float halfHeight=214.898743f*.5f;
static void run(CaptureRenderer& renderer,const std::string& path,float drop,int expected,int facing=1,
                bool authored=false,bool bufferRoll=false) {
    game::LevelScene scene; std::string error;
    assert(scene.Load(path,renderer,error));
    const auto& line=scene.Ground().front();
    const float x=(line.x1+line.x2)*.5f;
    const float t=(x-line.x1)/(line.x2-line.x1);
    const float floor=line.y1+t*(line.y2-line.y1);
    game::PlayerMotion motion;
    motion.Reset(x,floor+halfHeight+(authored ? 0.f : drop),authored);
    if (bufferRoll) { motion.SetMovementUnlocks(false,true); motion.BufferRoll(); }
    bool landed=false;
    for (int frame=0;frame<240;++frame) {
        motion.Update(scene,1.f/60.f,facing<0 && frame==0 ? -1 : 0,false);
        if (motion.Grounded()) { landed=true; break; }
        assert(scene.LandingSprays().empty());
    }
    assert(landed);
    assert(scene.LandingSprays().size()==(expected?1u:0u));
    if (expected) {
        const auto& spray=scene.LandingSprays().front();
        assert(spray.texture==renderer.textures.at("Game/Media/rock_particle.png"));
        assert(spray.depth==3000 && spray.particles.size()==static_cast<size_t>(expected));
        assert(std::abs(spray.x-(motion.X()+facing*10.f))<.01f);
        assert(std::abs(spray.y-(motion.Y()-halfHeight+(expected==5?35.f:0.f)))<.01f);
        assert(spray.width==1200 && spray.height==720 && !spray.looping && spray.warmed);
        assert(spray.fadeIn==.15f && spray.fadeOut==.9f && spray.scalesByLifetime);
        assert(spray.scaleTo==(expected==5?5.f:15.f) && spray.loopTime==1.f);
        for (int i=0;i<expected;++i) {
            const auto& p=spray.particles[i];
            assert(p.directionX>=0 && p.directionX<(expected==5?.8f:.4f));
            assert(std::abs(p.directionX+p.directionY-1.f)<.0001f);
            assert(std::abs(p.lifetime-(expected==5?1.2f-.4f*i/5:1.f-.6f*i/10))<.0001f);
            assert(std::abs(p.startX-spray.x)<=50 && std::abs(p.startY-spray.y)<=2);
            assert(p.size>=(expected==5?32.f:5.f) && p.size<=(expected==5?32.f:10.f));
            assert(p.speed>=(expected==5?1.01f:3.01f) && p.speed<=(expected==5?1.5f:5.f));
            assert(p.gravity==.8f && p.brightness>=.81f && p.brightness<=1.f);
        }
    }
    for (int i=0;i<5;++i) motion.Update(scene,1.f/60.f,0,false);
    assert(scene.LandingSprays().size()==(expected?1u:0u));
    assert(scene.JumpTakeoffSprays().empty() && scene.FootstepSprays().empty());
}
int main() {
    CaptureRenderer renderer;
    const std::string forest="Game/XmlFiles/levels/forest_exploration_1.xml";
    run(renderer,forest,299.9f,0);
    run(renderer,forest,300.f,5);
    run(renderer,forest,699.9f,5,-1);
    run(renderer,forest,700.f,10);
    run(renderer,forest,700.f,0,1,true);
    run(renderer,forest,700.f,10,1,false,true);
    std::ifstream in(forest); assert(in);
    std::string xml((std::istreambuf_iterator<char>(in)),{});
    const auto at=xml.find("<material value=\"grassground\" />"); assert(at!=std::string::npos);
    xml.replace(at,std::string("<material value=\"grassground\" />").size(),"<material value=\"missing_landing_material\" />");
    const auto fixture=std::filesystem::temp_directory_path()/"deathleak_landing_no_texture.xml";
    { std::ofstream out(fixture); out<<xml; }
    run(renderer,fixture.string(),700.f,0);
    std::filesystem::remove(fixture);
    // A box at the same top as the textured line wins the existing tie rule.
    std::ifstream original(forest); assert(original);
    std::string boxed((std::istreambuf_iterator<char>(original)),{});
    game::LevelScene source; std::string error; assert(source.Load(forest,renderer,error));
    const auto& line=source.Ground().front();
    const float boxX=(line.x1+line.x2)*.5f;
    const float boxY=(line.y1+line.y2)*.5f;
    const auto end=boxed.rfind("</level>"); assert(end!=std::string::npos);
    boxed.insert(end,"<SolidMovingSprite id=\"999999\"><position x=\""+std::to_string(boxX)+
        "\" y=\""+std::to_string(boxY-50.f)+
        "\"/><dimensions width=\"500\" height=\"100\"/><collisionboxdimensions x=\"500\" y=\"100\" offsetX=\"0\" offsetY=\"0\"/></SolidMovingSprite>\n");
    { std::ofstream out(fixture); out<<boxed; }
    run(renderer,fixture.string(),700.f,0);
    std::filesystem::remove(fixture);
    const auto ledgeFixture=std::filesystem::temp_directory_path()/"deathleak_landing_ledge.xml";
    { std::ofstream out(ledgeFixture);
      out<<"<level>\n<Player id=\"1\"><position x=\"-100\" y=\"807.44937\"/><dimensions width=\"181.393768\" height=\"215.352036\"/><texture filename=\"Media/UI/player_silhouette.png\"/></Player>\n"
            "<SolidLineStrip id=\"2\"><position x=\"-300\" y=\"700\"/><dimensions width=\"300\" height=\"1\"/><material value=\"grassground\"/><points num_points=\"2\" x_0=\"0\" y_0=\"0\" x_1=\"300\" y_1=\"0\"/></SolidLineStrip>\n"
            "<SolidLineStrip id=\"3\"><position x=\"0\" y=\"0\"/><dimensions width=\"1000\" height=\"1\"/><material value=\"grassground\"/><points num_points=\"2\" x_0=\"0\" y_0=\"0\" x_1=\"1000\" y_1=\"0\"/></SolidLineStrip>\n</level>\n"; }
    game::LevelScene ledge; assert(ledge.Load(ledgeFixture.string(),renderer,error));
    game::PlayerMotion walker; walker.Reset(-100.f,700.f+halfHeight,true);
    walker.Update(ledge,1.f/60.f,0,false); assert(walker.Grounded());
    bool leftEdge=false, reachedLower=false;
    for (int i=0;i<240;++i) {
        walker.Update(ledge,1.f/60.f,walker.X()<110.f?1:0,false);
        leftEdge |= !walker.Grounded();
        if (leftEdge && walker.Grounded()) { reachedLower=true; break; }
    }
    assert(leftEdge && reachedLower && ledge.LandingSprays().size()==1);
    assert(ledge.LandingSprays().front().particles.size()==10);
    std::filesystem::remove(ledgeFixture);
    std::cout<<"PASS: landing thresholds, variants, authored entry, roll buffer, absent texture\n";
}

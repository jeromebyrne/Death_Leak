#include "../game/Combat.h"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
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
static void tick(game::Combat& combat,const game::LevelScene& scene,CaptureRenderer& renderer,
                 float x,float y,float dt,bool fire=false) {
    combat.Update(scene,x,y,dt,fire,false,true,false,1,renderer);
}
int main() {
    CaptureRenderer renderer;
    game::LevelScene forest;
    std::string error;
    assert(forest.Load("Game/XmlFiles/levels/forest_exploration_1.xml",renderer,error));
    assert(!forest.Ground().empty() && forest.Ground().front().material=="grassground");
    const auto& line=forest.Ground().front(); // authored SolidLineStrip ID3
    const float x=(line.x1+line.x2)*.5f;
    const float floor=line.y1+(x-line.x1)/(line.x2-line.x1)*(line.y2-line.y1);
    const float y=floor-24.f; // knife starts one unit above line
    game::Combat hit;
    tick(hit,forest,renderer,x,y,0.f,true);
    assert(hit.ShotCount()==1 && hit.MaterialSprayCount()==0);
    tick(hit,forest,renderer,x,y,.1f);
    assert(hit.ShotCount()==0 && hit.MaterialSprayCount()==1);
    const auto& spray=*hit.MaterialSpray(0);
    assert(spray.texture==renderer.textures.at("Game/Media/rock_particle.png"));
    assert(spray.depth==2500 && spray.particles.size()==10);
    assert(spray.width==3200.f && spray.height==1200.f);
    assert(!spray.looping && spray.warmed && spray.loopTime==10.f);
    assert(spray.fadeIn==.15f && spray.fadeOut==.6f && spray.scalesByLifetime && spray.scaleTo==3.5f);
    // The source subtracts line.Position()-rayEnd from line.Position(), so
    // emitter origin is untruncated ray end, not swept intersection.
    assert(std::abs(spray.x-(x+10.f+120.f+42.25f))<1.f);
    assert(spray.y<floor);
    for (size_t i=0;i<spray.particles.size();++i) {
        const auto& p=spray.particles[i];
        assert(p.lifetime==.75f-.04f*static_cast<float>(i));
        assert(p.speed>=1.01f && p.speed<=4.f);
        assert(p.size>=11.f && p.size<=30.f);
        assert(p.gravity==.7f && p.brightness>=.7f && p.brightness<=1.f);
        assert(p.startX>=spray.x-15.f && p.startX<=spray.x+15.f);
        assert(p.startY>=spray.y-30.f && p.startY<=spray.y+30.f);
        assert(p.directionX>=-1.f && p.directionX<-.6f);
        assert(std::abs(p.directionX+p.directionY+1.f)<.0001f);
        assert(p.alpha==0.f);
    }
    tick(hit,forest,renderer,x,y,.1f);
    assert(hit.MaterialSprayCount()==1 && hit.ShotCount()==0);
    game::Combat miss;
    tick(miss,forest,renderer,x,y+100.f,0.f,true);
    tick(miss,forest,renderer,x,y+100.f,.1f);
    assert(miss.MaterialSprayCount()==0 && miss.ShotCount()==1);

    game::LevelScene box;
    assert(box.Load("port/tests/fixtures/solid_box.xml",renderer,error));
    game::Combat boxed;
    tick(boxed,box,renderer,0.f,200.f,0.f,true);
    tick(boxed,box,renderer,0.f,200.f,.15f);
    assert(boxed.ShotCount()==0 && boxed.MaterialSprayCount()==0);

    // Synthetic encounter places a living skeleton above a grassground strip.
    // It tests knife pass-through and its own non-player projectile contact.
    const auto fixture=std::filesystem::temp_directory_path()/"deathleak_knife_line_spray_cpu.xml";
    {
        std::ofstream out(fixture);
        out<<"<level>\n"
              "<Player id=\"1\"><position x=\"0\" y=\"0\"/><dimensions width=\"180\" height=\"215\"/><texture filename=\"Media/UI/player_silhouette.png\"/></Player>\n"
              "<SolidLineStrip id=\"3\"><position x=\"0\" y=\"200\"/><dimensions width=\"1000\" height=\"1\"/><material value=\"grassground\"/><points num_points=\"2\" x_0=\"-500\" y_0=\"0\" x_1=\"500\" y_1=\"0\"/></SolidLineStrip>\n"
              "<NPCTrigger id=\"9\"><position x=\"0\" y=\"100\"/><dimensions width=\"100\" height=\"400\"/><num_enemies value=\"1\" enemy_type=\"kSkeleton\"/></NPCTrigger>\n"
              "</level>\n";
    }
    game::LevelScene encounter;
    assert(encounter.Load(fixture.string(),renderer,error));
    std::filesystem::remove(fixture);
    game::Combat enemy;
    tick(enemy,encounter,renderer,0.f,175.f,0.f,true); // blade starts at y=200
    assert(enemy.EnemyCount()==1 && enemy.ShotCount()==1);
    tick(enemy,encounter,renderer,0.f,175.f,.1f);
    assert(enemy.MaterialSprayCount()==0 && enemy.ShotCount()==1); // passes line
    // A second encounter keeps the player below the line while enemy AI fires.
    game::Combat hostile;
    tick(hostile,encounter,renderer,0.f,0.f,0.f);
    assert(hostile.EnemyCount()==1);
    bool sawHostile=false, sawContact=false;
    for (int i=0;i<300;++i) {
        const auto before=hostile.ShotCount();
        const float previousY = before ? hostile.Shots().front().y : 0.f;
        tick(hostile,encounter,renderer,0.f,0.f,1.f/60.f);
        for (const auto& shot:hostile.Shots()) if (shot.enemy) sawHostile=true;
        if (sawHostile && before>0 && hostile.ShotCount()<before &&
            previousY>=200.f && previousY<210.f) sawContact=true;
        assert(hostile.MaterialSprayCount()==0);
        if (sawContact) break;
    }
    assert(sawHostile && sawContact);
    std::cout<<"PASS: forest ID3 knife spray, no duplicate/noncontact, enemy pass-through and hostile line contact\n";
}

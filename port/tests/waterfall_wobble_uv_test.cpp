#include "../game/LevelScene.h"
#include "../linux/WobbleUV.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

using deathleak::render::Sprite;
using deathleak::render::WobbleUV;
using deathleak::render::PixelWobbleVertexUV;

class CaptureRenderer final : public deathleak::render::Renderer {
public:
    std::vector<Sprite> drawn;
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    deathleak::render::TextureId LoadTexture(const std::string&, std::string&) override { return 1; }
    void BeginFrame(deathleak::render::Color) override {}
    void DrawSprite(const Sprite& s) override { drawn.push_back(s); }
    void FillRect(deathleak::render::Rect, deathleak::render::Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    deathleak::render::MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
static bool near(float a, float b, float tolerance=1e-5f) { return std::fabs(a-b)<tolerance; }
static float fract(float value) { return value-std::floor(value); }
static WobbleUV sampleUV(WobbleUV uv, float time, float speed, float intensity) {
    uv.v += std::sin(uv.u*time*speed)*intensity;
    uv.u += std::sin(uv.v*time*speed)*intensity;
    return {fract(uv.u), fract(1.f-uv.v)};
}
int main() {
    setenv("DEATHLEAK_RENDER_ONLY_XML_IDS", "13,14", 1);
    CaptureRenderer renderer;
    deathleak::game::LevelScene scene;
    std::string error;
    assert(scene.Load("Game/XmlFiles/levels/waterfall_top.xml", renderer, error));
    auto draw = [&] {
        renderer.drawn.clear();
        scene.Draw(renderer, 300.f, 2600.f, 300.f, 2600.f);
        assert(renderer.drawn.size()==2);
        return renderer.drawn;
    };
    const auto zero=draw();
    const auto redraw=draw();
    scene.UpdateVisuals(1.f/60.f,300.f,2600.f,300.f,2600.f);
    const auto one=draw();
    for (int tick=1;tick<60;++tick) scene.UpdateVisuals(1.f/60.f,300.f,2600.f,300.f,2600.f);
    const auto sixty=draw();
    assert(!near(zero[0].destination.y,zero[1].destination.y)); // two distinct authored IDs
    for (size_t i=0;i<2;++i) {
        const auto& a=zero[i]; const auto& b=one[i]; const auto& c=sixty[i];
        assert(a.pixelWobble.enabled && near(a.pixelWobble.intensity,.005f));
        assert(near(a.pixelWobble.speedMod,10.f) && a.flipX && !a.flipY);
        assert(near(a.opacity,.3f) && near(a.uvShiftX,0.f));
        assert(near(a.uvShiftY,0.f) && near(redraw[i].uvShiftY,0.f));
        assert(near(b.uvShiftY,11.f*(2.f/3.f)*.001f));
        assert(near(c.uvShiftY,.44f,1e-4f));
        assert(a.destination.x==b.destination.x && a.destination.y==b.destination.y);
        assert(a.destination.width==c.destination.width && a.destination.height==c.destination.height);
        assert(near(b.opacity,a.opacity) && near(c.opacity,a.opacity));
        // This is the exact helper used by the GL quad: positive phase reaches
        // the shader input before both ordered wobble operations.
        const auto top=PixelWobbleVertexUV(a.flipX,a.flipY,b.uvShiftX,b.uvShiftY,false,false);
        const auto bottom=PixelWobbleVertexUV(a.flipX,a.flipY,b.uvShiftX,b.uvShiftY,false,true);
        assert(near(top.u,1.f) && near(top.v,1.f+b.uvShiftY));
        assert(near(bottom.v,b.uvShiftY));
        for (float screenDown : {0.f,.5f,1.f}) {
            WobbleUV input{top.u,top.v+(bottom.v-top.v)*screenDown};
            assert(near(sampleUV(input,3.f,10.f,0.f).v,fract(screenDown-b.uvShiftY)));
        }
        WobbleUV input{.37f,.63f+b.uvShiftY};
        const auto before=sampleUV(input,3.f,10.f,.005f);
        const auto after=sampleUV({input.u,input.v-b.uvShiftY},3.f,10.f,.005f);
        // A post-wobble-only phase leaves U unchanged; source phase changes
        // the second sine because V has already been scrolled.
        assert(std::fabs(before.u-after.u)>1e-4f);
    }
    std::cout << "PASS: IDs13/14 wobble shader vertex UV receives downward V phase before ordered wobble\n";
}

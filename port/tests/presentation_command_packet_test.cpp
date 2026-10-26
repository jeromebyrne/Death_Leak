#include "../linux/PresentationCommandPacket.h"
#include "../linux/RefreshPresentationService.h"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

using namespace deathleak;
using namespace deathleak::render;
using namespace deathleak::linux_port;

struct Spy final : Renderer {
    std::vector<std::string> calls;
    std::vector<float> spriteXs;
    bool surfaceStable = true;
    mutable int inputReads = 0;
    int eventPolls = 0;
    int textureLoads = 0;
    bool Initialize(const char*, int, int, bool, std::string&) override { return true; }
    TextureId LoadTexture(const std::string&, std::string&) override { ++textureLoads; return 7; }
    void BeginFrame(Color c) override { calls.push_back("begin:" + std::to_string(c.r)); }
    void SetWorldZoom(float z) override { calls.push_back("zoom:" + std::to_string(z)); }
    void DrawSprite(const Sprite& s) override { calls.push_back("sprite:" + std::to_string(s.texture)); spriteXs.push_back(s.destination.x); }
    void DrawSpriteBatch(const Sprite* s, std::size_t n) override {
        calls.push_back("batch:" + std::to_string(n));
        for (std::size_t i=0; i<n; ++i) calls.push_back("item:" + std::to_string(s[i].texture));
    }
    void FillRect(Rect, Color c) override { calls.push_back("fill:" + std::to_string(c.r)); }
    void FillRects(const Rect*, std::size_t n, Color) override { calls.push_back("fills:" + std::to_string(n)); }
    void ApplyScenePostprocess() override { calls.push_back("post"); }
    void ResolveUnprocessedFrame() override { calls.push_back("resolve"); }
    void EndFrame() override { calls.push_back("present"); }
    bool PollQuit() override { ++eventPolls; return false; }
    bool PresentationSurfaceStable() const override { return surfaceStable; }
    bool KeyDown(int) const override { ++inputReads; return false; }
    MouseState Mouse() const override { ++inputReads; return {}; }
    bool SaveFrame(const std::string&, std::string&) override { calls.push_back("capture"); return true; }
};
int main() {
    auto spy = std::make_unique<Spy>();
    Spy* raw = spy.get();
    PresentationRecordingRenderer recorder(std::move(spy));
    int simulationUpdates = 1;
    const auto origin = RefreshPresentationService::Clock::now();
    RefreshPresentationService refresh(origin, 120, recorder);
    assert(refresh.TryPresent(origin) == RefreshPresentationService::Result::NoSnapshot);
    recorder.BeginFrame({3,0,0,255});
    recorder.SetWorldZoom(1.25f);
    Sprite single{}; single.texture = 7; recorder.DrawSprite(single);
    Sprite batch[2]{}; batch[0].texture = 8; batch[1].texture = 9;
    recorder.DrawSpriteBatch(batch, 2);
    recorder.ApplyScenePostprocess();
    recorder.FillRect({1,2,3,4},{17,0,0,255});
    Rect rects[2]{{},{}};
    recorder.FillRects(rects, 2, {1,2,3,4});
    recorder.ResolveUnprocessedFrame();
    std::string error;
    recorder.SaveFrame("first", error);
    recorder.EndFrame();
    assert(recorder.Packet().Sealed());
    const auto first = raw->calls;
    assert(first.back() == "present");
    assert(recorder.Packet().Size() == 8);
    batch[0].texture = 99; // Packet owns its command inputs.
    raw->calls.clear();
    assert(refresh.TryPresent(origin) == RefreshPresentationService::Result::Presented);
    auto replayAndPresent = std::vector<std::string>(first.begin(), first.end()-2);
    replayAndPresent.push_back("present");
    assert(raw->calls == replayAndPresent);
    assert(simulationUpdates == 1);
    assert(refresh.RepeatPresents() == 1);
    // A display slot cannot turn an unstable surface into an extra present.
    raw->surfaceStable = false;
    raw->calls.clear();
    assert(refresh.TryPresent(origin + std::chrono::seconds(1)) == RefreshPresentationService::Result::SurfaceChanged);
    assert(raw->calls.empty() && refresh.RepeatPresents() == 1);
    raw->surfaceStable = true;
    for (int repeat=0; repeat<100000; ++repeat) {
        raw->calls.clear();
        recorder.Replay();
        assert(raw->calls == std::vector<std::string>(first.begin(), first.end()-2));
        assert(simulationUpdates == 1);
    }
    // Positive next-update state change replaces the packet; old image is not retained.
    ++simulationUpdates;
    recorder.BeginFrame({4,0,0,255});
    single.texture = 10; recorder.DrawSprite(single);
    recorder.EndFrame();
    raw->calls.clear(); recorder.Replay();
    assert(raw->calls == std::vector<std::string>({"begin:4", "sprite:10"}));
    raw->calls.clear();
    assert(refresh.TryPresent(origin + std::chrono::seconds(1)) == RefreshPresentationService::Result::Presented);
    assert(raw->calls == std::vector<std::string>({"begin:4", "sprite:10", "present"}));
    assert(refresh.RepeatPresents() == 2 && simulationUpdates == 2);
    // Production fresh path: seal N without an accidental alpha-1 swap, present
    // N-1 at the boundary, then a distinct read-only midpoint display slot.
    FreshPresentationPacket fresh;
    const auto makeFrame = [](float x) {
        FullFrameSnapshot f; f.sceneEpoch = 7;
        f.packet.Add(PresentationCommandPacket::Command{PresentationCommandPacket::Kind::Begin});
        PresentationCommandPacket::Command sprite{PresentationCommandPacket::Kind::Sprite};
        sprite.key={10,0}; sprite.sprite.texture=7;
        sprite.sprite.destination={x,20,20,30}; f.packet.Add(sprite);
        f.packet.Add(PresentationCommandPacket::Command{PresentationCommandPacket::Kind::Postprocess});
        PresentationCommandPacket::Command hud{PresentationCommandPacket::Kind::Sprite};
        hud.key={20,0}; hud.sprite.texture=9;
        hud.sprite.destination={50,50,100,20}; f.packet.Add(hud);
        f.packet.Seal(); return f;
    };
    fresh.Seal(makeFrame(0),{0,20,0,0,7});
    fresh.Seal(makeFrame(10),{10,20,0,0,7});
    const auto freshOrigin = RefreshPresentationService::Clock::now();
    RefreshPresentationService freshService(freshOrigin, 120, recorder);
    raw->calls.clear();
    recorder.BeginFrame({1,0,0,255}); recorder.DrawSprite(single);
    recorder.SealWithoutPresent();
    assert(raw->calls.back() != "present");
    raw->calls.clear();
    assert(freshService.PresentFreshBoundary(fresh,freshOrigin)==RefreshPresentationService::Result::Presented);
    assert(raw->calls == std::vector<std::string>({"begin:0","sprite:7","post","sprite:9","present"}));
    assert(raw->spriteXs[raw->spriteXs.size()-2]==0.f);
    raw->calls.clear();
    assert(freshService.TryPresent(freshOrigin+std::chrono::milliseconds(9))==RefreshPresentationService::Result::Presented);
    assert(freshService.FreshPresents()==1);
    assert(raw->calls == std::vector<std::string>({"begin:0","sprite:7","post","sprite:9","present"}));
    assert(raw->spriteXs[raw->spriteXs.size()-2]>0.f && raw->spriteXs[raw->spriteXs.size()-2]<10.f);
    const float lastPresentedX=raw->spriteXs[raw->spriteXs.size()-2];
    fresh.Seal(makeFrame(20),{20,20,0,0,7});
    assert(freshService.PresentFreshBoundary(fresh,freshOrigin+std::chrono::milliseconds(17))==RefreshPresentationService::Result::Presented);
    assert(raw->spriteXs[raw->spriteXs.size()-2]==10.f && raw->spriteXs[raw->spriteXs.size()-2]>=lastPresentedX);
    raw->surfaceStable=false; raw->calls.clear();
    assert(freshService.TryPresent(freshOrigin+std::chrono::milliseconds(26))==RefreshPresentationService::Result::SurfaceChanged);
    assert(raw->calls.empty()); raw->surfaceStable=true;
    assert(raw->textureLoads == 0 && raw->inputReads == 0 && raw->eventPolls == 0);
    assert(!raw->PollQuit() && raw->eventPolls == 1); // positive fixed-update event pump
    std::cout << "ordered packet 100000 repeat replays and positive next update passed\n";
}

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
    bool Initialize(const char*, int, int, bool, std::string&) override { return true; }
    TextureId LoadTexture(const std::string&, std::string&) override { return 7; }
    void BeginFrame(Color c) override { calls.push_back("begin:" + std::to_string(c.r)); }
    void SetWorldZoom(float z) override { calls.push_back("zoom:" + std::to_string(z)); }
    void DrawSprite(const Sprite& s) override { calls.push_back("sprite:" + std::to_string(s.texture)); }
    void DrawSpriteBatch(const Sprite* s, std::size_t n) override {
        calls.push_back("batch:" + std::to_string(n));
        for (std::size_t i=0; i<n; ++i) calls.push_back("item:" + std::to_string(s[i].texture));
    }
    void FillRect(Rect, Color c) override { calls.push_back("fill:" + std::to_string(c.r)); }
    void FillRects(const Rect*, std::size_t n, Color) override { calls.push_back("fills:" + std::to_string(n)); }
    void ApplyScenePostprocess() override { calls.push_back("post"); }
    void ResolveUnprocessedFrame() override { calls.push_back("resolve"); }
    void EndFrame() override { calls.push_back("present"); }
    bool PollQuit() override { return false; }
    bool PresentationSurfaceStable() const override { return true; }
    bool KeyDown(int) const override { return false; }
    MouseState Mouse() const override { return {}; }
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
    std::cout << "ordered packet 100000 repeat replays and positive next update passed\n";
}

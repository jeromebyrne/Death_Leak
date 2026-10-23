// Test-only CPU backend: no SDL, GL, device, or framebuffer.
#include "../render/Renderer.h"
#include "../audio/SoundPlayer.h"
#include <memory>
#include <unordered_map>

namespace deathleak::render {
class CpuRenderer final : public Renderer {
    std::unordered_map<std::string, TextureId> ids;
public:
    bool Initialize(const char*, int, int, std::string&) override { return true; }
    TextureId LoadTexture(const std::string& path, std::string&) override {
        return ids.try_emplace(path, static_cast<TextureId>(ids.size()+1)).first->second;
    }
    void BeginFrame(Color) override {}
    void DrawSprite(const Sprite&) override {}
    void FillRect(Rect, Color) override {}
    void ApplyScenePostprocess() override {}
    void EndFrame() override {}
    bool PollQuit() override { return false; }
    bool KeyDown(int) const override { return false; }
    MouseState Mouse() const override { return {}; }
    bool SaveFrame(const std::string&, std::string&) override { return false; }
};
std::unique_ptr<Renderer> CreateRenderer(bool) { return std::make_unique<CpuRenderer>(); }
}

namespace deathleak::audio {
class CpuSoundPlayer final : public SoundPlayer {
public:
    bool Initialize(std::string&) override { return true; }
    void Play(Cue, int) override {}
    unsigned PlayDevilVoice(bool) override { return 0; }
    bool VoiceFinished(unsigned) override { return true; }
    void SetSfxEnabled(bool) override {}
    void SetAmbience(const std::vector<AmbientSource>&) override {}
    void UpdateAmbience(float, float) override {}
};
std::unique_ptr<SoundPlayer> CreateSoundPlayer() { return std::make_unique<CpuSoundPlayer>(); }
}

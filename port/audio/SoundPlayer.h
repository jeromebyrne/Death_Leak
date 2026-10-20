#pragma once
#include <memory>
#include <string>
#include <vector>

namespace deathleak::audio {
struct AmbientSource {
    std::string file;
    float x = 0, y = 0, width = 0, height = 0, fadeWidth = 0, fadeHeight = 0;
    bool adjustToCamera = false, repeat = false;
};
enum class Cue { KnifeThrow, OrbCollect, FinalTitle, StoneSmash, PaperPickup };

// Optional game audio boundary; SDL and device handles stay in the backend.
class SoundPlayer {
public:
    virtual ~SoundPlayer() = default;
    virtual bool Initialize(std::string& error) = 0;
    virtual void Play(Cue cue, int variation = 0) = 0;
    // Zero means audio is unavailable or muted; the caller may complete dialog immediately.
    virtual unsigned PlayDevilVoice(bool third) = 0;
    virtual bool VoiceFinished(unsigned id) = 0;
    virtual void SetSfxEnabled(bool enabled) = 0;
    virtual void SetAmbience(const std::vector<AmbientSource>& sources) = 0;
    virtual void UpdateAmbience(float cameraX, float cameraY) = 0;
};
std::unique_ptr<SoundPlayer> CreateSoundPlayer();
}

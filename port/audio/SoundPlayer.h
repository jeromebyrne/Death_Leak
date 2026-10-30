#pragma once
#include <memory>
#include <cstdint>
#include <string>
#include <vector>

namespace deathleak::audio {
struct AmbientSource {
    std::string file;
    float x = 0, y = 0, width = 0, height = 0, fadeWidth = 0, fadeHeight = 0;
    bool adjustToCamera = false, repeat = false;
    float startDelay=0.f;
    bool followPlayer=false;
};
enum class Cue { KnifeThrow, OrbCollect, FinalTitle, StoneSmash, PaperPickup, HealthUpgrade, JumpVocal };

// Optional game audio boundary; SDL and device handles stay in the backend.
class SoundPlayer {
public:
    virtual ~SoundPlayer() = default;
    virtual bool Initialize(std::string& error) = 0;
    virtual void Play(Cue cue, int variation = 0) = 0;
    // Authored one-shot WAV under Game/Media/Audio; optional for non-SDL backends.
    virtual void PlayFile(const std::string&) {}
    // Independently owned looping positional SFX, not authored level ambience.
    // Zero is unavailable/muted/invalid; handles survive mute and must be stopped
    // on owner destruction or cleared on scene/session reset. Main thread only.
    using PositionalVoice = std::uint64_t;
    virtual PositionalVoice StartPositionalLoop(const AmbientSource&) { return 0; }
    virtual void UpdatePositionalLoop(PositionalVoice, float, float, float, float) {}
    virtual void StopPositionalLoop(PositionalVoice) {}
    virtual void ClearPositionalLoops() {}
    // Zero means audio is unavailable or muted; the caller may complete dialog immediately.
    virtual unsigned PlayDevilVoice(bool third) = 0;
    virtual bool VoiceFinished(unsigned id) = 0;
    virtual void SetSfxEnabled(bool enabled) = 0;
    virtual void SetAmbience(const std::vector<AmbientSource>& sources) = 0;
    virtual void UpdateAmbience(float cameraX, float cameraY) = 0;
    virtual void UpdateObjectAmbience(float cameraX,float cameraY,float,float,float) { UpdateAmbience(cameraX,cameraY); }
};
// The original Character::PlayJumpSFX draws once per successful player action.
// Values 5..19 intentionally produce no cue; draw is explicit for deterministic tests.
inline void DispatchJumpRollVocal(SoundPlayer& sound, int draw) {
    if (draw >= 0 && draw < 5) sound.Play(Cue::JumpVocal, draw);
}
std::unique_ptr<SoundPlayer> CreateSoundPlayer();
}

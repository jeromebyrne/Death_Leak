#pragma once
#include <memory>
#include <string>

namespace deathleak::audio {
enum class Cue { KnifeThrow, OrbCollect };

// Optional game audio boundary; SDL and device handles stay in the backend.
class SoundPlayer {
public:
    virtual ~SoundPlayer() = default;
    virtual bool Initialize(std::string& error) = 0;
    virtual void Play(Cue cue, int variation = 0) = 0;
};
std::unique_ptr<SoundPlayer> CreateSoundPlayer();
}

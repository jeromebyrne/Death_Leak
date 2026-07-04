#ifndef DEATHLEAK_ENGINE_AUDIO_IAUDIOSYSTEM_H
#define DEATHLEAK_ENGINE_AUDIO_IAUDIOSYSTEM_H

#include <string>

using AudioHandle = unsigned int;

class IAudioSystem
{
public:
    virtual ~IAudioSystem() = default;

    virtual bool Initialise() = 0;
    virtual void Update() = 0;
    virtual AudioHandle PlaySound(const std::string& assetPath, bool loop) = 0;
    virtual void Stop(AudioHandle handle) = 0;
    virtual void SetMasterVolume(float volume) = 0;
};

#endif

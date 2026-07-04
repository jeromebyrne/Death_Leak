#ifndef DEATHLEAK_BACKENDS_MAC_MACNULLAUDIOSYSTEM_H
#define DEATHLEAK_BACKENDS_MAC_MACNULLAUDIOSYSTEM_H

#include "Engine/Audio/IAudioSystem.h"

#include <string>
#include <vector>

class MacNullAudioSystem final : public IAudioSystem
{
public:
    bool Initialise() override;
    void Update() override;
    AudioHandle PlaySound(const std::string& assetPath, bool loop) override;
    void Stop(AudioHandle handle) override;
    void SetMasterVolume(float volume) override;

    float MasterVolume() const { return mMasterVolume; }
    unsigned int PlayCallCount() const { return mPlayCallCount; }

private:
    float mMasterVolume = 1.0f;
    unsigned int mNextHandle = 1;
    unsigned int mPlayCallCount = 0;
    std::vector<AudioHandle> mActiveHandles;
};

#endif

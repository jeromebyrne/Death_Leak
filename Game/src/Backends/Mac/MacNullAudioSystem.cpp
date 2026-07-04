#include "Backends/Mac/MacNullAudioSystem.h"

#include <algorithm>
#include <iostream>

bool MacNullAudioSystem::Initialise()
{
    return true;
}

void MacNullAudioSystem::Update()
{
}

AudioHandle MacNullAudioSystem::PlaySound(const std::string& assetPath, bool loop)
{
    const AudioHandle handle = mNextHandle++;
    ++mPlayCallCount;
    mActiveHandles.push_back(handle);

    std::cout << "audio stub play: " << assetPath
              << " loop=" << (loop ? "true" : "false")
              << " handle=" << handle << "\n";
    return handle;
}

void MacNullAudioSystem::Stop(AudioHandle handle)
{
    mActiveHandles.erase(
        std::remove(mActiveHandles.begin(), mActiveHandles.end(), handle),
        mActiveHandles.end());
}

void MacNullAudioSystem::SetMasterVolume(float volume)
{
    mMasterVolume = std::clamp(volume, 0.0f, 1.0f);
}

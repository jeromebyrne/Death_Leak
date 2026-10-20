#include "../audio/SoundPlayer.h"
#include <SDL.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <vector>

namespace deathleak::audio {
namespace {
class SDLSoundPlayer final : public SoundPlayer {
public:
    ~SDLSoundPlayer() override {
        if (device_) SDL_CloseAudioDevice(device_);
        if (audioInitialized_) SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
    bool Initialize(std::string& error) override {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) { error = SDL_GetError(); return false; }
        audioInitialized_ = true;
        SDL_AudioSpec desired{};
        desired.freq = 48000; desired.format = AUDIO_S16SYS; desired.channels = 2;
        desired.samples = 1024; desired.callback = Callback; desired.userdata = this;
        device_ = SDL_OpenAudioDevice(nullptr, 0, &desired, &spec_, 0);
        if (!device_) { error = SDL_GetError(); return false; }
        constexpr std::array<const char*, 6> files = {
            "Game/Media/Audio/knifeThrow.wav", "Game/Media/Audio/knifeThrow2.wav",
            "Game/Media/Audio/knifeThrow3.wav", "Game/Media/Audio/orb/collect_1.wav",
            "Game/Media/Audio/boomy_intro.wav", "Game/Media/Audio/rock_smash.wav"};
        for (size_t i = 0; i < files.size(); ++i) {
            SDL_AudioSpec source{}; Uint8* data = nullptr; Uint32 length = 0;
            if (!SDL_LoadWAV(files[i], &source, &data, &length)) { error = SDL_GetError(); return false; }
            SDL_AudioCVT cvt{};
            if (SDL_BuildAudioCVT(&cvt, source.format, source.channels, source.freq,
                                  spec_.format, spec_.channels, spec_.freq) < 0) {
                SDL_FreeWAV(data); error = SDL_GetError(); return false;
            }
            cvt.len = static_cast<int>(length);
            cvt.buf = static_cast<Uint8*>(SDL_malloc(length * cvt.len_mult));
            if (!cvt.buf) { SDL_FreeWAV(data); error = "Audio conversion allocation failed"; return false; }
            std::memcpy(cvt.buf, data, length); SDL_FreeWAV(data);
            if (SDL_ConvertAudio(&cvt) != 0) { SDL_free(cvt.buf); error = SDL_GetError(); return false; }
            samples_[i].assign(cvt.buf, cvt.buf + cvt.len_cvt);
            SDL_free(cvt.buf);
        }
        SDL_PauseAudioDevice(device_, 0);
        return true;
    }
    void Play(Cue cue, int variation) override {
        if (!device_ || !sfxEnabled_) return;
        const size_t index = cue == Cue::OrbCollect ? 3 : cue == Cue::FinalTitle ? 4 : cue == Cue::StoneSmash ? 5 : static_cast<size_t>((variation % 3 + 3) % 3);
        SDL_LockAudioDevice(device_);
        if (voices_.size() < 16) voices_.push_back({index, 0});
        SDL_UnlockAudioDevice(device_);
    }
    void SetSfxEnabled(bool enabled) override {
        if (!device_) { sfxEnabled_ = enabled; return; }
        SDL_LockAudioDevice(device_);
        sfxEnabled_ = enabled;
        if (!enabled) voices_.clear();
        SDL_UnlockAudioDevice(device_);
    }
private:
    bool sfxEnabled_ = true;
    struct Voice { size_t sample, offset; };
    static void Callback(void* userdata, Uint8* stream, int length) {
        auto& self = *static_cast<SDLSoundPlayer*>(userdata);
        std::memset(stream, 0, length);
        for (auto& voice : self.voices_) {
            const auto& sample = self.samples_[voice.sample];
            const size_t count = std::min(static_cast<size_t>(length), sample.size() - voice.offset);
            SDL_MixAudioFormat(stream, sample.data() + voice.offset, self.spec_.format,
                               static_cast<Uint32>(count), SDL_MIX_MAXVOLUME / 2);
            voice.offset += count;
        }
        self.voices_.erase(std::remove_if(self.voices_.begin(), self.voices_.end(),
            [&](const Voice& voice) { return voice.offset == self.samples_[voice.sample].size(); }), self.voices_.end());
    }
    bool audioInitialized_ = false;
    SDL_AudioDeviceID device_ = 0;
    SDL_AudioSpec spec_{};
    std::array<std::vector<Uint8>, 6> samples_;
    std::vector<Voice> voices_;
};
}
std::unique_ptr<SoundPlayer> CreateSoundPlayer() { return std::make_unique<SDLSoundPlayer>(); }
}

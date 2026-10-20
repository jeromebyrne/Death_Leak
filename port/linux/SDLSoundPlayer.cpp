#include "../audio/SoundPlayer.h"
#include <SDL.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <cmath>
#include <memory>
#include <iostream>
#include <unordered_map>
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
        constexpr std::array<const char*, 7> files = {
            "Game/Media/Audio/knifeThrow.wav", "Game/Media/Audio/knifeThrow2.wav",
            "Game/Media/Audio/knifeThrow3.wav", "Game/Media/Audio/orb/collect_1.wav",
            "Game/Media/Audio/boomy_intro.wav", "Game/Media/Audio/rock_smash.wav", "Game/Media/Audio/paper_pickup.wav"};
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
        const size_t index = cue == Cue::OrbCollect ? 3 : cue == Cue::FinalTitle ? 4 : cue == Cue::StoneSmash ? 5 : cue == Cue::PaperPickup ? 6 : static_cast<size_t>((variation % 3 + 3) % 3);
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
    void SetAmbience(const std::vector<AmbientSource>& sources) override {
        // Prepare outside the callback lock. MP3 and absent files remain unported.
        std::unordered_map<std::string, std::shared_ptr<std::vector<Uint8>>> decoded;
        std::vector<AmbientVoice> next;
        for (const auto& source : sources) {
            if (source.file.size() < 4 || source.file.substr(source.file.size() - 4) != ".wav") continue;
            auto& sample = decoded[source.file];
            if (!sample) {
                SDL_AudioSpec input{}; Uint8* data = nullptr; Uint32 length = 0;
                if (!SDL_LoadWAV(source.file.c_str(), &input, &data, &length)) continue;
                SDL_AudioCVT cvt{};
                if (SDL_BuildAudioCVT(&cvt, input.format, input.channels, input.freq,
                                      spec_.format, spec_.channels, spec_.freq) < 0) { SDL_FreeWAV(data); continue; }
                cvt.len = static_cast<int>(length);
                cvt.buf = static_cast<Uint8*>(SDL_malloc(length * cvt.len_mult));
                if (!cvt.buf) { SDL_FreeWAV(data); continue; }
                std::memcpy(cvt.buf, data, length); SDL_FreeWAV(data);
                if (SDL_ConvertAudio(&cvt) == 0 && cvt.len_cvt > 0)
                    sample = std::make_shared<std::vector<Uint8>>(static_cast<Uint8*>(cvt.buf), static_cast<Uint8*>(cvt.buf) + cvt.len_cvt);
                SDL_free(cvt.buf);
            }
            if (sample && !sample->empty()) next.push_back({source, sample, 0, source.adjustToCamera ? 0.f : 1.f});
        }
        if (!device_) return;
        SDL_LockAudioDevice(device_);
        ambience_ = std::move(next); // Old level ends atomically at the audio boundary.
        SDL_UnlockAudioDevice(device_);
        std::cout << "Audio ambient WAV voices=" << ambience_.size() << " authored=" << sources.size() << '\n';
    }
    void UpdateAmbience(float cameraX, float cameraY) override {
        if (!device_) return;
        SDL_LockAudioDevice(device_);
        for (auto& voice : ambience_) {
            const auto& a = voice.source;
            if (!a.adjustToCamera) { voice.gain = 1.f; continue; }
            const float dx = std::max(0.f, std::abs(cameraX - a.x) - a.width * 0.5f);
            const float dy = std::max(0.f, std::abs(cameraY - a.y) - a.height * 0.5f);
            const float fx = (a.fadeWidth - a.width) * 0.5f;
            const float fy = (a.fadeHeight - a.height) * 0.5f;
            const float gx = dx == 0 ? 1.f : fx > 0 ? 1.f - dx / fx : 0.f;
            const float gy = dy == 0 ? 1.f : fy > 0 ? 1.f - dy / fy : 0.f;
            voice.gain = std::clamp(std::min(gx, gy), 0.f, 1.f);
        }
        SDL_UnlockAudioDevice(device_);
    }
private:
    struct AmbientVoice {
        AmbientSource source;
        std::shared_ptr<std::vector<Uint8>> sample;
        size_t offset;
        float gain;
    };
    std::vector<AmbientVoice> ambience_;
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
        if (self.sfxEnabled_) for (auto& voice : self.ambience_) {
            const auto& sample = *voice.sample;
            if (!voice.gain || voice.offset >= sample.size()) continue;
            size_t output = 0;
            while (output < static_cast<size_t>(length) && voice.offset < sample.size()) {
                const size_t count = std::min(static_cast<size_t>(length) - output, sample.size() - voice.offset);
                SDL_MixAudioFormat(stream + output, sample.data() + voice.offset, self.spec_.format,
                    static_cast<Uint32>(count), static_cast<int>(std::lround(voice.gain * SDL_MIX_MAXVOLUME / 2)));
                output += count; voice.offset += count;
                if (voice.offset == sample.size() && voice.source.repeat) voice.offset = 0;
            }
        }
        self.voices_.erase(std::remove_if(self.voices_.begin(), self.voices_.end(),
            [&](const Voice& voice) { return voice.offset == self.samples_[voice.sample].size(); }), self.voices_.end());
    }
    bool audioInitialized_ = false;
    SDL_AudioDeviceID device_ = 0;
    SDL_AudioSpec spec_{};
    std::array<std::vector<Uint8>, 7> samples_;
    std::vector<Voice> voices_;
};
}
std::unique_ptr<SoundPlayer> CreateSoundPlayer() { return std::make_unique<SDLSoundPlayer>(); }
}

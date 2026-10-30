#include "../audio/SoundPlayer.h"
#include <SDL.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <cmath>
#include <memory>
#include <iostream>
#include <vector>
#include <filesystem>
#include <limits>
#include <unordered_map>

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
        constexpr std::array<const char*, 15> files = {
            "Game/Media/Audio/knifeThrow.wav", "Game/Media/Audio/knifeThrow2.wav",
            "Game/Media/Audio/knifeThrow3.wav", "Game/Media/Audio/orb/collect_1.wav",
            "Game/Media/Audio/boomy_intro.wav", "Game/Media/Audio/rock_smash.wav", "Game/Media/Audio/paper_pickup.wav",
            "Game/Media/Audio/character/health_devil/vo/devil_1_fx.wav",
            "Game/Media/Audio/character/health_devil/vo/devil_3_fx.wav",
            "Game/Media/Audio/character/drink_health_upgrade.wav",
            "Game/Media/Audio/character/jump_1.wav", "Game/Media/Audio/character/jump_2.wav",
            "Game/Media/Audio/character/jump_3.wav", "Game/Media/Audio/character/jump_4.wav",
            "Game/Media/Audio/character/jump_5.wav"};
        for (size_t i = 0; i < files.size(); ++i) {
            if (!LoadSample(files[i], samples_[i], error)) return false;
        }
        SDL_PauseAudioDevice(device_, 0);
        return true;
    }
    void Play(Cue cue, int variation) override {
        if (!device_ || !sfxEnabled_) return;
        const size_t index = cue == Cue::OrbCollect ? 3 : cue == Cue::FinalTitle ? 4 : cue == Cue::StoneSmash ? 5 : cue == Cue::PaperPickup ? 6 : cue == Cue::HealthUpgrade ? 9 : cue == Cue::JumpVocal ? 10 + static_cast<size_t>((variation % 5 + 5) % 5) : static_cast<size_t>((variation % 3 + 3) % 3);
        SDL_LockAudioDevice(device_);
        if (voices_.size() < 16) voices_.push_back({index, 0, 0});
        SDL_UnlockAudioDevice(device_);
    }
    void PlayFile(const std::string& file) override {
        if (!device_ || !sfxEnabled_) return;
        // Only existing WAVs within the authored audio tree are accepted. Resolve
        // symlinks too: a lexical prefix alone would allow an escaped asset link.
        const std::filesystem::path path(file);
        if (path.is_absolute() || file.find('\\') != std::string::npos ||
            file.find('\0') != std::string::npos || path.extension() != ".wav") return;
        for (const auto& component : path) if (component == "..") return;
        const auto normalized = path.lexically_normal();
        const std::string key = normalized.generic_string();
        if (key.compare(0, 17, "Game/Media/Audio/") != 0) return;
        std::error_code ec;
        const auto root = std::filesystem::canonical("Game/Media/Audio", ec);
        if (ec) return;
        const auto resolved = std::filesystem::canonical(normalized, ec);
        if (ec) return;
        const auto relative = resolved.lexically_relative(root);
        if (relative.empty() || *relative.begin() == "..") return;
        auto cached = fileSamples_.find(key);
        size_t index;
        if (cached == fileSamples_.end()) {
            std::vector<Uint8> sample;
            std::string error;
            if (!LoadSample(key.c_str(), sample, error)) return;
            // Decoding never runs in the callback or while holding its lock.
            SDL_LockAudioDevice(device_);
            index = samples_.size();
            samples_.push_back(std::move(sample));
            SDL_UnlockAudioDevice(device_);
            fileSamples_.emplace(key, index);
        } else index = cached->second;
        SDL_LockAudioDevice(device_);
        if (voices_.size() < 16) voices_.push_back({index, 0, 0});
        SDL_UnlockAudioDevice(device_);
    }
    unsigned PlayDevilVoice(bool third) override {
        if (!device_ || !sfxEnabled_) return 0;
        SDL_LockAudioDevice(device_);
        // Story dialog controls reward timing: never reject it because ordinary
        // one-shots filled the shared pool. Evict the oldest ordinary cue first;
        // if every slot is already dialog, allow another tracked dialog voice.
        if (voices_.size() >= 16) {
            const auto ordinary = std::find_if(voices_.begin(), voices_.end(),
                [](const Voice& voice) { return voice.id == 0; });
            if (ordinary != voices_.end()) voices_.erase(ordinary);
        }
        // Handle zero is reserved for unavailable/muted audio. Skip it on wrap.
        if (++nextVoiceId_ == 0) ++nextVoiceId_;
        const unsigned id = nextVoiceId_;
        voices_.push_back({third ? 8u : 7u, 0, id});
        SDL_UnlockAudioDevice(device_);
        return id;
    }
    bool VoiceFinished(unsigned id) override {
        if (!id || !device_) return true;
        SDL_LockAudioDevice(device_);
        const bool finished = !sfxEnabled_ || std::none_of(voices_.begin(), voices_.end(),
            [id](const Voice& voice) { return voice.id == id; });
        SDL_UnlockAudioDevice(device_);
        return finished;
    }
#ifdef DEATHLEAK_AUDIO_TESTING
    void PauseForTest() { SDL_PauseAudioDevice(device_, 1); }
    size_t CachedFilesForTest() const { return fileSamples_.size(); }
    size_t ActiveVoicesForTest() const { return voices_.size(); } // Device paused.
    size_t AmbientVoicesForTest() const { return ambience_.size(); }
    bool AdvanceForTest(int bytes) { // Device paused: drive the real mixer callback deterministically.
        std::vector<Uint8> output(static_cast<size_t>(bytes));
        Callback(this, output.data(), bytes);
        return std::any_of(output.begin(), output.end(), [](Uint8 b) { return b != 0; });
    }
#endif
    void SetSfxEnabled(bool enabled) override {
        if (!device_) { sfxEnabled_ = enabled; return; }
        SDL_LockAudioDevice(device_);
        sfxEnabled_ = enabled;
        if (!enabled) voices_.clear();
        SDL_UnlockAudioDevice(device_);
    }
    void SetAmbience(const std::vector<AmbientSource>& sources) override {
        // File decoding and priming happen outside the audio callback lock.
        std::vector<std::shared_ptr<AmbientVoice>> next;
        size_t buffered = 0;
        for (const auto& source : sources) {
            if (source.file.size() < 4 || source.file.substr(source.file.size() - 4) != ".wav") continue;
            auto voice = OpenAmbient(source);
            if (!voice) continue;
            Fill(*voice);
            buffered += voice->buffer.size();
            next.push_back(std::move(voice));
        }
        if (!device_) return;
        SDL_LockAudioDevice(device_);
        ambience_ = std::move(next); // Old level ends atomically at the audio boundary.
        SDL_UnlockAudioDevice(device_);
        std::cout << "Audio ambient streamed WAV voices=" << ambience_.size()
                  << " authored=" << sources.size() << " buffered_bytes=" << buffered << '\n';
    }
    void UpdateAmbience(float cameraX,float cameraY) override { UpdateObjectAmbience(cameraX,cameraY,cameraX,cameraY,0.f); }
    void UpdateObjectAmbience(float cameraX,float cameraY,float playerX,float playerY,float dt) override {
        if (!device_) return;
        std::vector<std::shared_ptr<AmbientVoice>> snapshot;
        SDL_LockAudioDevice(device_);
        snapshot = ambience_;
        for (auto& voice : snapshot) {
            const auto& a = voice->source;
            voice->startDelay=std::max(0.f,voice->startDelay-dt);
            const float ownerX=a.followPlayer ? playerX : a.x;
            const float ownerY=a.followPlayer ? playerY : a.y;
            if (!a.adjustToCamera) { voice->gain = 1.f; continue; }
            const float dx = std::max(0.f, std::abs(cameraX - ownerX) - a.width * 0.5f);
            const float dy = std::max(0.f, std::abs(cameraY - ownerY) - a.height * 0.5f);
            const float fx = (a.fadeWidth - a.width) * 0.5f;
            const float fy = (a.fadeHeight - a.height) * 0.5f;
            const float gx = dx == 0 ? 1.f : fx > 0 ? 1.f - dx / fx : 0.f;
            const float gy = dy == 0 ? 1.f : fy > 0 ? 1.f - dy / fy : 0.f;
            voice->gain = std::clamp(std::min(gx, gy), 0.f, 1.f);
        }
        SDL_UnlockAudioDevice(device_);
        // Only the main thread touches file and converter; callback only consumes buffer.
        for (auto& voice : snapshot) {
            SDL_LockAudioDevice(device_);
            const size_t available = voice->buffer.size() - voice->readPos;
            SDL_UnlockAudioDevice(device_);
            if (available < kTargetBytes && !voice->finished) Fill(*voice);
        }
    }
private:
    bool LoadSample(const char* file, std::vector<Uint8>& sample, std::string& error) {
        SDL_AudioSpec source{}; Uint8* data = nullptr; Uint32 length = 0;
        if (!SDL_LoadWAV(file, &source, &data, &length)) { error = SDL_GetError(); return false; }
        SDL_AudioCVT cvt{};
        if (SDL_BuildAudioCVT(&cvt, source.format, source.channels, source.freq,
                              spec_.format, spec_.channels, spec_.freq) < 0) {
            SDL_FreeWAV(data); error = SDL_GetError(); return false;
        }
        if (!length || length > static_cast<Uint32>(std::numeric_limits<int>::max() / cvt.len_mult)) {
            SDL_FreeWAV(data); error = "Audio sample exceeds conversion size limit"; return false;
        }
        cvt.len = static_cast<int>(length);
        cvt.buf = static_cast<Uint8*>(SDL_malloc(static_cast<size_t>(length) * cvt.len_mult));
        if (!cvt.buf) { SDL_FreeWAV(data); error = "Audio conversion allocation failed"; return false; }
        std::memcpy(cvt.buf, data, length); SDL_FreeWAV(data);
        if (SDL_ConvertAudio(&cvt) != 0) { SDL_free(cvt.buf); error = SDL_GetError(); return false; }
        sample.assign(cvt.buf, cvt.buf + cvt.len_cvt);
        SDL_free(cvt.buf);
        return true;
    }
    static constexpr size_t kTargetBytes = 48000 * 2 * 2 / 2; // 0.5 second at output format.
    struct AmbientVoice {
        AmbientSource source;
        SDL_RWops* file = nullptr;
        SDL_AudioStream* converter = nullptr;
        Sint64 dataStart = 0, dataSize = 0, dataRead = 0;
        Uint16 channels = 0, bits = 0;
        bool finished = false;
        std::vector<Uint8> buffer;
        size_t readPos = 0;
        float gain = 0,startDelay=0;
        ~AmbientVoice() {
            if (converter) SDL_FreeAudioStream(converter);
            if (file) SDL_RWclose(file);
        }
    };
    static bool ReadU16(SDL_RWops* f, Uint16& value) {
        Uint8 b[2];
        if (SDL_RWread(f, b, 1, 2) != 2) return false;
        value = static_cast<Uint16>(b[0] | b[1] << 8); return true;
    }
    static bool ReadU32(SDL_RWops* f, Uint32& value) {
        Uint8 b[4];
        if (SDL_RWread(f, b, 1, 4) != 4) return false;
        value = Uint32(b[0]) | Uint32(b[1]) << 8 | Uint32(b[2]) << 16 | Uint32(b[3]) << 24;
        return true;
    }
    std::shared_ptr<AmbientVoice> OpenAmbient(const AmbientSource& source) {
        auto voice = std::make_shared<AmbientVoice>();
        voice->source = source;
        voice->gain = source.adjustToCamera ? 0.f : 1.f;
        voice->startDelay=std::max(0.f,source.startDelay);
        voice->file = SDL_RWFromFile(source.file.c_str(), "rb");
        if (!voice->file) return {};
        Uint32 riffSize = 0;
        char tag[4];
        if (SDL_RWread(voice->file, tag, 1, 4) != 4 || std::memcmp(tag, "RIFF", 4) ||
            !ReadU32(voice->file, riffSize) || SDL_RWread(voice->file, tag, 1, 4) != 4 ||
            std::memcmp(tag, "WAVE", 4)) return {};
        Uint16 format = 0;
        Uint32 rate = 0;
        bool foundFormat = false, foundData = false;
        const Sint64 fileSize = SDL_RWsize(voice->file);
        while (SDL_RWtell(voice->file) >= 0 && SDL_RWtell(voice->file) + 8 <= fileSize) {
            Uint32 size = 0;
            if (SDL_RWread(voice->file, tag, 1, 4) != 4 || !ReadU32(voice->file, size)) break;
            const Sint64 start = SDL_RWtell(voice->file);
            if (start < 0 || Sint64(size) > fileSize - start) return {};
            if (!std::memcmp(tag, "fmt ", 4) && size >= 16) {
                Uint32 byteRate = 0; Uint16 blockAlign = 0;
                if (!ReadU16(voice->file, format) || !ReadU16(voice->file, voice->channels) ||
                    !ReadU32(voice->file, rate) || !ReadU32(voice->file, byteRate) ||
                    !ReadU16(voice->file, blockAlign) || !ReadU16(voice->file, voice->bits)) return {};
                foundFormat = true;
            } else if (!std::memcmp(tag, "data", 4) && !foundData) {
                voice->dataStart = start;
                voice->dataSize = size;
                foundData = true;
            }
            if (SDL_RWseek(voice->file, start + size + (size & 1), RW_SEEK_SET) < 0) return {};
        }
        if (!foundFormat || !foundData || format != 1 || !rate || !voice->dataSize ||
            (voice->channels != 1 && voice->channels != 2) ||
            (voice->bits != 8 && voice->bits != 16 && voice->bits != 24 && voice->bits != 32)) return {};
        const SDL_AudioFormat inputFormat = voice->bits == 8 ? AUDIO_U8 :
            voice->bits == 16 ? AUDIO_S16LSB : AUDIO_S32LSB;
        voice->converter = SDL_NewAudioStream(inputFormat, Uint8(voice->channels), int(rate),
                                                spec_.format, spec_.channels, spec_.freq);
        if (!voice->converter || SDL_RWseek(voice->file, voice->dataStart, RW_SEEK_SET) < 0) return {};
        return voice;
    }
    bool Drain(AmbientVoice& voice) {
        const int ready = SDL_AudioStreamAvailable(voice.converter);
        if (ready < 0) return false;
        if (!ready) return true;
        std::vector<Uint8> converted(static_cast<size_t>(ready));
        const int count = SDL_AudioStreamGet(voice.converter, converted.data(), ready);
        if (count < 0) return false;
        SDL_LockAudioDevice(device_);
        if (voice.readPos) {
            voice.buffer.erase(voice.buffer.begin(), voice.buffer.begin() + voice.readPos);
            voice.readPos = 0;
        }
        voice.buffer.insert(voice.buffer.end(), converted.begin(), converted.begin() + count);
        SDL_UnlockAudioDevice(device_);
        return true;
    }
    void Fill(AmbientVoice& voice) {
        constexpr size_t kSourceFrames = 4096;
        const size_t frameBytes = voice.channels * (voice.bits / 8);
        for (int chunks = 0; chunks < 32; ++chunks) {
            SDL_LockAudioDevice(device_);
            const size_t available = voice.buffer.size() - voice.readPos;
            SDL_UnlockAudioDevice(device_);
            if (available >= kTargetBytes || voice.finished) break;
            if (voice.dataRead == voice.dataSize) {
                if (SDL_AudioStreamFlush(voice.converter) < 0 || !Drain(voice)) {
                    voice.finished = true; break;
                }
                if (voice.source.repeat) {
                    if (SDL_RWseek(voice.file, voice.dataStart, RW_SEEK_SET) < 0) {
                        voice.finished = true; break;
                    }
                    voice.dataRead = 0;
                } else { voice.finished = true; break; }
            }
            const size_t toRead = static_cast<size_t>(std::min<Sint64>(
                static_cast<Sint64>(kSourceFrames * frameBytes), voice.dataSize - voice.dataRead));
            if (!toRead) break;
            std::array<Uint8, kSourceFrames * 8> raw{};
            const size_t got = SDL_RWread(voice.file, raw.data(), 1, toRead);
            if (!got) { voice.finished = true; break; }
            voice.dataRead += got;
            if (voice.bits == 24) {
                std::array<Uint8, kSourceFrames * 8> expanded{};
                for (size_t i = 0; i < got / 3; ++i) {
                    expanded[i * 4] = 0;
                    expanded[i * 4 + 1] = raw[i * 3];
                    expanded[i * 4 + 2] = raw[i * 3 + 1];
                    expanded[i * 4 + 3] = raw[i * 3 + 2];
                }
                if (SDL_AudioStreamPut(voice.converter, expanded.data(), int(got / 3 * 4)) < 0) {
                    voice.finished = true; break;
                }
            } else if (SDL_AudioStreamPut(voice.converter, raw.data(), int(got)) < 0) {
                voice.finished = true; break;
            }
            if (!Drain(voice)) { voice.finished = true; break; }
        }
    }
    std::vector<std::shared_ptr<AmbientVoice>> ambience_;
    bool sfxEnabled_ = true;
    struct Voice { size_t sample, offset; unsigned id; };
    unsigned nextVoiceId_ = 0;
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
        if (self.sfxEnabled_) for (auto& ptr : self.ambience_) {
            auto& voice = *ptr;
            if(voice.startDelay>0.f)continue;
            const size_t available = voice.buffer.size() - voice.readPos;
            const size_t count = std::min(static_cast<size_t>(length), available);
            if (count && voice.gain > 0.f)
                SDL_MixAudioFormat(stream, voice.buffer.data() + voice.readPos, self.spec_.format,
                    static_cast<Uint32>(count), static_cast<int>(std::lround(voice.gain * SDL_MIX_MAXVOLUME / 2)));
            voice.readPos += count; // Keep time moving even when muted or outside fade bounds.
        }
        self.voices_.erase(std::remove_if(self.voices_.begin(), self.voices_.end(),
            [&](const Voice& voice) { return voice.offset == self.samples_[voice.sample].size(); }), self.voices_.end());
    }
    bool audioInitialized_ = false;
    SDL_AudioDeviceID device_ = 0;
    SDL_AudioSpec spec_{};
    std::vector<std::vector<Uint8>> samples_{15};
    std::unordered_map<std::string, size_t> fileSamples_;
    std::vector<Voice> voices_;
};
}
std::unique_ptr<SoundPlayer> CreateSoundPlayer() { return std::make_unique<SDLSoundPlayer>(); }
}

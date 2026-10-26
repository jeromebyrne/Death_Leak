#include "../render/Renderer.h"
#ifdef DEATHLEAK_TEST_BODY_SLOW_FRAME_COMMAND
#include "../tests/private_body_slow_frame_command_observer_20261023.h"
#endif
#include "../game/LevelScene.h"
#include "../game/ShaderClock.h"
#include "FixedStepPacer.h"
#include "PresentationCommandPacket.h"
#include "FullFrameInterpolationGate.h"
#include "MixedCommandInterpolationGate.h"
#include "RealSceneTopologyCapture.h"
#include "RefreshSlotScheduler.h"
#include "../game/WeatherLightning.h"
#include "../game/WeatherRain.h"
#include "../game/WeatherTextureResidency.h"
#include "../game/FilmFrameResidency.h"
#include "../game/LoadingOverlayPacket.h"
#include "../game/Combat.h"
#include "../game/PlayerMotion.h"
#include "../game/CameraShake.h"
#include "../game/Hud.h"
#include "../game/HudPresentationPacket.h"
#include "../game/BitmapText.h"
#include "../game/PaperText.h"
#include "../game/Settings.h"
#include "../audio/SoundPlayer.h"
#include <chrono>
#include <thread>
#include <cstdint>
#include <unordered_map>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <unordered_set>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <optional>
#include "FilmFrameSelector.h"
#include <limits>
#include <sys/wait.h>
#include <unistd.h>

using namespace deathleak::render;
namespace deathleak::render { void StartCaveDecodeWorker(); void StopCaveDecodeWorker(); }

namespace {

// UI.xml uses a 1920x1080 centered coordinate system. Preserve the authored
// widget rectangles while rendering in the port's 1280x720 logical viewport.
constexpr Rect AuthoredUI(float x, float y, float width, float height) {
    constexpr float scale = 2.f / 3.f;
    return {(960.f + x) * scale, (540.f - y - height) * scale,
            width * scale, height * scale};
}
bool Hit(Rect rect, MouseState mouse) {
    return mouse.x >= rect.x && mouse.x < rect.x + rect.width &&
           mouse.y >= rect.y && mouse.y < rect.y + rect.height;
}
constexpr std::array<Rect, 3> menuBounds = {{AuthoredUI(-435,-100,160,160),
    AuthoredUI(-410,-275,100,100), AuthoredUI(-395,-435,90,90)}};
// First intro phase has its own cumulative clock; the held pull-progress clock
// below is intentionally independent and resets on release.
struct IntroPromptClock {
    float phaseTotal = 0.f;
    void Advance(float dt) { phaseTotal += dt; }
    void Reset() { phaseTotal = 0.f; }
    bool Visible(bool looped, bool rawInteractHeld) const {
        return looped && phaseTotal > 8.f && !rawInteractHeld;
    }
};
// Fixed-update snapshot of the intro X prompt admission. Draw may read it
// repeatedly without sampling live input or mutable scene/clock state.
struct IntroPromptPresentation {
    bool visible = false;
    void Prepare(bool looped, bool rawInteractHeld, bool suppressed,
                 bool artResident, const IntroPromptClock& clock) {
        visible = !suppressed && artResident && clock.Visible(looped, rawInteractHeld);
    }
    bool Visible() const { return visible; }
};
constexpr std::array<Rect, 3> optionsBounds = {{AuthoredUI(-420,-60,132,132),
    AuthoredUI(-420,-200,132,132), AuthoredUI(-390,-470,90,90)}};
std::string Sha256File(const std::string& path) {
    int pipefd[2];
    if (pipe(pipefd) != 0) return {};
    const pid_t child = fork();
    if (child == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]); close(pipefd[1]);
        execlp("sha256sum", "sha256sum", "--", path.c_str(), static_cast<char*>(nullptr));
        _exit(127);
    }
    close(pipefd[1]);
    if (child < 0) { close(pipefd[0]); return {}; }
    std::string output;
    char buffer[128];
    ssize_t count;
    while ((count = read(pipefd[0], buffer, sizeof(buffer))) > 0) output.append(buffer, count);
    close(pipefd[0]);
    int status = 0;
    if (waitpid(child, &status, 0) != child || !WIFEXITED(status) || WEXITSTATUS(status) != 0 ||
        output.size() < 65 || output[64] != ' ') return {};
    const auto hash = output.substr(0, 64);
    if (!std::all_of(hash.begin(), hash.end(), [](char c) {
            return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
        })) return {};
    return hash;
}
std::string JsonString(const std::string& value) {
    std::ostringstream out;
    out << '"';
    for (unsigned char c : value) {
        if (c == '"' || c == '\\') out << '\\' << c;
        else if (c < 0x20) out << "\\u00" << "0123456789abcdef"[c >> 4]
                              << "0123456789abcdef"[c & 15];
        else out << c;
    }
    out << '"';
    return out.str();
}
// Diagnostic decorator records renderer-level scene submissions in call order.
// It is installed only for the explicit post-fade packet; normal play uses the
// original renderer directly. Paths are captured at LoadTexture, including
// animation frames and runtime effects loaded by LevelScene.
class PacketTraceRenderer final : public Renderer {
public:
    explicit PacketTraceRenderer(std::unique_ptr<Renderer> inner, std::string id22Prefix = {})
        : inner_(std::move(inner)), id22Prefix_(std::move(id22Prefix)) {}
    void SetRequestedRefreshHz(int hz) override { inner_->SetRequestedRefreshHz(hz); }
    bool Initialize(const char* title, int width, int height, bool fullscreen, std::string& error) override {
        return inner_->Initialize(title, width, height, fullscreen, error);
    }
    TextureId LoadTexture(const std::string& path, std::string& error) override {
        auto id = inner_->LoadTexture(path, error);
        if (id != InvalidTexture) paths_[id] = path;
        return id;
    }
    void BeginFrame(Color color) override { inner_->BeginFrame(color); }
    void SetWorldZoom(float zoom) override { inner_->SetWorldZoom(zoom); }
    void DrawSprite(const Sprite& sprite) override {
        if (sceneTrace_) {
            const auto it = paths_.find(sprite.texture);
            draws_.push_back({sprite.texture, it == paths_.end() ? std::string{} : it->second,
                              sprite.source, sprite.destination, sprite.tint, sprite.opacity,
                              sprite.rotationDegrees, sprite.flipX, sprite.flipY,
                              sprite.repeatX, sprite.uvShiftX, sprite.uvShiftY,
                              sprite.foliageSway.enabled, sprite.pixelWobble.enabled});
        }
        if (sceneTrace_ && !id22Prefix_.empty() && draws_.back().path ==
            "Game/Media/levels/waterfall_top/ground_blur.png") {
            std::string error;
            id22Ordinal_ = static_cast<int>(draws_.size() - 1);
            preOk_ = inner_->SaveFrame(id22Prefix_ + ".pre.bmp", error);
            if (!preOk_) id22Error_ = "pre: " + error;
            inner_->DrawSprite(sprite);
            error.clear();
            postOk_ = inner_->SaveFrame(id22Prefix_ + ".post.bmp", error);
            if (!postOk_) id22Error_ += " post: " + error;
            return;
        }
        inner_->DrawSprite(sprite);
    }
    void FillRect(Rect rect, Color color) override { inner_->FillRect(rect, color); }
    void ApplyScenePostprocess() override { inner_->ApplyScenePostprocess(); }
    void ResolveUnprocessedFrame() override { inner_->ResolveUnprocessedFrame(); }
    void EndFrame() override { inner_->EndFrame(); }
    bool PollQuit() override { return inner_->PollQuit(); }
    bool KeyDown(int key) const override { return inner_->KeyDown(key); }
    ControllerSticks ControllerAxes() const override { return inner_->ControllerAxes(); }
    MouseState Mouse() const override { return inner_->Mouse(); }
    bool SaveFrame(const std::string& path, std::string& error) override {
        return inner_->SaveFrame(path, error);
    }
    void StartScene() { draws_.clear(); sceneTrace_ = true; }
    int Id22Ordinal() const { return id22Ordinal_; }
    bool Id22Captured() const { return preOk_ && postOk_ && id22Ordinal_ >= 0; }
    const std::string& Id22Error() const { return id22Error_; }
    void StopScene() { sceneTrace_ = false; }
    struct Draw {
        TextureId texture; std::string path; Rect source, destination; Color tint;
        float opacity, rotation; bool flipX, flipY, repeatX; float uvShiftX, uvShiftY;
        bool foliageSway, pixelWobble;
    };
    const std::vector<Draw>& Draws() const { return draws_; }
private:
    std::unique_ptr<Renderer> inner_;
    std::unordered_map<TextureId, std::string> paths_;
    std::vector<Draw> draws_;
    bool sceneTrace_ = false;
    std::string id22Prefix_, id22Error_;
    int id22Ordinal_ = -1;
    bool preOk_ = false, postOk_ = false;
};
}

int main(int argc, char** argv) {
    int frameLimit = 0;
    std::string capture;
    std::string mixedReplayPrefix;
    int mixedReplayFrame = -1;
    int mixedHoldAuditFrame = -1;
    std::string atomicFrame415Packet;
    std::string atomicPostfadePacket;
    std::string atomicId22Prefix;
    std::string sameRunFrame415Capture;
    bool sameRunWaterfallPackets = false;
    std::string levelPreview;
    std::string playLevel;
    std::string script;
    std::string cameraScriptFile;
    std::string mouseScriptFile;
    std::string saveFile, loadSaveFile;
    std::string settingsFile = "deathleak-settings.xml";
    float savedX = 0, savedY = 0, savedHealth = 25, savedFocus = 100;
    bool restorePosition = false;
    std::unordered_set<std::string> collectedOrbs, ownedKeys, unlockedDoors, collectedPapers, rewardedHealthDevils, brokenSmashables;
    int healthDevilRewardCount = 0;
    bool deflectionUnlocked = false, speedUnlocked = false;
    bool downwardDashUnlocked = false, rollUnlocked = false;
    bool projectileDamageUnlocked = false, projectileSpeedUnlocked = false;
    float savedMaxHealth = 25;
    bool hasSavedMaxHealth = false;
    int orbBalance = 0;
    bool balanceLoaded = false;
    int completions = 0;
    std::string language;
    // Validated by save_bridge.py; carried verbatim until Windows-only systems are ported.
    std::string windowsPermanentExtension;
    std::string selectedLanguage;
    std::unordered_map<std::string, std::int64_t> triggerHistory;
    bool menuLoop = false;
    bool simulateOnly = false;
    bool simulateWallClock = false;
    bool profile = false;
    bool showFps = false;
    bool repeatPresentationCheck = false;
    bool fullFrameTopologyCheck = false;
    bool refreshSlots = false;
    bool combatState = false;
    bool freshSaveIntro = false;
    bool steamDeck = false;
    int startEncounter = -1;
    int startHazard = -1;
    bool exhaustedFocus = false;
    bool suppressLandingSprayDraw = false;
    bool suppressIntroPromptDraw = false;
    float fixedDt = 0;
    int testRefreshHz = 0;
    int requestedRefreshHz = 0;
    int windowWidth = 1280, windowHeight = 720;
    bool fullscreen = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--frames" && i + 1 < argc) frameLimit = std::atoi(argv[++i]);
        else if (arg == "--capture" && i + 1 < argc) capture = argv[++i];
        else if (arg == "--fullscreen") fullscreen = true;
        else if (arg == "--refresh-hz" && i + 1 < argc) {
            const std::string rate = argv[++i];
            if (rate.empty() || rate.find_first_not_of("0123456789") != std::string::npos ||
                rate.size() > 3) { std::cerr << "--refresh-hz expects an integer within 24..360\n"; return 2; }
            requestedRefreshHz = std::stoi(rate);
            if (requestedRefreshHz < 24 || requestedRefreshHz > 360) {
                std::cerr << "--refresh-hz expects an integer within 24..360\n"; return 2;
            }
        }
        else if ((arg == "--window-size" || arg == "--resolution") && i + 1 < argc) {
            const std::string size = argv[++i];
            const auto separator = size.find('x');
            try {
                if (separator == std::string::npos || size.find('x', separator + 1) != std::string::npos)
                    throw std::invalid_argument("format");
                size_t usedWidth = 0, usedHeight = 0;
                const int width = std::stoi(size.substr(0, separator), &usedWidth);
                const int height = std::stoi(size.substr(separator + 1), &usedHeight);
                if (usedWidth != separator || usedHeight != size.size() - separator - 1 ||
                    width < 640 || width > 7680 || height < 360 || height > 4320)
                    throw std::invalid_argument("bounds");
                windowWidth = width; windowHeight = height;
            } catch (const std::exception&) {
                std::cerr << arg << " expects WIDTHxHEIGHT within 640x360..7680x4320\n";
                return 2;
            }
        }
        else if (arg == "--atomic-frame415-packet" && i + 1 < argc) atomicFrame415Packet = argv[++i];
        else if (arg == "--atomic-postfade-waterfall-packet" && i + 1 < argc) atomicPostfadePacket = argv[++i];
        else if (arg == "--atomic-waterfall-id22-prefix" && i + 1 < argc) atomicId22Prefix = argv[++i];
        else if (arg == "--atomic-same-run-waterfall-packets" && i + 4 < argc) {
            sameRunWaterfallPackets = true;
            sameRunFrame415Capture = argv[++i];
            atomicFrame415Packet = argv[++i];
            capture = argv[++i];
            atomicPostfadePacket = argv[++i];
        }
        else if (arg == "--level-preview" && i + 1 < argc) levelPreview = argv[++i];
        else if (arg == "--play" && i + 1 < argc) playLevel = argv[++i];
        else if (arg == "--save-file" && i + 1 < argc) saveFile = argv[++i];
        else if (arg == "--settings-file" && i + 1 < argc) settingsFile = argv[++i];
        else if (arg == "--language" && i + 1 < argc) selectedLanguage = argv[++i];
        else if (arg == "--load-save" && i + 1 < argc) loadSaveFile = argv[++i];
        else if (arg == "--script" && i + 1 < argc) script = argv[++i];
        else if (arg == "--mouse-script" && i + 1 < argc) mouseScriptFile = argv[++i];
        else if (arg == "--camera-script" && i + 1 < argc) cameraScriptFile = argv[++i];
        else if (arg == "--fixed-dt" && i + 1 < argc) fixedDt = std::atof(argv[++i]);
        else if (arg == "--test-refresh-hz" && i + 1 < argc) testRefreshHz = std::atoi(argv[++i]);
        else if (arg == "--simulate-only") simulateOnly = true;
        else if (arg == "--simulate-wall-clock") simulateWallClock = true;
        else if (arg == "--menu-loop") menuLoop = true;
        else if (arg == "--repeat-presentation-check") repeatPresentationCheck = true;
        else if (arg == "--full-frame-topology-check") fullFrameTopologyCheck = true;
        else if (arg == "--mixed-replay-prefix" && i + 1 < argc) { mixedReplayPrefix = argv[++i]; fullFrameTopologyCheck = true; }
        else if (arg == "--mixed-replay-frame" && i + 1 < argc) mixedReplayFrame = std::atoi(argv[++i]);
        else if (arg == "--mixed-hold-audit-frame" && i + 1 < argc) { mixedHoldAuditFrame = std::atoi(argv[++i]); fullFrameTopologyCheck = true; }
        else if (arg == "--refresh-slots") refreshSlots = true;
        else if (arg == "--profile") { profile = true; showFps = true; }
        else if (arg == "--debug") showFps = true;
        else if (arg == "--show-fps") showFps = true;
        else if (arg == "--combat-state") combatState = true;
        else if (arg == "--fresh-save-intro") freshSaveIntro = true;
        else if (arg == "--steam-deck") steamDeck = true;
        else if (arg == "--exhaust-focus") exhaustedFocus = true;
        else if (arg == "--capture-suppress-landing-draw") suppressLandingSprayDraw = true;
        else if (arg == "--capture-suppress-intro-prompt") suppressIntroPromptDraw = true;
        else if (arg == "--start-encounter" && i + 1 < argc) startEncounter = std::atoi(argv[++i]);
        else if (arg == "--start-hazard" && i + 1 < argc) startHazard = std::atoi(argv[++i]);
        else { std::cerr << "Usage: deathleak-linux [--frames N] [--capture output.bmp] [--resolution WIDTHxHEIGHT] [--fullscreen [--refresh-hz HZ]] [--level-preview level.xml | --play level.xml] [--debug | --show-fps] [--profile]\n"; return 2; }
    }
    if (requestedRefreshHz && !fullscreen) {
        std::cerr << "--refresh-hz requires --fullscreen\n"; return 2;
    }
    if (!atomicFrame415Packet.empty() && !sameRunWaterfallPackets && (capture.empty() || frameLimit != 416 ||
        atomicFrame415Packet == capture || fixedDt <= 0.f || script.empty() || playLevel.empty())) {
        std::cerr << "--atomic-frame415-packet requires --capture, --frames 416, --fixed-dt, --script, --play and a distinct JSON path\n";
        return 2;
    }
    if (!atomicPostfadePacket.empty() && !sameRunWaterfallPackets && (capture.empty() || frameLimit <= 0 ||
        atomicPostfadePacket == capture || !atomicFrame415Packet.empty() ||
        fixedDt <= 0.f || script.empty() || playLevel.empty())) {
        std::cerr << "--atomic-postfade-waterfall-packet requires --capture, --frames, --fixed-dt, --script, --play and a distinct JSON path\n";
        return 2;
    }
    if (sameRunWaterfallPackets && (frameLimit != 425 || fixedDt <= 0.f || script.empty() || playLevel.empty() ||
        sameRunFrame415Capture.empty() || atomicFrame415Packet.empty() || capture.empty() || atomicPostfadePacket.empty() ||
        sameRunFrame415Capture == atomicFrame415Packet || sameRunFrame415Capture == capture ||
        sameRunFrame415Capture == atomicPostfadePacket || atomicFrame415Packet == capture ||
        atomicFrame415Packet == atomicPostfadePacket || capture == atomicPostfadePacket)) {
        std::cerr << "--atomic-same-run-waterfall-packets requires --frames 425, --fixed-dt, --script, --play and four distinct nonempty paths: frame415.bmp frame415.json frame424.bmp frame424.json\n";
        return 2;
    }
    if (!atomicId22Prefix.empty() && (frameLimit != 639 || fixedDt <= 0.f ||
        script.empty() || playLevel.empty() || capture.empty() ||
        !atomicPostfadePacket.empty() || !atomicFrame415Packet.empty())) {
        std::cerr << "--atomic-waterfall-id22-prefix requires --frames 639, --capture, --fixed-dt, --script, --play and no other atomic packet\n";
        return 2;
    }
    if (suppressIntroPromptDraw && capture.empty()) {
        std::cerr << "--capture-suppress-intro-prompt requires --capture\n"; return 2;
    }
    if (suppressLandingSprayDraw && capture.empty()) {
        std::cerr << "--capture-suppress-landing-draw requires --capture\n"; return 2;
    }
    if (!selectedLanguage.empty() && selectedLanguage != "en-gb" && selectedLanguage != "pt-br" && selectedLanguage != "de") {
        std::cerr << "Unsupported language locale\n"; return 2;
    }
    deathleak::game::Settings settings;
    if (!settings.Load(settingsFile)) { std::cerr << "Invalid settings file: " << settingsFile << '\n'; return 2; }
    // A completed run leaves a metadata-only save, matching the original
    // SaveManager wipe: language and number of completions survive, progress does not.
    // A new game may select the same save path without loading old progress.
    if (!saveFile.empty() && saveFile != loadSaveFile) {
        std::ifstream previous(saveFile);
        std::string line;
        if (std::getline(previous, line) && (line == "DL5" || line == "DL6")) {
            while (std::getline(previous, line)) {
                if (line.rfind("C:", 0) == 0) {
                    try {
                        size_t end = 0;
                        const auto value = std::stoll(line.substr(2), &end);
                        if (end == line.size()-2 && value >= 0 && value < 1000000000) completions = static_cast<int>(value);
                    } catch (const std::exception&) {}
                } else if (line.rfind("G:", 0) == 0) language = line.substr(2);
            }
        }
    }
    if (!playLevel.empty() && !levelPreview.empty()) { std::cerr << "Choose either --play or --level-preview\n"; return 2; }
    if (freshSaveIntro && (!loadSaveFile.empty() || playLevel != "Game/XmlFiles/levels/forest_exploration_1.xml")) {
        std::cerr << "--fresh-save-intro requires a new --play forest_exploration_1.xml session without --load-save\n"; return 2;
    }
    if (!loadSaveFile.empty()) {
        std::ifstream saved(loadSaveFile);
        std::string version;
        std::string positionLine;
        if (!std::getline(saved, version) || (version != "DL1" && version != "DL2" && version != "DL3" && version != "DL4" && version != "DL5") ||
            !std::getline(saved, playLevel) || !std::getline(saved, positionLine) ||
            !std::filesystem::exists(playLevel)) {
            std::cerr << "Invalid or missing save: " << loadSaveFile << '\n'; return 2;
        }
        std::istringstream position(positionLine);
        if (!(position >> savedX >> savedY) ||
            (version != "DL1" && !(position >> savedHealth >> savedFocus)) ||
            !std::isfinite(savedX) || !std::isfinite(savedY) ||
            !std::isfinite(savedHealth) || !std::isfinite(savedFocus)) {
            std::cerr << "Invalid save state: " << loadSaveFile << '\n'; return 2;
        }
        std::string line;
        while (std::getline(saved, line)) {
            if (line.rfind("K:", 0) == 0) ownedKeys.insert(line.substr(2));
            else if (line.rfind("D:", 0) == 0) unlockedDoors.insert(line.substr(2));
            else if (line.rfind("P:", 0) == 0) collectedPapers.insert(line.substr(2));
            else if (line.rfind("V:", 0) == 0) rewardedHealthDevils.insert(line.substr(2));
            else if (line.rfind("Z:", 0) == 0) brokenSmashables.insert(line.substr(2));
            else if (line.rfind("J:", 0) == 0) {
                try {
                    size_t end = 0; const auto value = std::stoll(line.substr(2), &end);
                    if (end != line.size()-2 || value < 0 || value >= 1000000000) throw std::invalid_argument("devil count");
                    healthDevilRewardCount = static_cast<int>(value);
                } catch (const std::exception&) { std::cerr << "Invalid health devil count in save\n"; return 2; }
            }
            else if (line == "U:kDeflection") deflectionUnlocked = true;
            else if (line == "U:kSpeedIncrease") speedUnlocked = true;
            else if (line == "U:kDownwardDash") downwardDashUnlocked = true;
            else if (line == "U:kRoll") rollUnlocked = true;
            else if (line == "U:kProjectileDamageIncrease") projectileDamageUnlocked = true;
            else if (line == "U:kProjectileSpeedIncrease") projectileSpeedUnlocked = true;
            else if (line.rfind("H:", 0) == 0) {
                try {
                    size_t end = 0;
                    const auto value = std::stoul(line.substr(2), &end);
                    if (end != line.size()-2 || value < 1 || value > 1000000) throw std::invalid_argument("max health");
                    savedMaxHealth = static_cast<float>(value);
                    hasSavedMaxHealth = true;
                } catch (const std::exception&) { std::cerr << "Invalid max health in save\n"; return 2; }
            }
            else if (line.rfind("C:", 0) == 0) {
                try {
                    size_t end = 0;
                    const auto value = std::stoll(line.substr(2), &end);
                    if (end != line.size()-2 || value < 0 || value >= 1000000000) throw std::invalid_argument("completions");
                    completions = static_cast<int>(value);
                } catch (const std::exception&) { std::cerr << "Invalid completion count in save\n"; return 2; }
            }
            else if (line.rfind("G:", 0) == 0) language = line.substr(2);
            else if (version == "DL5" && line.rfind("W1:", 0) == 0) {
                if (!windowsPermanentExtension.empty() || line.size() <= 3 || line.size() > 65536 ||
                    line.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=", 3) != std::string::npos) {
                    std::cerr << "Invalid Windows migration extension in save\n"; return 2;
                }
                windowsPermanentExtension = line;
            }
            else if (version == "DL5" && line.rfind("B:", 0) == 0) {
                try {
                    size_t end = 0;
                    const auto value = std::stoll(line.substr(2), &end);
                    if (balanceLoaded || end != line.size()-2 || value < 0 || value > 1000000000) throw std::invalid_argument("balance");
                    orbBalance = static_cast<int>(value); balanceLoaded = true;
                } catch (const std::exception&) { std::cerr << "Invalid orb balance in save\n"; return 2; }
            }
            else if ((version == "DL4" || version == "DL5") && line.rfind("N:", 0) == 0) {
                const auto tab = line.rfind('\t');
                if (tab != std::string::npos) {
                    try { triggerHistory[line.substr(2, tab-2)] = std::stoll(line.substr(tab+1)); }
                    catch (const std::exception&) { std::cerr << "Invalid trigger history in save\n"; return 2; }
                }
            } else if (!line.empty()) collectedOrbs.insert(line);
        }
        if (!balanceLoaded) orbBalance = static_cast<int>(collectedOrbs.size());
        if (version == "DL5" && !balanceLoaded) { std::cerr << "Missing orb balance in save\n"; return 2; }
        restorePosition = true;
        if (saveFile.empty()) saveFile = loadSaveFile;
    }
    if (!selectedLanguage.empty()) language = selectedLanguage;
    // With no explicit level, launch into the authored main-menu slice.
    if (playLevel.empty() && levelPreview.empty()) menuLoop = true;
    if ((simulateOnly || !script.empty()) && playLevel.empty() && !menuLoop) { std::cerr << "Simulation options require --play\n"; return 2; }
    if (simulateWallClock && (!simulateOnly || fixedDt <= 0.f)) {
        std::cerr << "--simulate-wall-clock requires --simulate-only and --fixed-dt\n"; return 2;
    }
    if (testRefreshHz && (!simulateOnly || fixedDt > 0.f || script.empty() || frameLimit <= 0 ||
                          testRefreshHz < 30 || testRefreshHz > 240)) {
        std::cerr << "--test-refresh-hz requires --simulate-only, --play, --script, --frames, no --fixed-dt, and 30..240 Hz\n";
        return 2;
    }
    if (refreshSlots && (fixedDt > 0.f || simulateOnly || repeatPresentationCheck || !fullscreen || !requestedRefreshHz)) {
        std::cerr << "--refresh-slots requires live fullscreen GPU play, --refresh-hz, and no fixed-dt/repeat diagnostic\n"; return 2;
    }
    if (!mixedReplayPrefix.empty() && (mixedReplayFrame <= 0 || mixedReplayFrame >= frameLimit || simulateOnly)) {
        std::cerr << "--mixed-replay-prefix requires --mixed-replay-frame in [1,frames-1] and GPU rendering\n"; return 2;
    }
    if (repeatPresentationCheck && (capture.empty() || frameLimit <= 0 || simulateOnly)) {
        std::cerr << "--repeat-presentation-check requires --capture, --frames and GPU rendering\n"; return 2;
    }
    if (fixedDt < 0 || fixedDt > 0.05f) { std::cerr << "--fixed-dt must be between 0 and 0.05\n"; return 2; }
    std::vector<char> scriptedInput;
    if (!script.empty()) {
        std::istringstream parts(script); std::string token;
        while (std::getline(parts, token, ',')) {
            auto colon = token.find(':');
            if (colon == std::string::npos || token.empty() || std::string("LRJ.FBNTQEIHMPXSOYZC").find(token[0]) == std::string::npos) { std::cerr << "Bad --script token\n"; return 2; }
            int count = std::atoi(token.c_str() + colon + 1);
            if (count < 1 || count > 100000 || scriptedInput.size() + count > 100000) { std::cerr << "Bad --script duration\n"; return 2; }
            scriptedInput.insert(scriptedInput.end(), count, token[0]);
        }
        if (!frameLimit) frameLimit = scriptedInput.size();
    }
    std::unordered_map<int, MouseState> scriptedMouse;
    if (!mouseScriptFile.empty()) {
        std::ifstream events(mouseScriptFile);
        std::string line;
        int lastFrame = -1;
        while (std::getline(events, line)) {
            std::istringstream row(line);
            int at = -1, down = -1; float x = -1, y = -1; char comma1, comma2, comma3;
            if (!(row >> at >> comma1 >> x >> comma2 >> y >> comma3 >> down) ||
                comma1 != ',' || comma2 != ',' || comma3 != ',' || at <= lastFrame ||
                at > 100000 || (down != 0 && down != 1) || !std::isfinite(x) || !std::isfinite(y) ||
                (row >> std::ws && !row.eof())) { std::cerr << "Bad --mouse-script row\n"; return 2; }
            scriptedMouse[at] = {x, y, down != 0}; lastFrame = at;
        }
        if (!events.eof() || lastFrame < 0) { std::cerr << "Bad --mouse-script file\n"; return 2; }
        if (!frameLimit) frameLimit = lastFrame + 1;
    }
    // Simulation-only manager-visitation trace. It never changes a live camera
    // or save, and permits connected cull/revisit tests impossible with the
    // short steady-state camera lag and ordinary player movement.
    std::unordered_map<int, std::pair<float, float>> scriptedCamera;
    if (!cameraScriptFile.empty()) {
        if (!simulateOnly || playLevel.empty()) { std::cerr << "--camera-script requires --simulate-only --play\n"; return 2; }
        std::ifstream events(cameraScriptFile);
        int lastFrame = -1;
        std::string line;
        while (std::getline(events, line)) {
            std::istringstream row(line);
            int at = -1; float x = 0, y = 0; char comma1, comma2;
            if (!(row >> at >> comma1 >> x >> comma2 >> y) ||
                comma1 != ',' || comma2 != ',' || at <= lastFrame || at > 100000 ||
                !std::isfinite(x) || !std::isfinite(y) || (row >> std::ws && !row.eof())) {
                std::cerr << "Bad --camera-script row\n"; return 2;
            }
            scriptedCamera[at] = {x, y}; lastFrame = at;
        }
        if (!events.eof() || lastFrame < 0) { std::cerr << "Bad --camera-script file\n"; return 2; }
    }
    // Explicit opt-in for Docker/dummy-video functional tests only.
    const char* testSoftwareEnv = std::getenv("DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER");
    const bool testAllowSoftware = testSoftwareEnv && std::string(testSoftwareEnv) == "1";
    auto renderer = CreateRenderer(testAllowSoftware);
    deathleak::linux_port::PresentationRecordingRenderer* repeatRecorder = nullptr;
    deathleak::linux_port::FullFrameSnapshot priorFullFrame;
    bool hasPriorFullFrame = false;
    PacketTraceRenderer* packetTrace = nullptr;
    if (!atomicPostfadePacket.empty() || !atomicId22Prefix.empty()) {
        auto traced = std::make_unique<PacketTraceRenderer>(std::move(renderer), atomicId22Prefix);
        packetTrace = traced.get();
        renderer = std::move(traced);
    }
#ifdef DEATHLEAK_TEST_BODY_SLOW_FRAME_COMMAND
    auto observer = std::make_unique<BodySlowFrameObserver>(std::move(renderer));
    auto* bodySlowFrameObserver = observer.get();
    renderer = std::move(observer);
#endif
    if (repeatPresentationCheck || refreshSlots || fullFrameTopologyCheck) {
        auto recording = std::make_unique<deathleak::linux_port::PresentationRecordingRenderer>(std::move(renderer));
        repeatRecorder = recording.get();
        renderer = std::move(recording);
    }
    std::string error;
    renderer->SetRequestedRefreshHz(requestedRefreshHz);
    if (!renderer->Initialize("Death Leak - Linux renderer slice", windowWidth, windowHeight, fullscreen, error)) {
        std::cerr << "Renderer initialization failed: " << error << '\n'; return 1;
    }
    std::cerr << "Window requested=" << windowWidth << "x" << windowHeight << " fullscreen=" << fullscreen << " logical=1280x720\n";
    std::unique_ptr<deathleak::audio::SoundPlayer> sound;
    if (!simulateOnly && !playLevel.empty()) {
        sound = deathleak::audio::CreateSoundPlayer();
        std::string audioError;
        if (!sound->Initialize(audioError)) {
            std::cerr << "Audio unavailable: " << audioError << '\n';
            sound.reset();
        } else sound->SetSfxEnabled(settings.sfx);
    }
    deathleak::game::LevelScene scene;
    deathleak::game::WeatherLightning weather;
    deathleak::game::WeatherRain rain;
    deathleak::game::WeatherTextureResidency weatherTextures;
    std::string levelPath = !playLevel.empty() ? playLevel : levelPreview;
    if (!levelPath.empty() && !scene.Load(levelPath, *renderer, error)) {
        std::cerr << error << '\n'; return 1;
    }
    if (restorePosition) scene.ApplyCollected(collectedOrbs);
    scene.ApplyKeys(ownedKeys);
    scene.ApplyPapers(collectedPapers);
    scene.ApplyHealthDevil(rewardedHealthDevils.count(levelPath) != 0);
    scene.ApplySmashable(brokenSmashables.count(levelPath) != 0);
    scene.ApplyDeflection(deflectionUnlocked);
    if (!levelPath.empty()) {
        std::cout << "Loaded " << scene.SpriteCount() << " original level sprites; "
                  << scene.SkippedCount() << " missing or unsupported assets; "
                  << scene.Ground().size() << " ground segments; " << scene.Triggers().size() << " level triggers\n";
    }
    auto background = (levelPath.empty() || menuLoop) ? renderer->LoadTexture("Game/Media/UI/main_menu_2.png", error) : InvalidTexture;
    if ((levelPath.empty() || menuLoop) && !background) { std::cerr << "Background load failed: " << error << '\n'; return 1; }
    auto player = (levelPath.empty() && !menuLoop) ? renderer->LoadTexture("Game/Media/UI/player_silhouette.png", error) : InvalidTexture;
    if (levelPath.empty() && !menuLoop && !player) { std::cerr << "Player art load failed: " << error << '\n'; return 1; }
    float playerX = 590, playerY = 430;
    deathleak::game::PlayerMotion motion;
    motion.SetImpactVisualDriven(!simulateOnly);
    deathleak::game::CameraShake cameraShake;
    motion.SetSpeedUpgrade(speedUnlocked);
    motion.SetMovementUnlocks(downwardDashUnlocked, rollUnlocked);
    motion.Reset(restorePosition ? savedX : scene.PlayerX(), restorePosition ? savedY : scene.PlayerY(), !restorePosition);
    float worldX = motion.X(), worldY = motion.Y();
    float cameraX = scene.CameraX(), cameraY = scene.CameraY();
    deathleak::game::RightStickCameraTarget cameraTarget(scene.CameraOffsetX(), scene.CameraOffsetY());
    if (startEncounter >= 0) {
        if (playLevel.empty() || startEncounter >= static_cast<int>(scene.Encounters().size())) { std::cerr << "Invalid --start-encounter index\n"; return 2; }
        worldX = scene.Encounters()[startEncounter].x; worldY = scene.Encounters()[startEncounter].y;
        cameraX = worldX; cameraY = worldY; motion.Reset(worldX, worldY);
    }
    if (startHazard >= 0) {
        if (playLevel.empty() || startHazard >= static_cast<int>(scene.Hazards().size())) { std::cerr << "Invalid --start-hazard index\n"; return 2; }
        worldX = scene.Hazards()[startHazard].x; worldY = scene.Hazards()[startHazard].y;
        cameraX = worldX; cameraY = worldY; motion.Reset(worldX, worldY);
    }
    auto deathArt = (playLevel.empty() && !menuLoop) ? InvalidTexture : renderer->LoadTexture("Game/Media/UI/you_died.png", error);
    deathleak::game::Hud hud;
    if (!playLevel.empty() || menuLoop) hud.Load(*renderer);
    deathleak::game::Combat combat;
    // Decode/upload combat art before the first active frame. Loading it on the
    // first encounter can otherwise stall the physical display for hundreds of ms.
    if (!simulateOnly && !playLevel.empty()) combat.SetArt(*renderer);
    combat.SetDeflectionUnlocked(deflectionUnlocked);
    combat.SetProjectileUpgrades(projectileDamageUnlocked, projectileSpeedUnlocked);
    combat.SetMaxHealth(savedMaxHealth);
    combat.SetTriggerHistory(std::move(triggerHistory));
    if (simulateWallClock) {
        combat.EnableSimulatedWallClock();
        std::cout << "TEST-ONLY encounter clock follows fixed gameplay dt; not physical real-time acceptance\n";
    }
    if (restorePosition) combat.SetVitals(savedHealth, savedFocus);
    if (exhaustedFocus) combat.SetFocus(0);
    double updateSeconds = 0, drawSeconds = 0;
    std::vector<double> profiledFrameMs;
    constexpr int kProfileWarmupFrames = 120;
    bool wasDead = false, restartHeld = false;
    bool dashHeld = false, rollHeld = false;
    // Player::Update freezes the final scene, reveals its title after 15 s,
    // and calls EndStory after 35 s (both thresholds are strict).
    float finalSceneTime = 0.f;
    bool finalTitleShown = false, storyEnded = false;
    float levelTime = 0.f, interactHeldTime = 0.f, doorUnlockWait = 0.f;
    bool unlockingDoor = false;
    bool upgradeModal = false, paperModal = false;
    deathleak::game::PaperText paperText;
    if (!paperText.Load("Game/XmlFiles/strings.xml")) { std::cerr << "Cannot load Game/XmlFiles/strings.xml\n"; return 1; }
    std::string paperTitle, paperDescription;
    float modalWarmup = 0.f;
    bool purchaseHeld = false, dismissHeld = false;
    auto modalBackground = (playLevel.empty() && !menuLoop) ? InvalidTexture : renderer->LoadTexture("Game/Media/UI/blank_dark.png", error);
    // gameloading.xml's sole active sprite is blank_dark.png. Its authored
    // 2048-square rectangle covers the entire 1920x1080 UI viewport; the
    // texture itself is uniformly black, so a viewport-sized quad is exact.
    if (!playLevel.empty() && !modalBackground)
        std::cerr << "Loading splash texture load failed: " << error << '\n';
    enum class LoadingPhase { None, FadeIn, OpaqueHold, FadeOut };
    LoadingPhase loadingPhase = LoadingPhase::None;
    float loadingPhaseTime = 0.f;
    std::string loadingDestination;
    constexpr float kLoadingFadeSeconds = 0.20f;
    constexpr float kLoadingMinimumSeconds = 0.05f;
    bool packetSawWaterfallLoading = false;
    int packetFirstPostfadeOrdinal = -1;
    auto finalFrame = (playLevel.empty() && !menuLoop) ? InvalidTexture : renderer->LoadTexture("Game/Media/UI/frame_top.png", error);
    if (!finalFrame && !(playLevel.empty() && !menuLoop))
        std::cerr << "HUD frame_top.png load failed: " << error << '\n';
    deathleak::game::FilmFrameResidency filmFrames;
    // game_hud's scratch widget uses monotonic uptime at 11 FPS, not the
    // capped gameplay/shader delta. Original last-switch state starts at zero.
    deathleak::linux_port::FilmFrameSelector gameplayFilm;
    // final_scene_hud is a distinct retained animated widget.
    deathleak::linux_port::FilmFrameSelector finalFilm;
    if (!simulateOnly && playLevel == "Game/XmlFiles/levels/sea_2.xml")
        filmFrames.Prepare(*renderer, error);
    auto modalOrb = (playLevel.empty() && !menuLoop) ? InvalidTexture : renderer->LoadTexture("Game/Media/UI/orb.png", error);
    auto paperBackButton = (playLevel.empty() && !menuLoop) ? InvalidTexture : renderer->LoadTexture("Game/Media/UI/gamepad_icons/key_esc.png", error);
    auto modalButton = (playLevel.empty() && !menuLoop) ? InvalidTexture : renderer->LoadTexture("Game/Media/UI/gamepad_icons/y.png", error);
    std::string interactionTarget;
    struct DevilDialog { std::string level; float x, y, width, height, elapsed, duration; unsigned voice; int depth; bool updateable, alwaysUpdate; };
    struct HealthPickup { std::string level; float x, y, vx, vy, gravityAcceleration = 0.f; };
    std::optional<DevilDialog> devilDialog;
    std::optional<HealthPickup> healthPickup;
    bool pickupUpdatedThisFrame = false;
    const auto healthPickupArt = renderer->LoadTexture("Game/Media/objects/health_upgrade.png", error);
    int facing = 1;
    int movementDirection = 1;
    std::string currentLevel = playLevel;
    // has_ungutted is a temporary session flag in the original, not a DL5 field.
    // Only the explicit new forest-1 entry requests the intro; loaded saves and
    // ordinary --play remain controllable, even if they use the same level.
    bool hasPulledSword = !freshSaveIntro;
    float introHeldTime = 0.f;
    IntroPromptClock introPromptClock;
    IntroPromptPresentation introPromptPresentation;
    // Steamworks IsSteamRunningOnSteamDeck is unavailable in this port. Select
    // Deck art explicitly, never from mere controller connection.
    const auto introPromptArt = freshSaveIntro && !simulateOnly ? renderer->LoadTexture(
        steamDeck ? "Game/Media/UI/gamepad_icons/x_deck.png" : "Game/Media/UI/gamepad_icons/x.png", error) : InvalidTexture;
    if (freshSaveIntro && !simulateOnly && !introPromptArt)
        std::cerr << "Fresh-save X prompt texture load failed: " << error << '\n';
    const float introLoadZoom = scene.CameraZoom();
    float introEffectiveZoom = introLoadZoom;
    if (!hasPulledSword) scene.SetIntroBodyPhase(deathleak::game::LevelScene::IntroBodyPhase::Looped);
    // Main-menu assets and ordering follow mainmenu.xml. Keyboard/controller
    // navigation is a Linux accessibility fallback for the source mouse widgets.
    auto menuTitle = menuLoop ? renderer->LoadTexture("Game/Media/UI/title.png", error) : InvalidTexture;
    auto menuBlur = menuLoop ? renderer->LoadTexture("Game/Media/UI/main_menu_blur.png", error) : InvalidTexture;
    auto titleBlur = menuLoop ? renderer->LoadTexture("Game/Media/UI/title_blurred.png", error) : InvalidTexture;
    auto menuFrame = menuLoop ? renderer->LoadTexture("Game/Media/UI/frame.png", error) : InvalidTexture;
    std::array<TextureId, 4> menuFilm{};
    if (menuLoop) {
        constexpr std::array<const char*, 4> names = {"film_frame.png", "film_frame_2.png", "film_frame_3.png", "film_frame_4.png"};
        for (size_t i = 0; i < names.size(); ++i)
            menuFilm[i] = renderer->LoadTexture(std::string("Game/Media/UI/") + names[i], error);
    }
    std::array<TextureId, 3> menuButtons{};
    std::array<TextureId, 3> menuFocus{};
    std::array<TextureId, 2> optionsOff{}, optionsOn{}, optionsOffFocus{}, optionsOnFocus{};
    if (menuLoop) {
        constexpr std::array<const char*, 3> normal = {"play1.png", "settings1.png", "back1.png"};
        constexpr std::array<const char*, 3> focused = {"play2.png", "settings2.png", "back2.png"};
        for (size_t i = 0; i < 3; ++i) {
            menuButtons[i] = renderer->LoadTexture(std::string("Game/Media/UI/buttons/") + normal[i], error);
            menuFocus[i] = renderer->LoadTexture(std::string("Game/Media/UI/buttons/") + focused[i], error);
        }
        constexpr std::array<const char*, 2> prefixes = {"music", "sfx"};
        for (size_t i = 0; i < prefixes.size(); ++i) {
            const std::string base = std::string("Game/Media/UI/") + prefixes[i];
            optionsOff[i] = renderer->LoadTexture(base + "_off.png", error);
            optionsOn[i] = renderer->LoadTexture(base + "_on.png", error);
            optionsOffFocus[i] = renderer->LoadTexture(base + "_off_focus.png", error);
            optionsOnFocus[i] = renderer->LoadTexture(base + "_on_focus.png", error);
        }
    }
    int menuSelection = 0;
    float menuFilmTime = 0;
    int mouseHover = -1, mousePressed = -1, mousePage = -1;
    bool lastMouseDown = false;
    MouseState scriptedMouseState;
    bool menuOptions = false, menuLanguages = false, menuUpHeld = false, menuDownHeld = false, menuActivateHeld = false, menuBackHeld = false;
    bool completionOccurred = false;
    std::string audioLevel;
    const auto persistMenuMetadata = [&]() -> bool {
        if (saveFile.empty()) return true;
        const std::string temporary = saveFile + ".tmp";
        std::ofstream saved(temporary, std::ios::trunc);
        saved << "DL6\nMENU\nC:" << completions << '\n';
        if (!language.empty()) saved << "G:" << language << '\n';
        saved.close();
        if (!saved) { std::cerr << "Menu save write failed: " << temporary << '\n'; return false; }
        std::error_code ec;
        std::filesystem::rename(temporary, saveFile, ec);
        if (ec) { std::cerr << "Menu save commit failed: " << ec.message() << '\n'; return false; }
        return true;
    };
    deathleak::game::ShaderClock shaderClock;
    float precedingRenderedModifiedDelta = 0.f;
    float cameraShakeTime = 0.f;
    auto start = std::chrono::steady_clock::now();
    auto last = std::chrono::steady_clock::now();
    deathleak::linux_port::FixedStepPacer livePacer(last);
    bool liveClockAnchored = false;
    deathleak::linux_port::RefreshSlotScheduler refreshClock(last, requestedRefreshHz ? requestedRefreshHz : 60);
    unsigned long long refreshOnlyPresents = 0;
    unsigned long long refreshSurfaceFailures = 0;
    std::vector<double> presentationIntervalsMs;
    auto previousPresentation = std::chrono::steady_clock::time_point{};
    const auto notePresentation = [&] {
        const auto presented = std::chrono::steady_clock::now();
        if (previousPresentation != std::chrono::steady_clock::time_point{})
            presentationIntervalsMs.push_back(std::chrono::duration<double, std::milli>(presented - previousPresentation).count());
        previousPresentation = presented;
    };
    const auto testClockStart = last;
    int testDisplaySlots = 0;
    auto fpsSampleStart = last;
    int presentedFramesInSample = 0;
    int displayedFps = -1;
    for (int frame = 0; !frameLimit || frame < frameLimit; ++frame) {
        if (renderer->PollQuit()) break;
        // Scripted/fixed-dt routes retain their historical one-update/one-render
        // behavior. Live play instead consumes fixed 60-Hz simulation quanta.
        if (fixedDt <= 0.f) {
            if (testRefreshHz && !livePacer.Due()) ++testDisplaySlots;
            const auto tickTime = testRefreshHz ? testClockStart +
                std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                    std::chrono::duration<double>(double(testDisplaySlots) / testRefreshHz)) :
                std::chrono::steady_clock::now();
            livePacer.Accumulate(tickTime);
            if (!livePacer.Due()) {
                if (refreshSlots && repeatRecorder && repeatRecorder->Packet().Sealed() &&
                    refreshClock.Due(tickTime)) {
                    if (renderer->PresentationSurfaceStable()) {
                        repeatRecorder->Replay();
                        if (renderer->PresentationSurfaceStable()) {
                            repeatRecorder->PresentReplay();
                            notePresentation();
                            ++refreshOnlyPresents;
                            ++presentedFramesInSample;
                            refreshClock.Presented(std::chrono::steady_clock::now());
                        } else { ++refreshSurfaceFailures; std::cerr << "Refresh surface changed during replay\n"; return 1; }
                    } else { ++refreshSurfaceFailures; std::cerr << "Refresh surface unstable before replay\n"; return 1; }
                }
                if (!testRefreshHz) std::this_thread::sleep_for(std::chrono::duration<double>(
                    std::min(livePacer.UntilNextSeconds(), 0.002)));
                --frame;
                continue;
            }
            livePacer.Consume();
        }
        // Decode the first visible cave layer during Forest traversal; keep GPU allocation on its normal load path.
        if (frame == 120 && currentLevel == "Game/XmlFiles/levels/forest_exploration_1.xml") StartCaveDecodeWorker();
        const ControllerSticks controllerSticks = renderer->ControllerAxes();
        deathleak::game::PlayerStrafeVisual strafeVisual{};
        auto now = std::chrono::steady_clock::now();
        const auto fpsSampleSeconds = std::chrono::duration<double>(now - fpsSampleStart).count();
        if (showFps && fpsSampleSeconds >= 0.5) {
            displayedFps = static_cast<int>(std::lround(presentedFramesInSample / fpsSampleSeconds));
            presentedFramesInSample = 0;
            fpsSampleStart = now;
        }
        float dt = fixedDt > 0.f ? fixedDt : 1.f / 60.f;
        last = now;
#ifdef DEATHLEAK_TEST_BODY_SLOW_FRAME_COMMAND
        const float requestedDt = dt;
#endif
        // Original program.cpp caps the modified update delta to its 60 Hz target
        // before Update/Render; the completed frame commits that same delta.
        constexpr float kTargetDelta = 1.f / 60.f;
        if (dt > kTargetDelta) dt = kTargetDelta;
        // This is the port's update delta; feed the modified delta here when
        // Windows-style damage slowdown is implemented.
#ifdef DEATHLEAK_TEST_BODY_SLOW_FRAME_COMMAND
        bodySlowFrameObserver->Start(frame, requestedDt, dt, precedingRenderedModifiedDelta);
#endif
        shaderClock.Update(dt);
        cameraShakeTime += dt;
        const char command = frame < static_cast<int>(scriptedInput.size()) ? scriptedInput[frame] : 0;
        // Game objects update before UIGameLoading changes the pause state.
        // The hold-exit frame is still paused; FadeOut-entry resumes gameplay.
        const bool loadingPausedThisFrame = loadingPhase == LoadingPhase::FadeIn ||
            loadingPhase == LoadingPhase::OpaqueHold;
        bool loadingHoldExitedThisFrame = false;
        const auto packetLoadingPhaseOnEntry = loadingPhase;
        bool packetGameplayRan = false, packetWeatherRan = false;
        bool packetSceneRan = false, packetPlayerRan = false;
#ifdef DEATHLEAK_TEST_LOADING_FADE_OBSERVER
        const auto loadingPhaseOnEntry = loadingPhase;
        bool loadingGameplayRan = false, loadingWeatherRan = false;
        bool loadingSceneVisualsRan = false, loadingPlayerVisualRan = false;
        int loadingDrawAlpha = -1;
#endif
        bool crouchInput = false;
        bool playerProjectileFired = false;
        bool playerEligibleFireAttempt = false;
        bool stickAim = false;
        float aimX = 0.f, aimY = 0.f;
        if (sound && currentLevel != audioLevel) {
            sound->SetAmbience(scene.Ambience());
            audioLevel = currentLevel;
            std::cout << "Audio level " << audioLevel << " authored_ambience=" << scene.Ambience().size() << '\n';
        }
        if (sound) sound->UpdateAmbience(cameraX, cameraY);

        if (renderer->KeyDown(Escape) && !upgradeModal && !paperModal && !dismissHeld && (!menuLoop || !playLevel.empty())) break;
        if (!renderer->KeyDown(Escape) && !upgradeModal && !paperModal) dismissHeld = false;
        if (menuLoop && playLevel.empty()) {
            menuFilmTime += dt;
            if (auto scripted = scriptedMouse.find(frame); scripted != scriptedMouse.end()) scriptedMouseState = scripted->second;
            const MouseState mouse = mouseScriptFile.empty() ? renderer->Mouse() : scriptedMouseState;
            const int page = menuLanguages ? 2 : menuOptions ? 1 : 0;
            mouseHover = -1;
            if (page == 0) {
                for (int i = 0; i < 3; ++i) if (Hit(menuBounds[i], mouse)) mouseHover = i;
            } else if (page == 1) {
                for (int i = 0; i < 2; ++i) if (Hit(optionsBounds[i], mouse)) mouseHover = i;
                if (Hit({500, 548, 250, 48}, mouse)) mouseHover = 2; // Linux language supplement.
                if (Hit(optionsBounds[2], mouse)) mouseHover = 3;
            }
            if (mouse.leftDown && !lastMouseDown) {
                mousePressed = mouseHover; mousePage = page;
                if (mousePressed >= 0) std::cout << "Menu mouse press page=" << page << " widget=" << mousePressed << " at frame " << frame << '\n';
            }
            const bool mouseActivate = !mouse.leftDown && lastMouseDown &&
                mousePressed >= 0 && mousePage == page && mousePressed == mouseHover;
            if (!mouse.leftDown && lastMouseDown) {
                std::cout << "Menu mouse release page=" << page << " widget=" << mousePressed
                          << " target=" << mouseHover << " activated=" << mouseActivate << " at frame " << frame << '\n';
                mousePressed = -1;
            }
            lastMouseDown = mouse.leftDown;
            const bool up = script.empty() ? renderer->KeyDown(Up) : command == 'J';
            const bool down = script.empty() ? renderer->KeyDown(Down) : command == 'N';
            const bool activate = script.empty() ? renderer->KeyDown(Interact) : command == 'S';
            const bool back = script.empty() ? renderer->KeyDown(Escape) : command == 'X';
            if (back && !menuBackHeld && menuLanguages) { menuLanguages = false; menuSelection = 2; }
            else if (back && !menuBackHeld && menuOptions) { menuOptions = false; menuSelection = 0; }
            else if (back && !menuBackHeld) break;
            const int menuCount = menuLanguages || menuOptions ? 4 : 3;
            if (up && !menuUpHeld) { menuSelection = (menuSelection + menuCount - 1) % menuCount; mouseHover = -1; }
            if (down && !menuDownHeld) { menuSelection = (menuSelection + 1) % menuCount; mouseHover = -1; }
            if (mouseActivate) menuSelection = mouseHover;
            if ((activate && !menuActivateHeld) || mouseActivate) {
                if (menuLanguages) {
                    if (menuSelection == 3) { menuLanguages = false; menuSelection = 2; }
                    else {
                        constexpr std::array<const char*, 3> locales = {"en-gb", "pt-br", "de"};
                        language = locales[menuSelection];
                        std::cout << "Menu language selected " << language << " at frame " << frame << '\n';
                        if (!persistMenuMetadata()) return 1;
                    }
                } else if (menuOptions) {
                    if (menuSelection == 3) { menuOptions = false; menuSelection = 0; }
                    else if (menuSelection == 2) { menuLanguages = true; menuSelection = 0; }
                    else {
                        if (menuSelection == 0) settings.music = !settings.music;
                        else settings.sfx = !settings.sfx;
                        if (!settings.Save(settingsFile)) { std::cerr << "Settings write failed: " << settingsFile << '\n'; return 1; }
                        if (sound) sound->SetSfxEnabled(settings.sfx);
                        std::cout << "Menu settings music=" << settings.music << " sfx=" << settings.sfx << " at frame " << frame << '\n';
                    }
                } else if (menuSelection == 2) break;
                else if (menuSelection == 1) { menuOptions = true; menuSelection = 0; }
                else {
                    const std::string freshLevel = "Game/XmlFiles/levels/grass_exploration_3.xml";
                    deathleak::game::LevelScene fresh;
                    if (!fresh.Load(freshLevel, *renderer, error)) { std::cerr << error << '\n'; return 1; }
                    scene = std::move(fresh);
                    cameraTarget = {scene.CameraOffsetX(), scene.CameraOffsetY()};
                    devilDialog.reset(); healthPickup.reset();
                    if (!simulateOnly && !sound) {
                        sound = deathleak::audio::CreateSoundPlayer();
                        std::string audioError;
                        if (!sound->Initialize(audioError)) { std::cerr << "Audio unavailable: " << audioError << '\n'; sound.reset(); }
                        else sound->SetSfxEnabled(settings.sfx);
                    }
                    playLevel = levelPath = currentLevel = freshLevel;
                    hasPulledSword = true; introHeldTime = 0.f; introPromptClock.Reset(); introEffectiveZoom = scene.CameraZoom();
                    collectedOrbs.clear(); ownedKeys.clear(); unlockedDoors.clear(); collectedPapers.clear(); rewardedHealthDevils.clear(); brokenSmashables.clear(); healthDevilRewardCount = 0;
                    orbBalance = 0; combat = deathleak::game::Combat{};
                    if (!simulateOnly) combat.SetArt(*renderer);
                    speedUnlocked = projectileDamageUnlocked = projectileSpeedUnlocked = false; downwardDashUnlocked = rollUnlocked = false; savedMaxHealth = 25; hasSavedMaxHealth = false; windowsPermanentExtension.clear();
                    motion.SetSpeedUpgrade(false); motion.SetMovementUnlocks(false, false);
                    motion.Reset(scene.PlayerX(), scene.PlayerY(), true);
                    worldX = motion.X(); worldY = motion.Y();
                    cameraX = scene.CameraX(); cameraY = scene.CameraY(); cameraShake.Reset();
                    finalSceneTime = levelTime = interactHeldTime = doorUnlockWait = 0;
                    finalTitleShown = storyEnded = wasDead = unlockingDoor = upgradeModal = paperModal = false;
                    interactionTarget.clear(); menuSelection = 0;
                    std::cout << "Menu New Game at frame " << frame << " entered " << currentLevel
                              << " completion count=" << completions << " language=" << language << '\n';
                }
            }
            menuUpHeld = up; menuDownHeld = down; menuActivateHeld = activate; menuBackHeld = back;
        } else if (playLevel.empty()) {
            if (renderer->KeyDown(Left)) playerX -= 250 * dt;
            if (renderer->KeyDown(Right)) playerX += 250 * dt;
            if (renderer->KeyDown(Up)) playerY -= 250 * dt;
            if (renderer->KeyDown(Down)) playerY += 250 * dt;
        } else if (loadingPausedThisFrame) {
            // UIGameLoading updates while gameplay is paused. The destination
            // is not installed until the outgoing scene is fully obscured.
            loadingPhaseTime += dt;
            if (loadingPhase == LoadingPhase::FadeIn && loadingPhaseTime > kLoadingFadeSeconds) {
                deathleak::game::LevelScene nextScene;
                if (nextScene.Load(loadingDestination, *renderer, error, currentLevel)) {
                    nextScene.ApplyCollected(collectedOrbs);
                    nextScene.ApplyKeys(ownedKeys);
                    nextScene.ApplyPapers(collectedPapers);
                    nextScene.ApplyHealthDevil(rewardedHealthDevils.count(loadingDestination) != 0);
                    nextScene.ApplySmashable(brokenSmashables.count(loadingDestination) != 0);
                    nextScene.ApplyDeflection(combat.DeflectionUnlocked());
                    scene = std::move(nextScene);
                    cameraTarget = {scene.CameraOffsetX(), scene.CameraOffsetY()};
                    devilDialog.reset(); healthPickup.reset();
                    currentLevel = loadingDestination; introPromptClock.Reset();
                    worldX = scene.PlayerX(); worldY = scene.PlayerY();
                    // Original GameObjectManager calls FollowObjectsOrigin(player) after
                    // LevelEntry places the player. Snap only authored follow axes;
                    // ordinary follow lag and bounds still apply on subsequent ticks.
                    cameraX = scene.CameraFollowX() ? worldX : scene.CameraX();
                    cameraY = scene.CameraFollowY() ? worldY : scene.CameraY();
                    cameraShake.Reset();
                    motion.Reset(worldX, worldY, true); combat.ClearLevel();
                    levelTime = interactHeldTime = doorUnlockWait = 0;
                    unlockingDoor = false; interactionTarget.clear();
                    std::cout << "Entered " << currentLevel << std::endl;
                } else std::cerr << "Transition failed: " << error << '\n';
                loadingPhase = LoadingPhase::OpaqueHold;
                loadingPhaseTime = 0.f;
            } else if (loadingPhase == LoadingPhase::OpaqueHold && loadingPhaseTime > kLoadingMinimumSeconds) {
                // Source starts the fade-out clock at Load(), but its loading
                // branch masks fade-out updates for at least 0.05 s. Preserve
                // elapsed time, including the first alpha step after the hold.
                loadingPhase = LoadingPhase::FadeOut;
                loadingHoldExitedThisFrame = true;
            }
        } else {
            packetGameplayRan = true;
#ifdef DEATHLEAK_TEST_LOADING_FADE_OBSERVER
            loadingGameplayRan = true;
#endif
            // FadeOut no longer pauses gameplay, but its UI clock still ticks
            // once per frame until the splash has fully disappeared.
            if (loadingPhase == LoadingPhase::FadeOut) loadingPhaseTime += dt;
            if (loadingPhase == LoadingPhase::FadeOut && loadingPhaseTime > kLoadingFadeSeconds) {
                loadingPhase = LoadingPhase::None;
                loadingPhaseTime = 0.f;
                loadingDestination.clear();
            }
            if (currentLevel == "Game/XmlFiles/levels/sea_2.xml") {
                finalSceneTime += dt;
                if (!finalTitleShown && finalSceneTime > 15.f) {
                    finalTitleShown = true;
                    if (sound) sound->Play(deathleak::audio::Cue::FinalTitle);
                    std::cout << "Final title DEATH LEAK at frame " << frame << " time=" << finalSceneTime << "\n";
                }
                if (finalSceneTime > 35.f) {
                    storyEnded = true;
                    ++completions;
                    std::cout << "EndStory at frame " << frame << " time=" << finalSceneTime << "\n";
                }
            } else {
            if (paperModal) {
                modalWarmup += dt;
                const bool dismiss = !script.empty() ? command == 'X' :
                    (renderer->KeyDown(Escape) || renderer->KeyDown(ModalBack));
                if (dismiss && !dismissHeld && modalWarmup >= 1.f) {
                    paperModal = false;
                    std::cout << "Dismissed paper modal at frame " << frame << '\n';
                }
                dismissHeld = dismiss;
            } else if (upgradeModal) {
                modalWarmup += dt;
                const bool purchase = !script.empty() ? command == 'P' : renderer->KeyDown(Purchase);
                const bool dismiss = !script.empty() ? command == 'X' :
                    (renderer->KeyDown(Escape) || renderer->KeyDown(ModalBack));
                if (purchase && !purchaseHeld) {
                    if (orbBalance >= 375) {
                        orbBalance -= 375;
                        combat.SetDeflectionUnlocked(true);
                        scene.ApplyDeflection(true);
                        upgradeModal = false;
                        std::cout << "Purchased kDeflection at frame " << frame << " balance=" << orbBalance << '\n';
                    } else std::cout << "Not enough orbs for kDeflection at frame " << frame << " balance=" << orbBalance << '\n';
                } else if (dismiss && !dismissHeld && modalWarmup >= 1.f) {
                    upgradeModal = false;
                    std::cout << "Dismissed kDeflection modal at frame " << frame << '\n';
                }
                purchaseHeld = purchase; dismissHeld = dismiss;
            } else if (!hasPulledSword) {
            // Player::Update runs Character physics before its unpulled-sword
            // early return. Keep that step on the completion tick as well.
            motion.Update(scene, dt, 0, false);
            worldX = motion.X(); worldY = motion.Y();
            // Raw Interact is held independently of world-object activation.
            // Normal motion, combat, roll, jump, interaction, and right-stick
            // target updates remain suppressed through the one-shot pull.
            if (scene.IntroBodyFinished()) {
                hasPulledSword = true;
                scene.SetIntroBodyPhase(deathleak::game::LevelScene::IntroBodyPhase::None);
                introHeldTime = 0.f;
                introPromptClock.Reset();
                introEffectiveZoom = introLoadZoom;
                std::cout << "Fresh-save intro completed at frame " << frame << '\n';
                combat.UpdateBloodFlights(scene, dt);
            } else {
                if (scene.GetIntroBodyPhase() == deathleak::game::LevelScene::IntroBodyPhase::Looped) {
                    const bool held = !script.empty() ? command == 'I' : renderer->KeyDown(Interact);
                    introPromptClock.Advance(dt);
                    if (held) {
                        introHeldTime += dt;
                        if (introHeldTime > 2.75f) {
                            introHeldTime = 0.f;
                            introPromptClock.Reset();
                            introEffectiveZoom = introLoadZoom;
                            scene.EmitIntroSwordPullBurst(*renderer, worldX, worldY);
                            combat.SpawnIntroBloodFlight(worldX, worldY + 5.f);
                            scene.SetIntroBodyPhase(deathleak::game::LevelScene::IntroBodyPhase::Pull);
                            std::cout << "Fresh-save intro pull at frame " << frame << '\n';
                        } else {
                            // Source requests a 0.1s shake on each sub-threshold held
                            // update; active windows refuse restarts until they expire.
                            cameraShake.Start((introHeldTime / 2.75f) * 5.f, 0.1f, cameraShakeTime - dt);
                            introEffectiveZoom = std::max(introLoadZoom * 0.75f, introEffectiveZoom - dt * 0.05f);
                        }
                    } else {
                        introHeldTime = 0.f;
                        introEffectiveZoom = std::min(introLoadZoom, introEffectiveZoom + dt * 0.10f);
                    }
                }
                deathleak::game::FollowCameraWithTarget({
                    cameraX, cameraY, worldX, worldY,
                    introEffectiveZoom, scene.CameraLagX(), scene.CameraLagY(),
                    scene.CameraFollowX(), scene.CameraFollowY(), combat.EnemyCount() != 0,
                    scene.HasCameraBounds(), scene.CameraBoundLeft(), scene.CameraBoundRight(),
                    scene.CameraBoundTop(), scene.CameraBoundBottom()
                }, cameraTarget, cameraX, cameraY);
                if (auto it = scriptedCamera.find(frame); it != scriptedCamera.end()) {
                    cameraX = it->second.first; cameraY = it->second.second;
                }
                if (!simulateOnly && script.empty()) cameraTarget.Update(controllerSticks, false, false, false, false);
                combat.UpdateBloodFlights(scene, dt);
                levelTime += dt;
            }
            } else {
            const float direction = !script.empty() ? static_cast<float>(command == 'R' || command == 'B' || command == 'E' || command == 'Y') - static_cast<float>(command == 'L' || command == 'N' || command == 'Q' || command == 'Z') :
                static_cast<float>(renderer->KeyDown(Right)) - static_cast<float>(renderer->KeyDown(Left));
            if (direction != 0) facing = movementDirection = direction < 0 ? -1 : 1;
            const bool restart = !script.empty() ? command == 'T' : renderer->KeyDown(Restart);
            if (combat.Health() <= 0) {
                if (!wasDead) { std::cout << "Player died at frame " << frame << " in " << currentLevel << std::endl; wasDead = true; }
                if (restart && !restartHeld) {
                    deathleak::game::LevelScene nextScene;
                    // Authored you_died.xml reloads grass_exploration_3.xml.
                    const std::string restartLevel = "Game/XmlFiles/levels/grass_exploration_3.xml";
                    if (nextScene.Load(restartLevel, *renderer, error)) {
                        nextScene.ApplyCollected(collectedOrbs);
                        nextScene.ApplyKeys(ownedKeys);
                        nextScene.ApplyPapers(collectedPapers);
                        nextScene.ApplyHealthDevil(rewardedHealthDevils.count(restartLevel) != 0);
                        nextScene.ApplySmashable(brokenSmashables.count(restartLevel) != 0);
                        nextScene.ApplyDeflection(combat.DeflectionUnlocked());
                        scene = std::move(nextScene);
                        cameraTarget = {scene.CameraOffsetX(), scene.CameraOffsetY()};
                        devilDialog.reset(); healthPickup.reset();
                        currentLevel = restartLevel; introPromptClock.Reset();
                        motion.Reset(scene.PlayerX(), scene.PlayerY(), true);
                        worldX = motion.X(); worldY = motion.Y();
                        cameraX = scene.CameraX(); cameraY = scene.CameraY(); cameraShake.Reset();
                        combat.ClearLevel(); combat.SetVitals(25.f, 100.f);
                        wasDead = false; levelTime = interactHeldTime = doorUnlockWait = 0; unlockingDoor = false; interactionTarget.clear();
                        std::cout << "Restarted " << currentLevel << " at frame " << frame << '\n';
                    } else std::cerr << "Restart failed: " << error << '\n';
                }
                restartHeld = restart;
            } else {
            restartHeld = restart;
            // Resolve crouch before movement; fire is resolved after the stick
            // deadzone below so the two controls remain independent on ground.
            crouchInput = ResolveGameplayAttackInput(!script.empty(), command, *renderer,
                motion.Grounded(), motion.WallSide() != 0, motion.Rolling(),
                combat.MeleeActive(), false).crouch;
            const bool jump = !script.empty() ? (command == 'J' || command == 'Q' || command == 'E' || command == 'Y' || command == 'Z') : renderer->KeyDown(Up);
            // Windows right shoulder attempts an airborne dash and a grounded melee.
            // O/left shoulder is roll; the motion layer keeps the landing input window.
            const bool melee = !script.empty() ? command == 'M' : renderer->KeyDown(Melee);
            const bool roll = !script.empty() ? command == 'O' : renderer->KeyDown(Roll);
            if (melee && !dashHeld && combat.Health() > 0.f && !combat.MeleeActive() &&
                motion.TryDownwardDash(combat.HasFocus(50.f))) combat.ConsumeFocus(50.f);
            dashHeld = melee;
            if (roll && !rollHeld && combat.Health() > 0.f && !combat.MeleeActive()) {
                motion.BufferRoll();
                if (motion.TryRoll(combat.HasFocus(2.f))) combat.ConsumeFocus(2.f);
            }
            rollHeld = roll;
            // Follow the previous input tick's target before advancing the player.
            // The helper retains the authored unzoomed Y/X clamp order and X lag.
            deathleak::game::FollowCameraWithTarget({
                cameraX, cameraY, worldX, worldY,
                scene.CameraZoom(), scene.CameraLagX(), scene.CameraLagY(),
                scene.CameraFollowX(), scene.CameraFollowY(), combat.EnemyCount() != 0,
                scene.HasCameraBounds(), scene.CameraBoundLeft(), scene.CameraBoundRight(),
                scene.CameraBoundTop(), scene.CameraBoundBottom()
            }, cameraTarget, cameraX, cameraY);
            if (auto it = scriptedCamera.find(frame); it != scriptedCamera.end()) {
                cameraX = it->second.first; cameraY = it->second.second;
            }
            const bool wasGrounded = motion.Grounded();
            motion.Update(scene, dt, (combat.MeleeActive() || motion.Rolling() || crouchInput) ? 0 : static_cast<int>(direction),
                          jump && !combat.MeleeActive());
            // Scripted input has no controller strafe resolution, but Roll footstep
            // spray still follows the last horizontal movement direction.
            strafeVisual.movementDirection = movementDirection;
            if (script.empty()) strafeVisual = deathleak::game::ResolvePlayerStrafeVisual(
                controllerSticks, movementDirection, motion.Rolling(), combat.MeleeActive(),
                !wasGrounded && motion.Grounded());
            if (motion.SmashedSomething()) {
                brokenSmashables.insert(currentLevel);
                if (sound) sound->Play(deathleak::audio::Cue::StoneSmash);
                std::cout << "Smashed " << currentLevel << " at frame " << frame << " spawned_orbs=" << scene.PendingSpawnedOrbs() << '\n';
            }
            if (motion.Grounded() && combat.Health() > 0.f && motion.TryBufferedRoll(combat.HasFocus(2.f)))
                combat.ConsumeFocus(2.f);
            for (unsigned n = motion.ConsumeJumpRollVocalActions(); n > 0; --n) {
                const int draw = std::rand() % 20;
                if (sound) deathleak::audio::DispatchJumpRollVocal(*sound, draw);
            }
            if (const auto impact = motion.ConsumeHardLandingEvent();
                impact != deathleak::game::PlayerMotion::HardLandingEvent::None) {
                cameraShake.StartMedium(cameraShakeTime - dt);
            }
            worldX = motion.X(); worldY = motion.Y();
            scene.UpdateAuthoredOrbs(dt, worldX, worldY, cameraX, cameraY);
            const auto collectedNow = scene.CollectCurrencyOrbs(worldX, worldY, collectedOrbs);
            const auto collectedSpawned = scene.UpdateSpawnedOrbs(worldX, worldY, dt);
            orbBalance += static_cast<int>(collectedNow + collectedSpawned);
            if (sound && (collectedNow || collectedSpawned)) sound->Play(deathleak::audio::Cue::OrbCollect);
            // XInput activates aim/fire when either right axis exceeds its
            // strict deadzone. SDL Y is screen-down, so convert to world-up.
            constexpr int rightDeadzone = 8689;
            const int rightY = deathleak::game::RightStickCameraTarget::WorldRightY(controllerSticks.rightY);
            stickAim = script.empty() && !simulateOnly && controllerSticks.connected &&
                (std::abs(controllerSticks.rightX) > rightDeadzone || std::abs(rightY) > rightDeadzone);
            const bool fire = ResolveGameplayAttackInput(!script.empty(), command, *renderer,
                wasGrounded, motion.WallSide() != 0, motion.Rolling(),
                combat.MeleeActive(), stickAim).fire;
            aimX = stickAim ? static_cast<float>(controllerSticks.rightX) : 0.f;
            aimY = stickAim ? static_cast<float>(rightY) : 0.f;
            playerEligibleFireAttempt = fire && !motion.Rolling() && !motion.InWater() &&
                !combat.MeleeActive() && combat.Health() > 0.f && motion.WallSide() == 0;
            const int firedBefore = combat.Fired();
            const float healthBeforeCombat = combat.Health();
            const int spawnedBeforeCombat = combat.Spawned();
            // Character::Update clears its air flags on solid ground or at a
            // wall, and changes from up to down only below -0.5 unit/tick.
            // PlayerMotion stores units/second. An authored-entry zero velocity
            // is not an airborne launch state; source also cancels crouch in air.
            const float verticalSpeed = motion.VelocityY();
            const bool shotAirborne = !motion.Grounded() && motion.WallSide() == 0 &&
                !motion.InWater() && verticalSpeed != 0.f;
            const deathleak::game::PlayerShotLaunchState launchState{
                fire && crouchInput && motion.Grounded() && motion.WallSide() == 0 &&
                    !motion.Rolling() && !combat.MeleeActive(),
                shotAirborne && verticalSpeed > -30.f,
                shotAirborne && verticalSpeed <= -30.f
            };
            combat.Update(scene, worldX, worldY, dt, fire, melee, motion.Grounded(), motion.Rolling(), facing, *renderer, aimX, aimY, launchState);
            playerProjectileFired = combat.Fired() > firedBefore;
            // SolidMovingSprite::OnCollision applies authored damage per contact, not per second.
            combat.ApplySolidContactDamage(motion.SolidContactDamage());
            // Windows input writes the next follow target after movement/combat.
            // A buffered roll may clear impact recovery on this same update.
            if (!simulateOnly && script.empty()) cameraTarget.Update(controllerSticks, combat.Health() > 0.f,
                motion.Rolling(), motion.ImpactRecovering(), combat.MeleeActive());
            if (std::getenv("DEATHLEAK_TRACE_ENCOUNTERS") &&
                (combat.Spawned() != spawnedBeforeCombat || combat.Health() != healthBeforeCombat))
                std::cout << "Encounter state frame=" << frame << " level=" << currentLevel
                          << " spawned_delta=" << combat.Spawned() - spawnedBeforeCombat
                          << " health_before=" << healthBeforeCombat << " health_after=" << combat.Health()
                          << " solid_damage=" << motion.SolidContactDamage()
                          << " enemies=" << combat.EnemyCount() << std::endl;
            // H remains a simulation-only direct hit for isolated combo/stun
            // checks; normal melee comes through Combat::Update above.
            if (simulateOnly && command == 'H') combat.DamageEnemy(0,1.5f);
            if (sound && playerProjectileFired) sound->Play(deathleak::audio::Cue::KnifeThrow, firedBefore);
            levelTime += dt;
            // Utilities::IsObjectInRectangle culls by drawable extents, not
            // collision bounds. These interactables have zero parallax in the
            // supported scene model; the native manager zone is 4000 square.
            auto inManagerZone = [&](float x, float y, float width, float height) {
                return std::abs(x-cameraX) < 2000.f + width*0.5f &&
                       std::abs(y-cameraY) < 2000.f + height*0.5f;
            };
            pickupUpdatedThisFrame = bool(healthPickup);
            if (devilDialog) {
                if (devilDialog->level != currentLevel) devilDialog.reset();
                else {
                    devilDialog->elapsed += dt;
                    // Fixed-step simulation uses authored PCM duration; live
                    // playback waits for the actual SDL voice to finish.
                    const bool finished = simulateOnly
                        ? (!settings.sfx || devilDialog->elapsed >= devilDialog->duration)
                        : (!sound || sound->VoiceFinished(devilDialog->voice));
                    // The voice continues while the devil is culled, but
                    // GiveReward runs only on a visited HealthDevil::Update.
                    if (finished && devilDialog->updateable &&
                        (devilDialog->alwaysUpdate || inManagerZone(devilDialog->x, devilDialog->y,
                                                                     devilDialog->width, devilDialog->height))) {
                        rewardedHealthDevils.insert(currentLevel);
                        ++healthDevilRewardCount;
                        scene.ApplyHealthDevil(true);
                        healthPickup = HealthPickup{currentLevel, devilDialog->x,
                            devilDialog->y + 20.f, 15.f, 10.f};
                        // std::list::sort keeps the current HealthDevil iterator
                        // valid. A new kPlayer pickup is visited later in this
                        // pass only if it sorts after the devil (including ties).
                        pickupUpdatedThisFrame = devilDialog->depth >= 3000;
                        std::cout << "Health devil reward spawned " << currentLevel
                                  << " at frame " << frame << " count=" << healthDevilRewardCount << '\n';
                        devilDialog.reset();
                    }
                }
            }
            // GameObjectManager::Update visits a pickup only inside its
            // camera-centered 4000x4000 zone (1920x1080 backbuffer). The
            // native 50x50 art can still overlap the zone when its origin
            // has crossed the edge; source Utilities uses strict bounds.
            // This is evaluated before motion, as in the manager loop.
            if (healthPickup && pickupUpdatedThisFrame) {
                pickupUpdatedThisFrame = inManagerZone(healthPickup->x, healthPickup->y, 50.f, 50.f);
            }
            // GameObject::UpdateInteractable runs in Sprite::Update before
            // MovingSprite advances a visited pickup. Test this frame's
            // interaction against the position at the start of its update.
            const float pickupInteractionX = healthPickup ? healthPickup->x : 0.f;
            const float pickupInteractionY = healthPickup ? healthPickup->y : 0.f;
            if (healthPickup && pickupUpdatedThisFrame) {
                if (healthPickup->level != currentLevel) healthPickup.reset();
                else {
                    // MovingSprite::Update adds last frame's acceleration before
                    // moving, then chooses next frame's gravity. Between 0 and
                    // -15 downward speed it falls at 3x gravity; vertical speed
                    // is capped by the authored HealthDevil spawn parameters.
                    const float steps = dt * 60.f;
                    // Pickup::OnCollision returns false. CollisionManager does
                    // not call a SolidLineStrip's opposite handler, and
                    // SolidMovingSprite ignores pickups. The authored upgrade
                    // therefore keeps falling through ground and solid boxes.
                    healthPickup->vy = std::clamp(healthPickup->vy - healthPickup->gravityAcceleration * steps,
                                                  -20.f, 20.f);
                    healthPickup->x += healthPickup->vx * steps;
                    healthPickup->y += healthPickup->vy * steps;
                    healthPickup->vx *= std::pow(0.95f, steps);
                    healthPickup->gravityAcceleration = healthPickup->vy > 0.f || healthPickup->vy < -15.f ? 0.8f : 2.4f;
                }
            }
            if (doorUnlockWait > 0) {
                doorUnlockWait = std::max(0.f, doorUnlockWait-dt);
                if (unlockingDoor) interactHeldTime = 0;
            }
            const bool interact = !script.empty() ? command == 'I' : renderer->KeyDown(Interact);
            // GameObject::UpdateInteractable uses player-rectangle overlap and 0.05 s hold.
            auto overlap = [&](float ox,float oy,float ow,float oh) {
                return std::abs(worldX-ox) <= (128.959793f+ow)*0.5f &&
                       std::abs(worldY-oy) <= (214.898743f+oh)*0.5f;
            };
            // A skipped GameObject::UpdateInteractable cannot clear the static
            // sCurrentInteractable lock or reset its own countdown. Determine
            // visitation independently of overlap/CanInteract before choosing
            // this frame's candidate. The lock is released only when its object
            // is actually visited and the player no longer overlaps it.
            auto visited = [&](const auto& object) {
                return object.updateable && (object.alwaysUpdate ||
                    inManagerZone(object.x, object.y, object.width, object.height));
            };
            bool lockFound = false, lockVisited = false, lockOverlaps = false;
            int releasedDepth = -2;
            size_t releasedOrder = 0;
            bool releasedThisFrame = false;
            auto inspectLock = [&](const std::string& id, const auto& object) {
                if (interactionTarget != id) return;
                lockFound = true;
                lockVisited = visited(object);
                lockOverlaps = overlap(object.x, object.y, object.width, object.height);
                releasedDepth = object.depth; releasedOrder = object.order;
            };
            for (const auto& key : scene.Keys()) inspectLock("K:" + key.keyId, key);
            for (const auto& paper : scene.Papers()) inspectLock("P:" + paper.descriptionId, paper);
            for (const auto& devil : scene.HealthDevils()) inspectLock("V:" + currentLevel, devil);
            for (const auto& scroll : scene.Scrolls()) inspectLock("S:" + scroll.id, scroll);
            for (const auto& door : scene.Doors()) inspectLock("D:" + door.id + door.destination, door);
            for (const auto& boat : scene.Boats()) inspectLock("B:" + boat.id, boat);
            if (interactionTarget == "H:" + currentLevel && healthPickup) {
                lockFound = true;
                lockVisited = pickupUpdatedThisFrame;
                releasedDepth = 3000; releasedOrder = std::numeric_limits<size_t>::max();
                lockOverlaps = std::abs(worldX-pickupInteractionX) < (128.959793f+50.f)*0.5f &&
                    std::abs(worldY-pickupInteractionY) < (214.898743f+70.f)*0.5f;
            }
            const bool frozenInteractable = !interactionTarget.empty() && lockFound && !lockVisited;
            if (!interactionTarget.empty() && (!lockFound || (lockVisited && !lockOverlaps))) {
                releasedThisFrame = lockFound && lockVisited;
                interactionTarget.clear(); interactHeldTime = 0.f;
            }
            // AddGameObject re-sorts the live list by descending depth. A
            // spawned pickup defaults to kPlayer (3000) and follows authored
            // objects on the same layer; a visited old target keeps its lock.
            std::string target;
            int selectedDepth = -2;
            size_t selectedOrder = 0;
            auto select = [&](const std::string& id, int depth, size_t order) {
                if (frozenInteractable || (!interactionTarget.empty() && id != interactionTarget)) return;
                // Objects earlier in the live manager list were already visited
                // while the old lock was held; only later objects can acquire
                // a lock after its owner releases it in this same update pass.
                if (releasedThisFrame && (depth > releasedDepth ||
                    (depth == releasedDepth && order <= releasedOrder))) return;
                const bool held = !interactionTarget.empty() && id == interactionTarget;
                const bool selectedHeld = !interactionTarget.empty() && target == interactionTarget;
                if (!held && (selectedHeld || (depth < selectedDepth) ||
                    (depth == selectedDepth && order >= selectedOrder))) return;
                target = id; selectedDepth = depth; selectedOrder = order;
            };
            const bool nearbyHealthPickup = healthPickup && pickupUpdatedThisFrame &&
                std::abs(worldX-pickupInteractionX) < (128.959793f+50.f)*0.5f &&
                std::abs(worldY-pickupInteractionY) < (214.898743f+70.f)*0.5f;
            // A culled authored object does not run UpdateInteractable: no
            // prompt, lock acquisition, or held countdown on that frame.
            // The spawned kPlayer pickup sorts ahead of lower-depth authored
            // interactables, but after earlier-authored kPlayer peers.
            if (nearbyHealthPickup) select("H:" + currentLevel, 3000,
                                           std::numeric_limits<size_t>::max());
            const deathleak::game::KeyPickup* nearbyKey = nullptr;
            const deathleak::game::PaperPickup* nearbyPaper = nullptr;
            const deathleak::game::HealthDevil* nearbyDevil = nullptr;
            const deathleak::game::DojoScroll* nearbyScroll = nullptr;
            const deathleak::game::Door* nearbyDoor = nullptr;
            const deathleak::game::Boat* nearbyBoat = nullptr;
            for (const auto& key : scene.Keys()) if (!ownedKeys.count(key.keyId) && visited(key) && overlap(key.x,key.y,key.width,key.height)) {
                select("K:" + key.keyId, key.depth, key.order);
                if (target == "K:" + key.keyId) nearbyKey = &key;
            }
            for (const auto& paper : scene.Papers()) if (!collectedPapers.count(paper.descriptionId) && visited(paper) && overlap(paper.x,paper.y,paper.width,paper.height)) {
                select("P:" + paper.descriptionId, paper.depth, paper.order);
                if (target == "P:" + paper.descriptionId) nearbyPaper = &paper;
            }
            if (!devilDialog && motion.Grounded() && !rewardedHealthDevils.count(currentLevel))
                for (const auto& devil : scene.HealthDevils()) if (visited(devil) && overlap(devil.x,devil.y,devil.width,devil.height)) {
                    select("V:" + currentLevel, devil.depth, devil.order);
                    if (target == "V:" + currentLevel) nearbyDevil = &devil;
                }
            if (!combat.DeflectionUnlocked())
                for (const auto& scroll : scene.Scrolls()) if (scroll.feature == "kDeflection" && visited(scroll) && overlap(scroll.x,scroll.y,scroll.width,scroll.height)) {
                    select("S:" + scroll.id, scroll.depth, scroll.order);
                    if (target == "S:" + scroll.id) nearbyScroll = &scroll;
                }
            if (levelTime >= 0.75f && motion.Grounded()) {
                for (const auto& door : scene.Doors()) if (visited(door) && overlap(door.x,door.y,door.width,door.height)) {
                    select("D:" + door.id + door.destination, door.depth, door.order);
                    if (target == "D:" + door.id + door.destination) nearbyDoor = &door;
                }
                for (const auto& boat : scene.Boats()) if (visited(boat) && overlap(boat.x,boat.y,boat.width,boat.height)) {
                    select("B:" + boat.id, boat.depth, boat.order);
                    if (target == "B:" + boat.id) nearbyBoat = &boat;
                }
            }
            // Only the selected object receives this frame's interaction hold.
            const bool selectedHealthPickup = nearbyHealthPickup && target == "H:" + currentLevel;
            if (target.empty() || target[0] != 'K') nearbyKey = nullptr;
            if (target.empty() || target[0] != 'P') nearbyPaper = nullptr;
            if (target.empty() || target[0] != 'V') nearbyDevil = nullptr;
            if (target.empty() || target[0] != 'S') nearbyScroll = nullptr;
            if (target.empty() || target[0] != 'D') nearbyDoor = nullptr;
            if (target.empty() || target[0] != 'B') nearbyBoat = nullptr;
            if (!frozenInteractable && (interactionTarget.empty() || !target.empty())) {
                if (!interact || target != interactionTarget) interactHeldTime = 0;
                interactionTarget = target;
                if (interact && !target.empty()) interactHeldTime += dt;
            }
            if (interactHeldTime >= 0.05f && selectedHealthPickup) {
                savedMaxHealth = combat.MaxHealth() + 20.f;
                hasSavedMaxHealth = true;
                combat.SetMaxHealth(savedMaxHealth);
                if (sound) sound->Play(deathleak::audio::Cue::HealthUpgrade);
                std::cout << "Health upgrade collected " << currentLevel
                          << " at frame " << frame << " max_health=" << savedMaxHealth << '\n';
                healthPickup.reset();
                interactHeldTime = 0; interactionTarget.clear();
            } else if (interactHeldTime >= 0.05f && nearbyKey) {
                ownedKeys.insert(nearbyKey->keyId);
                scene.ApplyKeys(ownedKeys);
                std::cout << "Collected key " << nearbyKey->keyId << " at frame " << frame << '\n';
                interactHeldTime = 0; interactionTarget.clear();
            } else if (interactHeldTime >= 0.05f && nearbyPaper) {
                collectedPapers.insert(nearbyPaper->descriptionId);
                scene.ApplyPapers(collectedPapers);
                paperTitle = paperText.Lookup(nearbyPaper->titleId, language);
                paperDescription = paperText.Lookup(nearbyPaper->descriptionId, language);
                paperModal = true; modalWarmup = 0.f; dismissHeld = false;
                if (sound) sound->Play(deathleak::audio::Cue::PaperPickup);
                std::cout << "Opened paper modal title=" << std::quoted(paperTitle)
                          << " description=" << std::quoted(paperDescription)
                          << " locale=" << (language.empty() ? "en-gb" : language)
                          << " at frame " << frame << '\n';
                std::cout << "Collected paper " << nearbyPaper->descriptionId << " at frame " << frame << '\n';
                interactHeldTime = 0; interactionTarget.clear();
            } else if (interactHeldTime >= 0.05f && nearbyDevil) {
                const bool third = healthDevilRewardCount >= 2;
                const unsigned voice = sound ? sound->PlayDevilVoice(third) : 0;
                devilDialog = DevilDialog{currentLevel, nearbyDevil->x, nearbyDevil->y,
                    nearbyDevil->width, nearbyDevil->height,
                    0.f, third ? 11.842177f : 12.448481f, voice, nearbyDevil->depth,
                    nearbyDevil->updateable, nearbyDevil->alwaysUpdate};
                std::cout << "Health devil dialog " << currentLevel << " at frame " << frame
                          << " voice=" << (third ? 3 : 1) << '\n';
                interactHeldTime = 0; interactionTarget.clear();
            } else if (interactHeldTime >= 0.5f && nearbyScroll) {
                upgradeModal = true; modalWarmup = 0; purchaseHeld = dismissHeld = false;
                interactHeldTime = 0; interactionTarget.clear();
                std::cout << "Opened kDeflection upgrade modal at frame " << frame << " cost=375 balance=" << orbBalance << '\n';
            } else if (interactHeldTime >= (unlockingDoor ? 0.4f : 0.05f) && nearbyDoor && doorUnlockWait <= 0) {
                const auto door = *nearbyDoor;
                interactHeldTime = 0;
                if (!door.requiredKey.empty() && !ownedKeys.count(door.requiredKey)) {
                    std::cout << "Door locked: requires " << door.requiredKey << " at frame " << frame << '\n';
                    doorUnlockWait = 1.f; // Door::OnInteracted locked retry countdown
                    unlockingDoor = false;
                } else if (!door.requiredKey.empty() && !unlockedDoors.count(door.id)) {
                    unlockedDoors.insert(door.id);
                    doorUnlockWait = 1.f; // Door::OnInteracted's unlocking animation
                    unlockingDoor = true;
                    std::cout << "Unlocked door " << door.id << " at frame " << frame << '\n';
                } else {
                    deathleak::game::LevelScene nextScene;
                    if (nextScene.Load(door.destination, *renderer, error, currentLevel, door.id)) {
                        nextScene.ApplyCollected(collectedOrbs);
                        nextScene.ApplyKeys(ownedKeys);
                        nextScene.ApplyPapers(collectedPapers);
                        nextScene.ApplyHealthDevil(rewardedHealthDevils.count(door.destination) != 0);
                        nextScene.ApplySmashable(brokenSmashables.count(door.destination) != 0);
                        nextScene.ApplyDeflection(combat.DeflectionUnlocked());
                        scene = std::move(nextScene);
                        cameraTarget = {scene.CameraOffsetX(), scene.CameraOffsetY()};
                        devilDialog.reset(); healthPickup.reset();
                        std::cout << "Door at frame " << frame << " from " << currentLevel << " to " << door.destination << '\n';
                        currentLevel = door.destination; introPromptClock.Reset();
                        motion.Reset(scene.PlayerX(),scene.PlayerY(), true);
                        worldX = motion.X(); worldY = motion.Y();
                        cameraX = scene.CameraX(); cameraY = scene.CameraY(); cameraShake.Reset();
                        combat.ClearLevel(); levelTime = interactHeldTime = doorUnlockWait = 0; unlockingDoor = false; interactionTarget.clear();
                    } else std::cerr << "Door transition failed: " << error << '\n';
                    continue;
                }
            }
            // Boat::CanInteract requires solid ground and a 0.75 s warmup.
            // Its source gate is the permanent has_repair_tools key, not a door ID.
            // With tools, Boat's hold duration is zero; without, it is 0.05 s.
            if (nearbyBoat && interact &&
                interactHeldTime >= (ownedKeys.count("has_repair_tools") ? 0.f : 0.05f)) {
                if (!ownedKeys.count("has_repair_tools")) {
                    std::cout << "Boat broken: requires has_repair_tools at frame " << frame << '\n';
                    interactHeldTime = 0; interactionTarget.clear();
                } else {
                    const std::string destination = "Game/XmlFiles/levels/sea_2.xml";
                    deathleak::game::LevelScene nextScene;
                    if (nextScene.Load(destination, *renderer, error, currentLevel)) {
                        nextScene.ApplyCollected(collectedOrbs);
                        nextScene.ApplyKeys(ownedKeys);
                        nextScene.ApplyPapers(collectedPapers);
                        nextScene.ApplyHealthDevil(rewardedHealthDevils.count(destination) != 0);
                        nextScene.ApplySmashable(brokenSmashables.count(destination) != 0);
                        nextScene.ApplyDeflection(combat.DeflectionUnlocked());
                        scene = std::move(nextScene);
                        cameraTarget = {scene.CameraOffsetX(), scene.CameraOffsetY()};
                        devilDialog.reset(); healthPickup.reset();
                        std::cout << "Boat at frame " << frame << " from " << currentLevel << " to " << destination << '\n';
                        currentLevel = destination; introPromptClock.Reset();
                        if (!simulateOnly) filmFrames.Prepare(*renderer, error);
                        motion.Reset(scene.PlayerX(),scene.PlayerY(), true);
                        worldX = motion.X(); worldY = motion.Y();
                        cameraX = scene.CameraX(); cameraY = scene.CameraY(); cameraShake.Reset();
                        combat.ClearLevel(); levelTime = interactHeldTime = doorUnlockWait = 0; unlockingDoor = false; interactionTarget.clear();
                    } else std::cerr << "Boat transition failed: " << error << '\n';
                    continue;
                }
            }
            for (const auto& trigger : scene.Triggers()) {
                // Original LevelTrigger uses Utilities::IsObjectInRectangle, which
                // tests the Player sprite dimensions, not its narrower solid
                // collision box or just its origin. This matters at the forts
                // west boundary, where the sprite overlaps the barn trigger.
                if (std::abs(worldX - trigger.x) < (trigger.width + scene.PlayerVisualWidth()) * 0.5f &&
                    std::abs(worldY - trigger.y) < (trigger.height + scene.PlayerVisualHeight()) * 0.5f &&
                    trigger.destination != currentLevel) {
                    // Loading replaces scene.Triggers(); never retain a reference into it.
                    const std::string destination = trigger.destination;
                    std::cout << "Trigger at frame " << frame << " from " << currentLevel << " player=(" << worldX << "," << worldY << ") to " << destination << std::endl;
                    if (currentLevel == "Game/XmlFiles/levels/forest_exploration_1.xml" &&
                        destination == "Game/XmlFiles/levels/waterfall_top.xml") {
                        loadingDestination = destination;
                        loadingPhase = LoadingPhase::FadeIn;
                        loadingPhaseTime = 0.f;
                    } else {
                        deathleak::game::LevelScene nextScene;
                        if (nextScene.Load(destination, *renderer, error, currentLevel)) {
                            nextScene.ApplyCollected(collectedOrbs);
                            nextScene.ApplyKeys(ownedKeys);
                            nextScene.ApplyPapers(collectedPapers);
                            nextScene.ApplyHealthDevil(rewardedHealthDevils.count(destination) != 0);
                            nextScene.ApplySmashable(brokenSmashables.count(destination) != 0);
                            nextScene.ApplyDeflection(combat.DeflectionUnlocked());
                            scene = std::move(nextScene);
                            cameraTarget = {scene.CameraOffsetX(), scene.CameraOffsetY()};
                            devilDialog.reset(); healthPickup.reset();
                            currentLevel = destination; introPromptClock.Reset();
                            worldX = scene.PlayerX(); worldY = scene.PlayerY();
                            // Original GameObjectManager::Initialise snaps followed axes
                            // to the LevelEntry-positioned player before the first draw.
                            cameraX = scene.CameraFollowX() ? worldX : scene.CameraX();
                            cameraY = scene.CameraFollowY() ? worldY : scene.CameraY();
                            cameraShake.Reset();
                            motion.Reset(worldX, worldY, true); combat.ClearLevel();
                            levelTime = interactHeldTime = doorUnlockWait = 0; unlockingDoor = false; interactionTarget.clear();
                            std::cout << "Entered " << currentLevel << std::endl;
                        } else std::cerr << "Transition failed: " << error << '\n';
                    }
                    break;
                }
            }
            }
            }
            }
        }
        if (storyEnded) {
            completionOccurred = true;
            if (!menuLoop) break;
            if (!persistMenuMetadata()) return 1;
            std::cout << "Main menu return after story completion count=" << completions << " language=" << language << '\n';
            playLevel.clear(); levelPath.clear(); currentLevel.clear();
            collectedOrbs.clear(); ownedKeys.clear(); unlockedDoors.clear(); collectedPapers.clear(); rewardedHealthDevils.clear(); brokenSmashables.clear(); healthDevilRewardCount = 0;
            devilDialog.reset(); healthPickup.reset();
            orbBalance = 0; scene = deathleak::game::LevelScene{};
            cameraTarget = {scene.CameraOffsetX(), scene.CameraOffsetY()};
            weather = deathleak::game::WeatherLightning{};
            rain = deathleak::game::WeatherRain{};
            combat = deathleak::game::Combat{};
            speedUnlocked = projectileDamageUnlocked = projectileSpeedUnlocked = false; downwardDashUnlocked = rollUnlocked = false; savedMaxHealth = 25; hasSavedMaxHealth = false; windowsPermanentExtension.clear();
            motion.SetSpeedUpgrade(false); motion.SetMovementUnlocks(false, false);
            finalSceneTime = 0; finalTitleShown = storyEnded = false;
            introPromptClock.Reset();
            menuSelection = 0; menuOptions = menuLanguages = false;
        }
        auto afterUpdate = std::chrono::steady_clock::now();
        updateSeconds += std::chrono::duration<double>(afterUpdate-now).count();
        if (!playLevel.empty()) {
            weather.SetAllowed(scene.AllowsWeather());
            if (!paperModal && !upgradeModal && !loadingPausedThisFrame) {
                packetWeatherRan = true;
#ifdef DEATHLEAK_TEST_LOADING_FADE_OBSERVER
                loadingWeatherRan = true;
#endif
                weather.Update(dt);
                rain.Update(dt, weather.Raining(), cameraX, cameraY);
            }
        }
        if (!simulateOnly && !playLevel.empty())
            weatherTextures.Prepare(*renderer, rain, rain.VisibleParticles(), weather.Alpha() > 0.f);
        if (simulateOnly) {
            // Keep the intro terminal signal advancing in deterministic CPU-only
            // simulations; ordinary gameplay visuals remain skipped.
            if (!hasPulledSword) scene.UpdatePlayerVisual(dt, 0.f, 0.f, motion.Grounded(),
                motion.WallSide(), false, false, motion.InWater(), 0.f, facing,
                false, false, {}, false, 0.f, 0.f, precedingRenderedModifiedDelta);
            precedingRenderedModifiedDelta = dt;
            continue;
        }
        const bool objectsPaused = paperModal || upgradeModal || loadingPausedThisFrame;
        if (!loadingPausedThisFrame) {
            packetSceneRan = true;
#ifdef DEATHLEAK_TEST_LOADING_FADE_OBSERVER
            loadingSceneVisualsRan = true;
#endif
            scene.UpdateVisuals(dt, worldX, worldY, cameraX, cameraY, objectsPaused, precedingRenderedModifiedDelta);
        }
        if (!objectsPaused) combat.UpdateVisuals(dt);
#ifdef DEATHLEAK_TEST_BODY_SLOW_FRAME_COMMAND
        if (!playLevel.empty() && !objectsPaused) bodySlowFrameObserver->BodyUpdate(motion.Grounded());
#endif
        if (!playLevel.empty() && !objectsPaused) {
            packetPlayerRan = true;
#ifdef DEATHLEAK_TEST_LOADING_FADE_OBSERVER
            loadingPlayerVisualRan = true;
#endif
            scene.UpdatePlayerVisual(dt, motion.VelocityX(), motion.VelocityY(),
            motion.Grounded(), motion.WallSide(), motion.Rolling(), motion.DownwardDashing(), motion.InWater(),
            combat.MeleeElapsed(), facing, playerProjectileFired, playerEligibleFireAttempt, strafeVisual, stickAim, aimX, aimY, precedingRenderedModifiedDelta, crouchInput, motion.Recovery());
        }
        if (!playLevel.empty() && !objectsPaused && scene.ImpactVisualFinished()) motion.CompleteImpactRecovery();
        // Ensure film art is resident before the update/presentation split;
        // the ordinary gameplay draw below only submits already prepared IDs.
        if (!simulateOnly && !levelPath.empty()) filmFrames.Prepare(*renderer, error);
        // Prepare the authored 11-FPS film once per fixed update. Draw below
        // only reads the prepared index, including on a future update-free present.
        if (!levelPath.empty()) {
            const auto filmNowMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
            auto& selectedFilm = currentLevel == "Game/XmlFiles/levels/sea_2.xml" ? finalFilm : gameplayFilm;
            selectedFilm.Prepare(static_cast<std::uint64_t>(filmNowMilliseconds), filmFrames.Size());
        }
        // Prepare camera shake on each fixed update. X then Y consume the original
        // conditional rand() signs; presentation only reads the prepared view.
        const bool jitter = cameraShake.WillJitter(cameraShakeTime - dt);
        const int shakeSignX = jitter && scene.CameraFollowX() ? ((std::rand() & 1) ? 1 : -1) : 1;
        const int shakeSignY = jitter && scene.CameraFollowY() ? ((std::rand() & 1) ? 1 : -1) : 1;
        cameraShake.Prepare(cameraShakeTime - dt, 1.f, scene.CameraFollowX(), scene.CameraFollowY(),
            shakeSignX, shakeSignY, cameraX, cameraY);
        // Advance the visual/shader clocks for every simulation step, including
        // intermediate catch-up steps. The display retains its last image until
        // the final state in this batch is ready to present.
        shaderClock.Prepare(); // One wobble step per update, including catch-up steps.
        if (!playLevel.empty()) {
            const bool rawInteractHeld = !script.empty() ? command == 'I' : renderer->KeyDown(Interact);
            const bool introLooped = !hasPulledSword &&
                currentLevel == "Game/XmlFiles/levels/forest_exploration_1.xml" &&
                scene.GetIntroBodyPhase() == deathleak::game::LevelScene::IntroBodyPhase::Looped;
            introPromptPresentation.Prepare(introLooped, rawInteractHeld,
                suppressIntroPromptDraw, introPromptArt != 0, introPromptClock);
        }
        // Snapshot only the HUD branch after the fixed update. The scene painter,
        // shader, film, and capture stages retain their existing order.
        std::optional<deathleak::game::HudPresentationPacket> hudPacket;
        if (!playLevel.empty() && currentLevel != "Game/XmlFiles/levels/sea_2.xml") {
            hud.Prepare(combat.Health(), combat.MaxHealth(), combat.Focus(),
                        shaderClock.Draw().totalTime - dt);
            hudPacket.emplace(hud, orbBalance);
        }
        const auto loadingPacket = deathleak::game::LoadingOverlayPacket::Prepare(
            loadingPhase != LoadingPhase::None, loadingHoldExitedThisFrame,
            loadingPhase == LoadingPhase::FadeIn, loadingPhase == LoadingPhase::FadeOut,
            loadingPhaseTime, kLoadingFadeSeconds);
        if (fixedDt <= 0.f && !livePacer.PresentNow()) {
            precedingRenderedModifiedDelta = dt;
            shaderClock.RenderCompleted(dt);
            continue;
        }
        if (!levelPath.empty()) scene.PreparePresentation();
        const auto shakeView = cameraShake.PresentationSample();
        const float drawCameraX = shakeView.viewX, drawCameraY = shakeView.viewY;
        const auto shaderFrame = shaderClock.Draw();
        renderer->BeginFrame({0, 0, 0, 255});
        if (levelPath.empty()) {
            renderer->DrawSprite({background, {}, {0, 0, 1280, 720}});
            if (menuLoop) {
                if (menuBlur) renderer->DrawSprite({menuBlur, {}, AuthoredUI(-990,-540,1969.2f,1115.8f), {255,255,255,115}});
                if (!menuOptions && !menuLanguages) {
                    if (titleBlur) renderer->DrawSprite({titleBlur, {}, AuthoredUI(-750,300,753,194), {255,255,255,128}});
                    if (menuTitle) renderer->DrawSprite({menuTitle, {}, AuthoredUI(-754,307,753,194)});
                }
                if (menuFrame) renderer->DrawSprite({menuFrame, {}, {0, 0, 1280, 720}});
                const int filmIndex = static_cast<int>(menuFilmTime * 11.f) % 4;
                if (menuFilm[filmIndex]) renderer->DrawSprite({menuFilm[filmIndex], {}, {0,0,1280,720}, {255,255,255,26}});
                if (menuLanguages) {
                    constexpr std::array<const char*, 4> labels = {"ENGLISH", "PORTUGUESE", "DEUTSCH", "BACK"};
                    for (int i = 0; i < 4; ++i)
                        deathleak::game::DrawBitmapText(*renderer, labels[i], 400, 270 + i*65, 4,
                            i == menuSelection ? Color{255,0,0,255} : Color{255,255,255,255});
                } else if (menuOptions) {
                    for (int i = 0; i < 2; ++i) {
                        const bool checked = i == 0 ? settings.music : settings.sfx;
                        const bool focused = (mouseHover >= 0 ? mouseHover : menuSelection) == i;
                        auto art = checked ? (focused ? optionsOnFocus[i] : optionsOn[i]) :
                            (focused ? optionsOffFocus[i] : optionsOff[i]);
                        if (art) renderer->DrawSprite({art, {}, optionsBounds[i]});
                    }
                    const bool backFocused = (mouseHover >= 0 ? mouseHover : menuSelection) == 3;
                    auto backArt = backFocused ? menuFocus[2] : menuButtons[2];
                    if (backArt) renderer->DrawSprite({backArt, {}, optionsBounds[2]});
                    deathleak::game::DrawBitmapText(*renderer, "LANGUAGE", 500, 555, 3,
                        (mouseHover >= 0 ? mouseHover : menuSelection) == 2 ? Color{255,0,0,255} : Color{255,255,255,255});
                } else {
                    for (int i = 0; i < 3; ++i) {
                        auto art = i == (mouseHover >= 0 ? mouseHover : menuSelection) ? menuFocus[i] : menuButtons[i];
                        if (art) renderer->DrawSprite({art, {}, menuBounds[i]});
                    }
                    deathleak::game::DrawBitmapText(*renderer, "PLAY", 480, 350, 4);
                    deathleak::game::DrawBitmapText(*renderer, "OPTIONS", 480, 492, 4);
                    deathleak::game::DrawBitmapText(*renderer, "QUIT", 480, 603, 4);
                    deathleak::game::DrawBitmapText(*renderer, "COMPLETED: " + std::to_string(completions), 750, 570, 3);
                }
            } else renderer->DrawSprite({player, {}, {playerX, playerY, 100, 170}});
        } else if (playLevel.empty())
            scene.Draw(*renderer, scene.CameraX() + (playerX - 590) * 1.5f,
                       scene.CameraY() + (430 - playerY) * 1.5f, scene.PlayerX(), scene.PlayerY(), {}, 1.f, 1.f, shaderFrame);
        else {
            // Only a genuinely fresh forest-1 intro changes the world projection.
            renderer->SetWorldZoom(!hasPulledSword ? introEffectiveZoom : 1.f);
            std::vector<deathleak::game::PainterItem> painterItems;
            combat.AppendPainterItems(painterItems, *renderer, drawCameraX, drawCameraY,
                fullFrameTopologyCheck ? std::function<void(const deathleak::game::ParticleSpray&, const deathleak::game::ParticleSpray::Particle&, const Sprite&)>(
                    [&](const auto& spray, const auto& particle, const Sprite&) {
                        auto key=deathleak::linux_port::EmittedParticleKey(spray,particle);
                        key.object |= (std::uint64_t{1}<<60); // Runtime material-spray namespace.
                        repeatRecorder->TagLastSprite(key);
                    }) : std::function<void(const deathleak::game::ParticleSpray&, const deathleak::game::ParticleSpray::Particle&, const Sprite&)>(),
                fullFrameTopologyCheck ? std::function<void(const deathleak::game::Shot&, const Sprite&)>(
                    [&](const auto& shot, const Sprite&) {
                        repeatRecorder->TagLastSprite({(std::uint64_t{1}<<59) | shot.stableIdentity,0});
                    }) : std::function<void(const deathleak::game::Shot&, const Sprite&)>());
            // Source Player::Draw places the X immediately after the player body.
            // A player-adjacent slot preserves same-depth object visitation order.
            std::function<void()> afterPlayerDraw;
            if (introPromptPresentation.Visible()) {
                constexpr float scale = 2.f / 3.f;
                const float width = steamDeck ? 60.f : 150.f;
                const float height = steamDeck ? 60.f : 90.f;
                const Sprite prompt{introPromptArt, {},
                    {640.f + (worldX - 35.f - drawCameraX - width / 2.f) * scale,
                     360.f + (drawCameraY - worldY - 50.f - height / 2.f) * scale,
                     width * scale, height * scale}};
                afterPlayerDraw = [&, prompt] { renderer->DrawSprite(prompt); };
            }
            if (healthPickup && healthPickupArt) {
                constexpr float scale = 2.f / 3.f;
                const auto sprite = deathleak::render::Sprite{healthPickupArt, {},
                    {640.f + (healthPickup->x-drawCameraX-25.f)*scale,
                     360.f + (drawCameraY-healthPickup->y-35.f)*scale,
                     50.f*scale, 70.f*scale}};
                // HealthUpgradePickup inherits SolidMovingSprite's kPlayer depth.
                painterItems.push_back({3000, [&, sprite] { renderer->DrawSprite(sprite); }});
            }
            if (weather.Alpha() > 0.f && weatherTextures.Lightning()) {
                const Sprite flash = weather.SpriteForCamera(weatherTextures.Lightning(), drawCameraX, drawCameraY);
                painterItems.push_back({20, [&, flash] { renderer->DrawSprite(flash); }});
            }
            if (rain.VisibleParticles())
                painterItems.push_back({20, [&] { rain.Draw(*renderer, drawCameraX, drawCameraY); }});
            const bool packetTargetDraw = packetTrace && frame == frameLimit - 1;
            if (packetTargetDraw) packetTrace->StartScene();
            scene.Draw(*renderer, drawCameraX, drawCameraY, worldX, worldY, std::move(painterItems),
                       combat.PlayerDamageBurstGain(), weather.FoliageSwayMultiplier(), shaderFrame, suppressLandingSprayDraw,
                       std::move(afterPlayerDraw), {},
                       fullFrameTopologyCheck ? std::function<void(const deathleak::game::SceneSprite&, const Sprite&, bool)>(
                           [&](const deathleak::game::SceneSprite& source, const Sprite&, bool arm) {
                               using Packet = deathleak::linux_port::PresentationCommandPacket;
                               const auto phase = source.parallaxLayer ?
                                   (source.autoScrollY ? Packet::PhasePolicy::AutoScrollY : Packet::PhasePolicy::CameraParallax) :
                                   (source.foliageSway.enabled || source.pixelWobble.enabled) ? Packet::PhasePolicy::ShaderClock :
                                   Packet::PhasePolicy::Static;
                               repeatRecorder->TagLastSprite({(source.stableIdentity << 1) | unsigned(arm),0},
                                   source.parallaxLayer ? 0.f : deathleak::linux_port::OrdinaryCameraFactorX(source.depth),
                                   source.parallaxLayer ? 0.f : deathleak::linux_port::OrdinaryCameraFactorY(source.depth),
                                   phase, source.cameraParallaxX * (source.flipX ? -1.f : 1.f),source.cameraParallaxY,source.autoScrollYStep,
                                   source.scrolling || source.fadeWhenPlayerOccluded || source.playerAimLine,
                                   source.playerAimLine);
                           }) : std::function<void(const deathleak::game::SceneSprite&, const Sprite&, bool)>(),
                       fullFrameTopologyCheck ? std::function<void(const deathleak::game::ParticleSpray&, const deathleak::game::ParticleSpray::Particle&, const Sprite&)>(
                           [&](const deathleak::game::ParticleSpray& spray,const deathleak::game::ParticleSpray::Particle& particle,const Sprite&) {
                               auto key=deathleak::linux_port::EmittedParticleKey(spray,particle);
                               key.object |= (std::uint64_t{1}<<62);
                               repeatRecorder->TagLastSprite(key);
                           }) : std::function<void(const deathleak::game::ParticleSpray&, const deathleak::game::ParticleSpray::Particle&, const Sprite&)>());
            if (packetTargetDraw) packetTrace->StopScene();
            renderer->ApplyScenePostprocess();
            // Windows Game::PostDraw completes before UIManager::Draw. Read back
            // this exact post-sepia/pre-HUD surface on the packet target draw.
            if (packetTargetDraw && !atomicId22Prefix.empty() && !packetTrace->Id22Captured()) {
                std::cerr << "ID22 atomic capture failed: " << packetTrace->Id22Error() << " ordinal=" << packetTrace->Id22Ordinal() << '\n'; return 1;
            }
            if (packetTargetDraw && !renderer->SaveFrame(capture + ".prehud.bmp", error)) {
                std::cerr << "Pre-HUD capture failed: " << error << '\n'; return 1;
            }
            // ShaderClock advanced at loop start; the original meter samples total
            // before this frame's modified delta is committed.
            if (currentLevel == "Game/XmlFiles/levels/sea_2.xml") {
                // final_scene_hud: four authored film frames at 11 FPS and
                // quarter opacity, plus the original frame_top art. Bitmap
                // lettering remains a typography fallback for Impact.
                if (filmFrames.Frame(finalFilm.Index())) renderer->DrawSprite({filmFrames.Frame(finalFilm.Index()), {}, {0,0,1280,720}, {255,255,255,64}});
                if (finalFrame) {
                    renderer->DrawSprite({finalFrame, {}, {0,0,1280,100}});
                    Sprite bottomBar{finalFrame, {}, {0,620,1280,100}};
                    bottomBar.flipY = true;
                    renderer->DrawSprite(bottomBar);
                }
                if (finalTitleShown) deathleak::game::DrawBitmapText(*renderer, "DEATH LEAK", 200, 255, 16, {255,0,0,245});
            } else {
                if (filmFrames.Frame(gameplayFilm.Index()))
                    { renderer->DrawSprite({filmFrames.Frame(gameplayFilm.Index()), {}, {0,0,1280,720}, {255,255,255,51}});
                    if (fullFrameTopologyCheck) repeatRecorder->TagLastSprite({(std::uint64_t{1}<<61)|1,0},0.f,0.f); }
                // game_hud.xml: opaque 50-unit bands follow the film and precede HUD widgets.
                if (finalFrame) {
                    renderer->DrawSprite({finalFrame, {}, AuthoredUI(-960, 490, 1920, 50), {255,255,255,255}});
                    if (fullFrameTopologyCheck) repeatRecorder->TagLastSprite({(std::uint64_t{1}<<61)|2,0},0.f,0.f);
                    Sprite bottomBar{finalFrame, {}, AuthoredUI(-960, -540, 1920, 50), {255,255,255,255}};
                    bottomBar.flipY = true;
                    renderer->DrawSprite(bottomBar);
                    if (fullFrameTopologyCheck) repeatRecorder->TagLastSprite({(std::uint64_t{1}<<61)|3,0},0.f,0.f);
                }
                const auto hudStart = fullFrameTopologyCheck ? repeatRecorder->Packet().Size() : 0;
                hudPacket->Draw(*renderer);
                if (fullFrameTopologyCheck) repeatRecorder->TagStaticRange(hudStart, (std::uint64_t{1}<<61)|0x1000);
            }
            if (paperModal)
                deathleak::game::DrawPaperModal(*renderer, paperTitle, paperDescription,
                    modalBackground, paperBackButton, modalWarmup >= 1.f);
            if (upgradeModal) {
                if (modalBackground) renderer->DrawSprite({modalBackground, {}, {0,0,1280,720}, {255,255,255,204}});
                if (modalOrb) renderer->DrawSprite({modalOrb, {}, {520,460,56,85}});
                if (modalButton) renderer->DrawSprite({modalButton, {}, {590,460,150,90}});
                deathleak::game::DrawBitmapText(*renderer, "NEW TECHNIQUE", 340, 120, 6);
                deathleak::game::DrawBitmapText(*renderer, "DEFLECTION", 400, 240, 7);
                deathleak::game::DrawBitmapText(*renderer, "COST 375 ORBS", 355, 360, 5);
                deathleak::game::DrawBitmapText(*renderer, "BALANCE: " + std::to_string(orbBalance), 380, 410, 5);
                deathleak::game::DrawBitmapText(*renderer, orbBalance >= 375 ? "Y BUY / ESC BACK" : "NOT ENOUGH ORBS / ESC BACK", 260, 565, 4);
            }
            if (combat.Health() <= 0 && deathArt)
                renderer->DrawSprite({deathArt, {}, {0, 0, 1280, 720}});
            // UIManager draws gameloading after scene postprocess and all HUD.
            loadingPacket.Draw(*renderer, modalBackground);
#ifdef DEATHLEAK_TEST_LOADING_FADE_OBSERVER
            if (loadingPacket.visible) loadingDrawAlpha = loadingPacket.tintAlpha;
#endif
        }
        // Debug overlay is the final UI layer, including during gameplay modals and fades.
        // It samples presented wall-clock frames, never the capped/fixed simulation delta.
        if (showFps) deathleak::game::DrawBitmapText(*renderer,
            displayedFps >= 0 ? "FPS " + std::to_string(displayedFps) : "FPS --",
            10.f, 693.f, 2.f, {255,255,255,230});
        // State-derived target: the first Waterfall draw after its loading UI retires.
        if (!atomicPostfadePacket.empty() && currentLevel == "Game/XmlFiles/levels/waterfall_top.xml") {
            if (loadingPhase != LoadingPhase::None) packetSawWaterfallLoading = true;
            if (packetSawWaterfallLoading && packetFirstPostfadeOrdinal < 0 &&
                loadingPhase == LoadingPhase::None && packetGameplayRan && packetWeatherRan &&
                packetSceneRan && packetPlayerRan) packetFirstPostfadeOrdinal = frame;
        }
        if (playLevel.empty()) renderer->ResolveUnprocessedFrame();
        const bool packetFrame415 = !atomicFrame415Packet.empty() && frame == 415;
        const bool packetPostfade = !atomicPostfadePacket.empty() && frame == frameLimit - 1;
        const std::string& frameCapture = (sameRunWaterfallPackets && packetFrame415) ? sameRunFrame415Capture : capture;
        if ((!capture.empty() && frame == frameLimit - 1 || sameRunWaterfallPackets && packetFrame415) &&
            !renderer->SaveFrame(frameCapture, error)) {
            std::cerr << "Capture failed: " << error << '\n'; return 1;
        }
        if (packetFrame415 || packetPostfade) {
            const bool postfadePacket = packetPostfade;
            const std::string& packetPath = postfadePacket ? atomicPostfadePacket : atomicFrame415Packet;
            // SaveFrame above is post-overlay/pre-EndFrame for this exact draw.
            // The JSON is committed only after the image and byte hashes exist.
            const bool firstReveal = loadingPhase == LoadingPhase::FadeOut && packetGameplayRan &&
                loadingPacket.alpha > 0.f && loadingPacket.alpha < 1.f;
            const bool firstPostfade = packetFirstPostfadeOrdinal == frame && packetSawWaterfallLoading &&
                loadingPhase == LoadingPhase::None && loadingPacket.alpha < 0.f &&
                loadingPacket.tintAlpha < 0 && packetGameplayRan && packetWeatherRan &&
                packetSceneRan && packetPlayerRan && !loadingPausedThisFrame;
            if (currentLevel != "Game/XmlFiles/levels/waterfall_top.xml" ||
                !(postfadePacket ? firstPostfade : firstReveal)) {
                std::cerr << "Atomic packet target is not requested Waterfall gameplay state; first postfade ordinal="
                          << packetFirstPostfadeOrdinal << '\n'; return 1;
            }
            std::array<unsigned char, 54> bmpHeader{};
            std::ifstream bmp(frameCapture, std::ios::binary);
            bmp.read(reinterpret_cast<char*>(bmpHeader.data()), bmpHeader.size());
            auto bmp32 = [&](size_t offset) {
                return static_cast<std::uint32_t>(bmpHeader[offset]) |
                    (static_cast<std::uint32_t>(bmpHeader[offset+1]) << 8) |
                    (static_cast<std::uint32_t>(bmpHeader[offset+2]) << 16) |
                    (static_cast<std::uint32_t>(bmpHeader[offset+3]) << 24);
            };
            if (!bmp || bmpHeader[0] != 'B' || bmpHeader[1] != 'M' ||
                bmp32(18) == 0 || bmp32(18) > 7680 || bmp32(22) == 0 || bmp32(22) > 4320 ||
                bmpHeader[26] != 1 || bmpHeader[27] != 0 ||
                bmpHeader[28] != 32 || bmpHeader[29] != 0) {
                std::cerr << "Atomic packet image header/dimensions invalid\n"; return 1;
            }
            const auto captureWidth = bmp32(18), captureHeight = bmp32(22);
            const auto imageHash = Sha256File(frameCapture);
            const std::string preHudPath = frameCapture + ".prehud.bmp";
            const auto preHudHash = postfadePacket ? Sha256File(preHudPath) : std::string{};
            // Hash the game process, not sha256sum's own /proc/self/exe after fork/exec.
            const auto exeHash = Sha256File("/proc/" + std::to_string(getpid()) + "/exe");
            const auto levelHash = Sha256File(currentLevel);
            std::error_code fileError;
            const auto imageBytes = std::filesystem::file_size(frameCapture, fileError);
            const auto preHudBytes = postfadePacket ? std::filesystem::file_size(preHudPath, fileError) : 0;
            if (imageHash.empty() || (postfadePacket && preHudHash.empty()) ||
                exeHash.empty() || levelHash.empty() || fileError) {
                std::cerr << "Atomic packet image/identity hash failed\n"; return 1;
            }
            const std::string temporary = packetPath + ".tmp";
            std::ofstream packet(temporary, std::ios::trunc);
            if (!packet) { std::cerr << "Atomic packet JSON open failed\n"; return 1; }
            packet << std::setprecision(9) << std::boolalpha;
            packet << "{\n  \"schema\": " << JsonString(postfadePacket ? "deathleak-linux-postfade-waterfall-v1" : "deathleak-linux-frame415-v1") << ",\n"
                   << "  \"matchedStateEligible\": false,\n"
                   << "  \"identity\": {\"engine\": \"linux_port\", \"executableSha256\": " << JsonString(exeHash)
                   << ", \"revision\": {\"value\": null, \"reason\": \"working-tree build; git revision not embedded\"}"
                   << ", \"levelXmlSha256\": " << JsonString(levelHash)
                   << ", \"contentManifestSha256\": {\"value\": null, \"reason\": \"manifest not instrumented\"}"
                   << ", \"inputRecipe\": {\"play\": " << JsonString(playLevel)
                   << ", \"script\": " << JsonString(script) << ", \"fixedDt\": " << fixedDt
                   << ", \"loadSave\": " << JsonString(loadSaveFile) << "}, \"renderOrdinal\": " << frame
                   << ", \"updateOrdinal\": " << frame << "},\n"
                   << "  \"image\": {\"file\": " << JsonString(frameCapture) << ", \"sha256\": " << JsonString(imageHash)
                   << ", \"bytes\": " << imageBytes << ", \"width\": " << captureWidth << ", \"height\": " << captureHeight
                   << ", \"pixelFormat\": \"BMP BGRA32\", \"colorSpace\": \"sRGB-assumed\""
                   << ", \"viewport\": [0,0," << captureWidth << "," << captureHeight << "], \"captureStage\": \"post_overlay_pre_endframe\""
                   << ", \"scenePostprocessSubmitted\": true},\n"
                   << (postfadePacket ? "  \"preHudImage\": " : "") << (postfadePacket ?
                       "{\"file\":" + JsonString(preHudPath) + ",\"sha256\":" + JsonString(preHudHash) +
                       ",\"bytes\":" + std::to_string(preHudBytes) +
                       ",\"width\":" + std::to_string(captureWidth) + ",\"height\":" + std::to_string(captureHeight) + ",\"pixelFormat\":\"BMP BGRA32\",\"captureStage\":\"post_scene_sepia_pre_hud\"}" :
                       "") << (postfadePacket ? ",\n" : "")
                   << "  \"transition\": {\"activeLevel\": " << JsonString(currentLevel);
            // Preserve the published frame-415 v1 shape; these fields belong to postfade packets only.
            if (postfadePacket)
                packet << ", \"firstPostfadeOrdinal\": " << packetFirstPostfadeOrdinal
                       << ", \"loadingSplashSubmitted\": " << (loadingPhase != LoadingPhase::None);
            packet << ", \"loadingPhaseOnEntry\": " << static_cast<int>(packetLoadingPhaseOnEntry)
                   << ", \"loadingPhaseAtDraw\": " << static_cast<int>(loadingPhase)
                   << ", \"loadingPhaseTime\": " << loadingPhaseTime
                   << ", \"pausedAtUpdateEntry\": " << loadingPausedThisFrame
                   << ", \"pausedAtDraw\": " << (loadingPhase == LoadingPhase::FadeIn || loadingPhase == LoadingPhase::OpaqueHold)
                   << ", \"loadingAlphaAtDraw\": " << loadingPacket.alpha
                   << ", \"loadingTintAlphaAtDraw\": " << loadingPacket.tintAlpha
                   << ", \"holdExitedThisFrame\": " << loadingHoldExitedThisFrame
                   << ", \"gameplayUpdated\": " << packetGameplayRan
                   << ", \"weatherUpdated\": " << packetWeatherRan
                   << ", \"sceneVisualsUpdated\": " << packetSceneRan
                   << ", \"playerVisualUpdated\": " << packetPlayerRan << "},\n"
                   << "  \"timingCameraPlayer\": {\"effectiveDt\": " << dt
                   << ", \"totalSimulationTime\": " << shaderFrame.totalTime
                   << ", \"precedingRenderedUpdateDt\": " << precedingRenderedModifiedDelta
                   << ", \"camera\": {\"storedX\": " << cameraX << ", \"storedY\": " << cameraY
                   << ", \"drawX\": " << drawCameraX << ", \"drawY\": " << drawCameraY
                   << ", \"zoom\": " << (!hasPulledSword ? introEffectiveZoom : 1.f)
                   << ", \"shakeActive\": " << shakeView.active << ", \"shakeRemaining\": " << shakeView.remaining
                   << "}, \"submittedMatrices\": {\"value\": null, \"reason\": \"renderer matrix submission not instrumented\"}"
                   << ", \"player\": {\"x\": " << worldX << ", \"y\": " << worldY
                   << ", \"velocityX\": " << motion.VelocityX() << ", \"velocityY\": " << motion.VelocityY()
                   << ", \"facing\": " << facing
                   << ", \"bodyArmClipFrame\": {\"value\": null, \"reason\": \"LevelScene animation submission not instrumented\"}}},\n"
                   << "  \"dynamicDraw\": {\"vertexNoiseTime\": " << shaderFrame.totalTime
                   << ", \"pixelWobbleTime\": " << shaderFrame.pixelWobbleTime
                   << ", \"pixelWobbleReversing\": " << shaderClock.Reversing()
                   << ", \"foliageGlobalMultiplier\": " << weather.FoliageSwayMultiplier()
                   << ", \"lightningAlpha\": " << weather.Alpha()
                   << ", \"rainVisible\": " << rain.VisibleParticles()
                   << ", \"orderedSceneDraws\": ";
            if (postfadePacket) {
                packet << "[";
                const auto& draws = packetTrace->Draws();
                for (size_t i = 0; i < draws.size(); ++i) {
                    const auto& d = draws[i];
                    if (i) packet << ",";
                    packet << "{\"ordinal\":" << i << ",\"textureId\":" << d.texture
                           << ",\"texturePath\":" << JsonString(d.path)
                           << ",\"source\":[" << d.source.x << "," << d.source.y << "," << d.source.width << "," << d.source.height << "]"
                           << ",\"destination\":[" << d.destination.x << "," << d.destination.y << "," << d.destination.width << "," << d.destination.height << "]"
                           << ",\"tint\":[" << static_cast<int>(d.tint.r) << "," << static_cast<int>(d.tint.g) << "," << static_cast<int>(d.tint.b) << "," << static_cast<int>(d.tint.a) << "]"
                           << ",\"opacity\":" << d.opacity << ",\"rotationDegrees\":" << d.rotation
                           << ",\"flipX\":" << d.flipX << ",\"flipY\":" << d.flipY
                           << ",\"repeatX\":" << d.repeatX << ",\"uvShift\":[" << d.uvShiftX << "," << d.uvShiftY << "]"
                           << ",\"foliageSway\":" << d.foliageSway << ",\"pixelWobble\":" << d.pixelWobble << "}";
                }
                packet << "]";
            } else packet << "{\"value\": null, \"reason\": \"per-object draw submission not instrumented\"}";
            if (postfadePacket) packet << ", \"sceneDrawIdentifierScope\": \"renderer-level sprite submissions; XML IDs and backend internal repeat/particle subdraws not mapped\"";
            packet
                   << ", \"rngHistory\": {\"value\": null, \"reason\": \"nondeterministic effect history not instrumented\"}},\n"
                   << "  \"uiOrder\": [\"film\",\"bars\",\"hud\",\"optional_fps\",\"modals\",\"loading_splash\"],\n"
                   << "  \"pixels\": {\"health\": " << combat.Health() << ", \"maxHealth\": " << combat.MaxHealth()
                   << ", \"focus\": " << combat.Focus() << ", \"orbs\": " << orbBalance
                   << ", \"fpsTextEnabled\": " << showFps
                   << ", \"displayedMeterFill\": {\"value\": null, \"reason\": \"HUD meter submission not instrumented\"}}\n}\n";
            packet.close();
            if (!packet) { std::cerr << "Atomic packet JSON write failed\n"; return 1; }
            std::filesystem::rename(temporary, packetPath, fileError);
            if (fileError) { std::cerr << "Atomic packet JSON commit failed: " << fileError.message() << '\n'; return 1; }
            std::cout << (postfadePacket ? "ATOMIC_POSTFADE_PACKET image=" : "ATOMIC_FRAME415_PACKET image=") << frameCapture << " json=" << packetPath
                      << " sha256=" << imageHash << "\n";
        }
        if (!atomicId22Prefix.empty() && frame == frameLimit - 1) {
            const auto& draws = packetTrace->Draws();
            const int ordinal = packetTrace->Id22Ordinal();
            if (currentLevel != "Game/XmlFiles/levels/waterfall_top.xml" ||
                loadingPhase != LoadingPhase::None || ordinal < 0 ||
                static_cast<size_t>(ordinal) >= draws.size()) {
                std::cerr << "ID22 atomic target state invalid\n"; return 1;
            }
            const auto& d = draws[ordinal];
            const std::string pre = atomicId22Prefix + ".pre.bmp";
            const std::string post = atomicId22Prefix + ".post.bmp";
            const auto preHash = Sha256File(pre), postHash = Sha256File(post);
            std::array<unsigned char, 26> id22Header{};
            std::ifstream id22Bmp(capture, std::ios::binary);
            id22Bmp.read(reinterpret_cast<char*>(id22Header.data()), id22Header.size());
            auto id22Dimension = [&](size_t at) {
                return static_cast<std::uint32_t>(id22Header[at]) |
                    (static_cast<std::uint32_t>(id22Header[at+1]) << 8) |
                    (static_cast<std::uint32_t>(id22Header[at+2]) << 16) |
                    (static_cast<std::uint32_t>(id22Header[at+3]) << 24);
            };
            const auto id22Width = id22Dimension(18), id22Height = id22Dimension(22);
            if (!id22Bmp || id22Header[0] != 'B' || id22Header[1] != 'M' ||
                id22Width == 0 || id22Width > 7680 || id22Height == 0 || id22Height > 4320) {
                std::cerr << "ID22 capture dimensions invalid\n"; return 1;
            }
            if (preHash.empty() || postHash.empty()) { std::cerr << "ID22 atomic hashes failed\n"; return 1; }
            std::ofstream packet(atomicId22Prefix + ".json.tmp", std::ios::trunc);
            if (!packet) { std::cerr << "ID22 atomic JSON open failed\n"; return 1; }
            packet << std::setprecision(9) << std::boolalpha
                   << "{\n  \"schema\": \"linux-waterfall-id22-same-render-v1\",\n"
                   << "  \"renderOrdinal\": " << frame << ", \"level\": " << JsonString(currentLevel)
                   << ", \"camera\": [" << drawCameraX << "," << drawCameraY
                   << "], \"player\": [" << worldX << "," << worldY << "],\n"
                   << "  \"sceneDrawOrdinal\": " << ordinal
                   << ", \"texturePath\": " << JsonString(d.path)
                   << ", \"textureSha256\": " << JsonString(Sha256File(d.path))
                   << ", \"source\": [" << d.source.x << "," << d.source.y << "," << d.source.width << "," << d.source.height
                   << "], \"destination\": [" << d.destination.x << "," << d.destination.y << "," << d.destination.width << "," << d.destination.height
                   << "], \"tint\": [" << int(d.tint.r) << "," << int(d.tint.g) << "," << int(d.tint.b) << "," << int(d.tint.a)
                   << "], \"opacity\": " << d.opacity << ", \"rotationDegrees\": " << d.rotation
                   << ", \"flipX\": " << d.flipX << ", \"flipY\": " << d.flipY
                   << ", \"repeatX\": " << d.repeatX << ", \"uvShift\": [" << d.uvShiftX << "," << d.uvShiftY
                   << "], \"foliageSway\": " << d.foliageSway << ", \"pixelWobble\": " << d.pixelWobble
                   << ",\n  \"pre\": {\"file\": " << JsonString(pre) << ", \"sha256\": " << JsonString(preHash)
                   << "}, \"post\": {\"file\": " << JsonString(post) << ", \"sha256\": " << JsonString(postHash)
                   << "}, \"final\": {\"file\": " << JsonString(capture) << ", \"sha256\": " << JsonString(Sha256File(capture))
                   << "}, \"captureStage\": \"scene_pre_and_post_draw_before_sepia\", \"width\": " << id22Width << ", \"height\": " << id22Height << "}\n";
            packet.close();
            if (!packet) { std::cerr << "ID22 atomic JSON write failed\n"; return 1; }
            std::filesystem::rename(atomicId22Prefix + ".json.tmp", atomicId22Prefix + ".json");
            std::cout << "ATOMIC_ID22_PACKET ordinal=" << ordinal << " pre=" << preHash << " post=" << postHash << '\n';
        }
        if (refreshSlots && !renderer->PresentationSurfaceStable()) {
            std::cerr << "Refresh surface unstable before ordinary present\n"; return 1;
        }
        renderer->EndFrame();
        if (fullFrameTopologyCheck && repeatRecorder) {
            deathleak::linux_port::FullFrameSnapshot frameSnapshot{
                repeatRecorder->Packet(),scene.SceneEpoch(),drawCameraX*(2.f/3.f),
                -drawCameraY*(2.f/3.f),false,shaderFrame.totalTime,shaderFrame.pixelWobbleTime};
            if (hasPriorFullFrame) {
                const auto decision=deathleak::linux_port::AdmitFullFrame(priorFullFrame,frameSnapshot,.5f);
                const auto mixed=deathleak::linux_port::AdmitMixedCommands(priorFullFrame,frameSnapshot,.5f);
                std::size_t projectileSampled=0,projectileHeld=0;
                for (std::size_t ci=0;ci<frameSnapshot.packet.Size();++ci) {
                    const auto& c=frameSnapshot.packet.Commands()[ci];
                    if (c.kind==deathleak::linux_port::PresentationCommandPacket::Kind::Sprite &&
                        (c.key.object >> 59)==1) {
                        if (ci<mixed.interpolate.size() && mixed.interpolate[ci]) ++projectileSampled;
                        else ++projectileHeld;
                    }
                }
                std::cout << "MIXED_COMMAND_TOPOLOGY frame=" << frame << " sampled=" << mixed.moving << " current=" << frameSnapshot.packet.Size()-mixed.moving << " projectileSampled=" << projectileSampled << " projectileHeld=" << projectileHeld << " valid=" << mixed.valid << '\n';
                if (frame == mixedHoldAuditFrame) {
                    const auto& aa=priorFullFrame.packet.Commands(); const auto& bb=frameSnapshot.packet.Commands();
                    for (std::size_t i=0;i<bb.size();++i) {
                        const auto& y=bb[i]; const auto* x=i<aa.size()?&aa[i]:nullptr;
                        if (i<mixed.interpolate.size() && mixed.interpolate[i]) continue;
                        std::cout << "HOLD_AUDIT ordinal=" << i << " kind=" << int(y.kind)
                                  << " key=" << y.key.object << ":" << y.key.generation
                                  << " texture=" << y.sprite.texture
                                  << " phase=" << int(y.phase)
                                  << " priorKind=" << (x?int(x->kind):-1)
                                  << " priorKey=" << (x?x->key.object:0) << ":" << (x?x->key.generation:0)
                                  << " priorTexture=" << (x?x->sprite.texture:0)
                                  << " priorWH=" << (x?x->sprite.destination.width:0) << "," << (x?x->sprite.destination.height:0)
                                  << " priorAlpha=" << (x?int(x->sprite.tint.a):0)
                                  << " particlePhase=" << (x?deathleak::linux_port::ParticlePhaseAdmitted(*x,y):false)
                                  << " sameSprite=" << (x&&x->kind==y.kind&&y.kind==deathleak::linux_port::PresentationCommandPacket::Kind::Sprite?deathleak::linux_port::SameSpriteExceptPosition(x->sprite,y.sprite):false)
                                  << " phaseAdmitted=" << (x&&x->kind==y.kind&&y.kind==deathleak::linux_port::PresentationCommandPacket::Kind::Sprite?deathleak::linux_port::PhaseAdmitted(*x,y,priorFullFrame,frameSnapshot):false)
                                  << " xy=" << y.sprite.destination.x << "," << y.sprite.destination.y
                                  << " wh=" << y.sprite.destination.width << "," << y.sprite.destination.height
                                  << " alpha=" << int(y.sprite.tint.a)
                                  << " rot=" << y.sprite.rotationDegrees << '\n';
                    }
                }

                const auto& before=priorFullFrame.packet.Commands();
                const auto& after=frameSnapshot.packet.Commands();
                for (std::size_t ci=0;ci<before.size() && ci<after.size();++ci) {
                    if (before[ci].kind==deathleak::linux_port::PresentationCommandPacket::Kind::Sprite &&
                        after[ci].kind==before[ci].kind && before[ci].key==after[ci].key &&
                        before[ci].sprite.texture!=after[ci].sprite.texture)
                        std::cout << "FULL_FRAME_TEXTURE_CHANGE frame=" << frame << " ordinal=" << ci
                                  << " key=" << before[ci].key.object << " texture="
                                  << before[ci].sprite.texture << "->" << after[ci].sprite.texture << '\n';
                }

                std::size_t total=0,tagged=0;
                for (const auto& command:frameSnapshot.packet.Commands())
                    if (command.kind==deathleak::linux_port::PresentationCommandPacket::Kind::Sprite) {
                        ++total; tagged += command.key.object != 0;
                    }
                for (std::size_t ci=0;ci<frameSnapshot.packet.Commands().size();++ci) {
                    const auto& c=frameSnapshot.packet.Commands()[ci];
                    if (c.kind==deathleak::linux_port::PresentationCommandPacket::Kind::Sprite && !c.key.object)
                        std::cout << "FULL_FRAME_UNOWNED frame=" << frame << " ordinal=" << ci
                                  << " texture=" << c.sprite.texture << " dst=" << c.sprite.destination.x
                                  << "," << c.sprite.destination.y << "," << c.sprite.destination.width
                                  << "," << c.sprite.destination.height << '\n';
                    if (c.kind==deathleak::linux_port::PresentationCommandPacket::Kind::SpriteBatch ||
                        c.kind==deathleak::linux_port::PresentationCommandPacket::Kind::FillBatch)
                        std::cout << "FULL_FRAME_BATCH frame=" << frame << " ordinal=" << ci
                                  << " kind=" << int(c.kind) << " count=" << c.sprites.size()+c.rects.size() << '\n';
                }
                std::cout << "FULL_FRAME_TOPOLOGY frame=" << frame << " commands=" << frameSnapshot.packet.Size()
                          << " sprites=" << tagged << '/' << total << " decision=" << int(decision) << '\n';
            }
            if (!mixedReplayPrefix.empty() && frame == mixedReplayFrame && hasPriorFullFrame) {
                // Diagnostic-only: replay complete sealed packets into the same SDL scene
                // target and unswapped output backbuffer. Begin/Zoom/postprocess, blend,
                // shaders and painter order are executed afresh for each arm. No arm is
                // presented; the next ordinary BeginFrame replaces the backbuffer.
                const auto current = frameSnapshot.packet;
                const auto alphaOne = deathleak::linux_port::SampleMixedCommands(priorFullFrame,frameSnapshot,1.f);
                const auto half = deathleak::linux_port::SampleMixedCommands(priorFullFrame,frameSnapshot,.5f);
                const auto captureReplay = [&](const auto& packet, const char* label) {
                    repeatRecorder->ReplayPacket(packet);
                    const std::string path = mixedReplayPrefix + "." + label + ".bmp";
                    if (!renderer->SaveFrame(path,error)) {
                        std::cerr << "Mixed replay capture failed: " << error << '\n'; return false;
                    }
                    std::cout << "MIXED_REPLAY frame=" << frame << " label=" << label
                              << " sceneEpoch=" << frameSnapshot.sceneEpoch
                              << " commands=" << packet.Size() << " sha256=" << Sha256File(path) << '\n';
                    return true;
                };
                if (!captureReplay(current,"current") || !captureReplay(alphaOne,"alpha1") ||
                    !captureReplay(half,"alpha05")) return 1;
            }
            priorFullFrame=std::move(frameSnapshot); hasPriorFullFrame=true;
        }
        if (fixedDt <= 0.f && !liveClockAnchored) {
            livePacer.RebaseAfterFirstPresent(std::chrono::steady_clock::now());
            liveClockAnchored = true;
        }
        if (refreshSlots && !renderer->PresentationSurfaceStable()) {
            std::cerr << "Refresh surface changed during ordinary present\n"; return 1;
        }
        if (profile || refreshSlots) notePresentation();
        if (refreshSlots) refreshClock.Presented(std::chrono::steady_clock::now());
        if (repeatPresentationCheck && repeatRecorder && frame == frameLimit - 1) {
            const auto repeatStart = std::chrono::steady_clock::now();
            repeatRecorder->Replay();
            const auto repeatDrawEnd = std::chrono::steady_clock::now();
            if (!renderer->SaveFrame(capture + ".repeat.bmp", error)) {
                std::cerr << "Repeat capture failed: " << error << '\n'; return 1;
            }
            const auto repeatPresentStart = std::chrono::steady_clock::now();
            repeatRecorder->PresentReplay();
            const auto repeatPresentEnd = std::chrono::steady_clock::now();
            std::cout << "PRESENTATION_REPEAT draw_ms="
                      << std::chrono::duration<double, std::milli>(repeatDrawEnd - repeatStart).count()
                      << " present_ms="
                      << std::chrono::duration<double, std::milli>(repeatPresentEnd - repeatPresentStart).count()
                      << " commands=" << repeatRecorder->Packet().Size()
                      << " capture=" << capture << ".repeat.bmp\n";
        }
#ifdef DEATHLEAK_TEST_LOADING_FADE_OBSERVER
        if (loadingPhaseOnEntry != LoadingPhase::None || loadingPhase != LoadingPhase::None) {
            std::cout << "FADE_OBSERVER frame=" << frame
                      << " entry=" << static_cast<int>(loadingPhaseOnEntry)
                      << " exit=" << static_cast<int>(loadingPhase)
                      << " gameplay=" << loadingGameplayRan
                      << " weather=" << loadingWeatherRan
                      << " scene=" << loadingSceneVisualsRan
                      << " player=" << loadingPlayerVisualRan
                      << " alpha=" << loadingDrawAlpha
                      << " hold_exit=" << loadingHoldExitedThisFrame
                      << " time=" << loadingPhaseTime << '\n';
        }
#endif
        if (showFps) ++presentedFramesInSample;
        precedingRenderedModifiedDelta = dt;
        shaderClock.RenderCompleted(dt);
        const auto afterDraw = std::chrono::steady_clock::now();
        drawSeconds += std::chrono::duration<double>(afterDraw-afterUpdate).count();
        if (profile && frame >= kProfileWarmupFrames) {
            const double frameMs = std::chrono::duration<double, std::milli>(afterDraw-now).count();
            profiledFrameMs.push_back(frameMs);
            if (frameMs > 33.333) {
                std::cout << "Profile slow_frame frame=" << frame
                          << " level=" << currentLevel
                          << " total_ms=" << frameMs
                          << " update_ms=" << std::chrono::duration<double, std::milli>(afterUpdate-now).count()
                          << " draw_present_ms=" << std::chrono::duration<double, std::milli>(afterDraw-afterUpdate).count()
                          << '\n';
            }
        }
    }
    if (!menuLoop && storyEnded && !saveFile.empty()) {
        const std::string temporary = saveFile + ".tmp";
        std::ofstream saved(temporary, std::ios::trunc);
        saved << "DL6\nMENU\nC:" << completions << '\n';
        if (!language.empty()) saved << "G:" << language << '\n';
        saved.close();
        if (!saved) { std::cerr << "Completion save write failed: " << temporary << '\n'; return 1; }
        std::error_code ec;
        std::filesystem::rename(temporary, saveFile, ec);
        if (ec) { std::cerr << "Completion save commit failed: " << ec.message() << '\n'; return 1; }
    }
    if (!playLevel.empty() && !saveFile.empty() && !storyEnded) {
        const std::string temporary = saveFile + ".tmp";
        std::ofstream saved(temporary, std::ios::trunc);
        saved << "DL5\n" << currentLevel << '\n' << std::setprecision(9) << worldX << ' ' << worldY
              << ' ' << combat.Health() << ' ' << combat.Focus() << '\n';
        saved << "B:" << orbBalance << '\n';
        if (completions) saved << "C:" << completions << '\n';
        if (!language.empty()) saved << "G:" << language << '\n';
        for (const auto& orb : collectedOrbs) saved << orb << '\n';
        for (const auto& key : ownedKeys) saved << "K:" << key << '\n';
        for (const auto& paper : collectedPapers) saved << "P:" << paper << '\n';
        for (const auto& level : rewardedHealthDevils) saved << "V:" << level << '\n';
        for (const auto& level : brokenSmashables) saved << "Z:" << level << '\n';
        if (healthDevilRewardCount) saved << "J:" << healthDevilRewardCount << '\n';
        for (const auto& door : unlockedDoors) saved << "D:" << door << '\n';
        if (combat.DeflectionUnlocked()) saved << "U:kDeflection\n";
        if (speedUnlocked) saved << "U:kSpeedIncrease\n";
        if (downwardDashUnlocked) saved << "U:kDownwardDash\n";
        if (rollUnlocked) saved << "U:kRoll\n";
        if (projectileDamageUnlocked) saved << "U:kProjectileDamageIncrease\n";
        if (projectileSpeedUnlocked) saved << "U:kProjectileSpeedIncrease\n";
        if (hasSavedMaxHealth) saved << "H:" << savedMaxHealth << '\n';
        if (!windowsPermanentExtension.empty()) saved << windowsPermanentExtension << '\n';
        for (const auto& [id, time] : combat.TriggerHistory()) saved << "N:" << id << '\t' << time << '\n';
        saved.close();
        if (!saved) { std::cerr << "Save write failed: " << temporary << '\n'; return 1; }
        std::error_code ec;
        std::filesystem::rename(temporary, saveFile, ec);
        if (ec) { std::cerr << "Save commit failed: " << ec.message() << '\n'; return 1; }
    }
    if (menuLoop && playLevel.empty()) std::cout << "Menu idle completion count=" << completions << " language=" << language << " afterEndStory=" << completionOccurred << '\n';
    if (!playLevel.empty()) {
        if (simulateOnly && healthPickup)
            std::cout << "Health upgrade pickup state x=" << healthPickup->x << " y=" << healthPickup->y
                      << " vx=" << healthPickup->vx << " vy=" << healthPickup->vy << '\n';
        const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        if (storyEnded) std::cout << "Main menu return event after story completion count=" << completions << " language=" << language << "\n";
        std::cout << "Simulation ended at " << currentLevel << " player=(" << worldX << "," << worldY
                  << ") orbs=" << collectedOrbs.size() << " keys=" << ownedKeys.size() << " health=" << combat.Health()
                  << " focus=" << combat.Focus() << " balance=" << orbBalance << " spawned=" << combat.Spawned() << " enemies=" << combat.EnemyCount()
                  << " hits=" << combat.Hits() << " kills=" << combat.Kills() << " fired=" << combat.Fired()
                  << " shots=" << combat.ShotCount() << " grounded=" << motion.Grounded() << " elapsed=" << elapsed << "s"
                  << " downward_dash_upgrade=" << downwardDashUnlocked << " roll_upgrade=" << rollUnlocked << " dash_active=" << motion.DownwardDashing() << " roll_active=" << motion.Rolling() << " vx=" << motion.VelocityX() << " vy=" << motion.VelocityY()
                  << " max_health=" << combat.MaxHealth() << " speed_upgrade=" << speedUnlocked << " projectile_damage_upgrade=" << projectileDamageUnlocked << " projectile_speed_upgrade=" << projectileSpeedUnlocked << " pending_smash_orbs=" << scene.PendingSpawnedOrbs() << "\n";
        if (combatState) {
            std::cout << "Player/NPC contacts=" << combat.PlayerContacts() << " contact_frames=" << combat.ContactFrames() << "\n";
            std::cout << "Melee active=" << combat.MeleeActive() << " strikes=" << combat.MeleeStrikes() << "\n";
            std::cout << "Deflection unlocked=" << combat.DeflectionUnlocked() << " count=" << combat.Deflections() << "\n";
            for (const auto& e : combat.Enemies()) std::cout << "Enemy " << e.type << " x=" << e.x << " y=" << e.y << " windup=" << e.windup << " vy=" << e.vy << " grounded=" << e.grounded << " combo=" << e.comboHits << " stun=" << e.stunTime << " teleport=" << e.teleportTime << " teleports=" << e.teleports << " jumps=" << e.jumps << " rolls=" << e.rolls << " roll=" << e.rollTime << "\n";
            for (const auto& s : combat.Shots()) std::cout << "Shot " << (s.enemy ? "enemy" : "player") << " x=" << s.x << " y=" << s.y << " vx=" << s.vx << " vy=" << s.vy << " deflected=" << s.wasDeflected << "\n";
        }
        if (profile) {
            std::cout << "Profile update=" << updateSeconds << "s draw/present=" << drawSeconds << "s\n";
            if (!profiledFrameMs.empty()) {
                std::sort(profiledFrameMs.begin(), profiledFrameMs.end());
                const auto percentile = [&](double fraction) {
                    const auto rank = static_cast<std::size_t>(std::ceil(fraction * profiledFrameMs.size()));
                    return profiledFrameMs[std::max<std::size_t>(1, rank) - 1];
                };
                const auto over25 = std::count_if(profiledFrameMs.begin(), profiledFrameMs.end(),
                    [](double ms) { return ms > 25.0; });
                const auto over33 = std::count_if(profiledFrameMs.begin(), profiledFrameMs.end(),
                    [](double ms) { return ms > 33.333; });
                std::cout << "Profile frame_ms warmup=" << kProfileWarmupFrames
                          << " samples=" << profiledFrameMs.size()
                          << " p50=" << percentile(0.50) << " p95=" << percentile(0.95)
                          << " p99=" << percentile(0.99) << " max=" << profiledFrameMs.back()
                          << " over25=" << over25 << " over33=" << over33 << '\n';
            }
        }
    }
    if (testRefreshHz) std::cout << "TEST_REFRESH_HZ=" << testRefreshHz
                                 << " display_slots=" << testDisplaySlots << '\n';
    if (refreshSlots) std::cerr << "Refresh-only presents=" << refreshOnlyPresents
                                << " surface_failures=" << refreshSurfaceFailures << "\n";
    if ((profile || refreshSlots) && !presentationIntervalsMs.empty()) {
        std::sort(presentationIntervalsMs.begin(), presentationIntervalsMs.end());
        const auto percentile = [&](double fraction) {
            const auto rank = static_cast<std::size_t>(std::ceil(fraction * presentationIntervalsMs.size()));
            return presentationIntervalsMs[std::max<std::size_t>(1, rank) - 1];
        };
        std::cout << "Present interval_ms samples=" << presentationIntervalsMs.size()
                  << " p50=" << percentile(0.50) << " p95=" << percentile(0.95)
                  << " p99=" << percentile(0.99) << " max=" << presentationIntervalsMs.back() << '\n';
    }
    if (fixedDt <= 0.f && livePacer.DroppedSteps())
        std::cerr << "Fixed-step overload dropped " << livePacer.DroppedSteps() << " update quanta\n";
    StopCaveDecodeWorker();
    return 0;
}

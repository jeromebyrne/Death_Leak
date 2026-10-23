#include "../render/Renderer.h"
#include "../game/LevelScene.h"
#include "../game/ShaderClock.h"
#include "../game/WeatherLightning.h"
#include "../game/WeatherRain.h"
#include "../game/Combat.h"
#include "../game/PlayerMotion.h"
#include "../game/CameraShake.h"
#include "../game/Hud.h"
#include "../game/BitmapText.h"
#include "../game/PaperText.h"
#include "../game/Settings.h"
#include "../audio/SoundPlayer.h"
#include <chrono>
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
#include <limits>

using namespace deathleak::render;

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
constexpr std::array<Rect, 3> optionsBounds = {{AuthoredUI(-420,-60,132,132),
    AuthoredUI(-420,-200,132,132), AuthoredUI(-390,-470,90,90)}};
}

int main(int argc, char** argv) {
    int frameLimit = 0;
    std::string capture;
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
    bool combatState = false;
    bool freshSaveIntro = false;
    int startEncounter = -1;
    int startHazard = -1;
    bool exhaustedFocus = false;
    bool suppressLandingSprayDraw = false;
    float fixedDt = 0;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--frames" && i + 1 < argc) frameLimit = std::atoi(argv[++i]);
        else if (arg == "--capture" && i + 1 < argc) capture = argv[++i];
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
        else if (arg == "--simulate-only") simulateOnly = true;
        else if (arg == "--simulate-wall-clock") simulateWallClock = true;
        else if (arg == "--menu-loop") menuLoop = true;
        else if (arg == "--profile") profile = true;
        else if (arg == "--combat-state") combatState = true;
        else if (arg == "--fresh-save-intro") freshSaveIntro = true;
        else if (arg == "--exhaust-focus") exhaustedFocus = true;
        else if (arg == "--capture-suppress-landing-draw") suppressLandingSprayDraw = true;
        else if (arg == "--start-encounter" && i + 1 < argc) startEncounter = std::atoi(argv[++i]);
        else if (arg == "--start-hazard" && i + 1 < argc) startHazard = std::atoi(argv[++i]);
        else { std::cerr << "Usage: deathleak-linux [--frames N] [--capture output.bmp] [--level-preview level.xml | --play level.xml]\n"; return 2; }
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
    if (fixedDt < 0 || fixedDt > 0.05f) { std::cerr << "--fixed-dt must be between 0 and 0.05\n"; return 2; }
    std::vector<char> scriptedInput;
    if (!script.empty()) {
        std::istringstream parts(script); std::string token;
        while (std::getline(parts, token, ',')) {
            auto colon = token.find(':');
            if (colon == std::string::npos || token.empty() || std::string("LRJ.FBNTQEIHMPXSOYZ").find(token[0]) == std::string::npos) { std::cerr << "Bad --script token\n"; return 2; }
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
    std::string error;
    if (!renderer->Initialize("Death Leak - Linux renderer slice", 1280, 720, error)) {
        std::cerr << "Renderer initialization failed: " << error << '\n'; return 1;
    }
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
    TextureId rainTexture = InvalidTexture;
    TextureId lightningTexture = InvalidTexture;
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
    auto finalFrame = (playLevel.empty() && !menuLoop) ? InvalidTexture : renderer->LoadTexture("Game/Media/UI/frame_top.png", error);
    if (!finalFrame && !(playLevel.empty() && !menuLoop))
        std::cerr << "HUD frame_top.png load failed: " << error << '\n';
    std::array<TextureId, 4> filmFrames{};
    // game_hud's scratch widget uses its own wall-clock 11 FPS cadence, not
    // the capped gameplay/shader delta. Start at frame one on its first draw.
    size_t gameplayFilmIndex = 0;
    std::optional<std::chrono::steady_clock::time_point> gameplayFilmLastSwitch;
    auto loadFilmFrames = [&] {
        if (simulateOnly || filmFrames[0]) return;
        constexpr std::array<const char*, 4> filmPaths = {
            "Game/Media/UI/film_frame.png", "Game/Media/UI/film_frame_2.png",
            "Game/Media/UI/film_frame_3.png", "Game/Media/UI/film_frame_4.png"};
        for (size_t i = 0; i < filmPaths.size(); ++i) filmFrames[i] = renderer->LoadTexture(filmPaths[i], error);
    };
    if (playLevel == "Game/XmlFiles/levels/sea_2.xml") loadFilmFrames();
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
    for (int frame = 0; !frameLimit || frame < frameLimit; ++frame) {
        if (renderer->PollQuit()) break;
        const ControllerSticks controllerSticks = renderer->ControllerAxes();
        deathleak::game::PlayerStrafeVisual strafeVisual{};
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;
        if (fixedDt > 0) dt = fixedDt;
        // Original program.cpp caps the modified update delta to its 60 Hz target
        // before Update/Render; the completed frame commits that same delta.
        constexpr float kTargetDelta = 1.f / 60.f;
        if (dt > kTargetDelta) dt = kTargetDelta;
        // This is the port's update delta; feed the modified delta here when
        // Windows-style damage slowdown is implemented.
        shaderClock.Update(dt);
        cameraShakeTime += dt;
        const char command = frame < static_cast<int>(scriptedInput.size()) ? scriptedInput[frame] : 0;
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
                    hasPulledSword = true; introHeldTime = 0.f; introEffectiveZoom = scene.CameraZoom();
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
        } else {
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
            // Raw Interact is held independently of world-object activation.
            // Normal motion, combat, roll, jump, interaction, and right-stick
            // target updates remain suppressed through the one-shot pull.
            if (scene.IntroBodyFinished()) {
                hasPulledSword = true;
                scene.SetIntroBodyPhase(deathleak::game::LevelScene::IntroBodyPhase::None);
                introHeldTime = 0.f;
                introEffectiveZoom = introLoadZoom;
                std::cout << "Fresh-save intro completed at frame " << frame << '\n';
            } else {
                if (scene.GetIntroBodyPhase() == deathleak::game::LevelScene::IntroBodyPhase::Looped) {
                    const bool held = !script.empty() ? command == 'I' : renderer->KeyDown(Interact);
                    if (held) {
                        introHeldTime += dt;
                        if (introHeldTime > 2.75f) {
                            introHeldTime = 0.f;
                            introEffectiveZoom = introLoadZoom;
                            scene.SetIntroBodyPhase(deathleak::game::LevelScene::IntroBodyPhase::Pull);
                            std::cout << "Fresh-save intro pull at frame " << frame << '\n';
                        } else introEffectiveZoom = std::max(introLoadZoom * 0.75f, introEffectiveZoom - dt * 0.05f);
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
                        currentLevel = restartLevel;
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
            motion.Update(scene, dt, (combat.MeleeActive() || motion.Rolling()) ? 0 : static_cast<int>(direction),
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
            const bool fire = (!script.empty() ? (command == 'F' || command == 'B' || command == 'N' || command == 'Y' || command == 'Z') : renderer->KeyDown(Down)) || stickAim;
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
                script.empty() && renderer->KeyDown(Down) &&
                    motion.Grounded() && motion.WallSide() == 0 && !motion.Rolling() &&
                    !combat.MeleeActive(),
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
                        currentLevel = door.destination;
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
                        currentLevel = destination;
                        loadFilmFrames();
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
                        currentLevel = destination;
                        worldX = scene.PlayerX(); worldY = scene.PlayerY();
                        cameraX = scene.CameraX(); cameraY = scene.CameraY(); cameraShake.Reset();
                        motion.Reset(worldX, worldY, true); combat.ClearLevel();
                        levelTime = interactHeldTime = doorUnlockWait = 0; unlockingDoor = false; interactionTarget.clear();
                        std::cout << "Entered " << currentLevel << std::endl;
                    } else std::cerr << "Transition failed: " << error << '\n';
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
            menuSelection = 0; menuOptions = menuLanguages = false;
        }
        auto afterUpdate = std::chrono::steady_clock::now();
        updateSeconds += std::chrono::duration<double>(afterUpdate-now).count();
        if (!playLevel.empty()) {
            weather.SetAllowed(scene.AllowsWeather());
            if (!paperModal && !upgradeModal) {
                weather.Update(dt);
                rain.Update(dt, weather.Raining(), cameraX, cameraY);
            }
        }
        if (simulateOnly) {
            // Keep the intro terminal signal advancing in deterministic CPU-only
            // simulations; ordinary gameplay visuals remain skipped.
            if (!hasPulledSword) scene.UpdatePlayerVisual(dt, 0.f, 0.f, motion.Grounded(),
                motion.WallSide(), false, false, motion.InWater(), 0.f, facing,
                false, false, {}, false, 0.f, 0.f, precedingRenderedModifiedDelta);
            precedingRenderedModifiedDelta = dt;
            continue;
        }
        const bool objectsPaused = paperModal || upgradeModal;
        scene.UpdateVisuals(dt, worldX, worldY, cameraX, cameraY, objectsPaused, precedingRenderedModifiedDelta);
        if (!objectsPaused) combat.UpdateVisuals(dt);
        if (!playLevel.empty() && !objectsPaused) scene.UpdatePlayerVisual(dt, motion.VelocityX(), motion.VelocityY(),
            motion.Grounded(), motion.WallSide(), motion.Rolling(), motion.DownwardDashing(), motion.InWater(),
            combat.MeleeElapsed(), facing, playerProjectileFired, playerEligibleFireAttempt, strafeVisual, stickAim, aimX, aimY, precedingRenderedModifiedDelta);
        // Camera2D draws X's random sign before Y's, only while shaking.
        const bool jitter = cameraShake.WillJitter(cameraShakeTime - dt);
        const int shakeSignX = jitter && scene.CameraFollowX() ? ((std::rand() & 1) ? 1 : -1) : 1;
        const int shakeSignY = jitter && scene.CameraFollowY() ? ((std::rand() & 1) ? 1 : -1) : 1;
        const auto shakeView = cameraShake.Update(cameraShakeTime - dt, 1.f, scene.CameraFollowX(), scene.CameraFollowY(),
            shakeSignX, shakeSignY, cameraX, cameraY);
        const float drawCameraX = shakeView.viewX, drawCameraY = shakeView.viewY;
        const auto shaderFrame = shaderClock.Draw();
        renderer->BeginFrame({8, 12, 20, 255});
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
            std::vector<deathleak::game::PainterItem> painterItems;
            combat.AppendPainterItems(painterItems, *renderer, drawCameraX, drawCameraY);
            if (healthPickup && healthPickupArt) {
                constexpr float scale = 2.f / 3.f;
                const auto sprite = deathleak::render::Sprite{healthPickupArt, {},
                    {640.f + (healthPickup->x-drawCameraX-25.f)*scale,
                     360.f + (drawCameraY-healthPickup->y-35.f)*scale,
                     50.f*scale, 70.f*scale}};
                // HealthUpgradePickup inherits SolidMovingSprite's kPlayer depth.
                painterItems.push_back({3000, [&, sprite] { renderer->DrawSprite(sprite); }});
            }
            if (weather.Alpha() > 0.f) {
                if (!lightningTexture) {
                    std::string lightningError;
                    lightningTexture = renderer->LoadTexture("Game/Media/lightning_layer.png", lightningError);
                    if (!lightningTexture) std::cerr << "Lightning texture load failed: " << lightningError << '\n';
                }
                if (lightningTexture) {
                    const Sprite flash = weather.SpriteForCamera(lightningTexture, drawCameraX, drawCameraY);
                    painterItems.push_back({20, [&, flash] { renderer->DrawSprite(flash); }});
                }
            }
            if (rain.VisibleParticles()) {
                if (!rainTexture) {
                    std::string rainError;
                    rainTexture = renderer->LoadTexture("Game/Media/rain_drop.png", rainError);
                    if (!rainTexture) std::cerr << "Rain texture load failed: " << rainError << '\n';
                    rain.SetTexture(rainTexture);
                }
                painterItems.push_back({20, [&] { rain.Draw(*renderer, drawCameraX, drawCameraY); }});
            }
            scene.Draw(*renderer, drawCameraX, drawCameraY, worldX, worldY, std::move(painterItems),
                       combat.PlayerDamageBurstGain(), weather.FoliageSwayMultiplier(), shaderFrame, suppressLandingSprayDraw);
            renderer->ApplyScenePostprocess();
            // ShaderClock advanced at loop start; the original meter samples total
            // before this frame's modified delta is committed.
            if (currentLevel == "Game/XmlFiles/levels/sea_2.xml") {
                // final_scene_hud: four authored film frames at 11 FPS and
                // quarter opacity, plus the original frame_top art. Bitmap
                // lettering remains a typography fallback for Impact.
                const int filmIndex = static_cast<int>(finalSceneTime * 11.f) % 4;
                if (filmFrames[filmIndex]) renderer->DrawSprite({filmFrames[filmIndex], {}, {0,0,1280,720}, {255,255,255,64}});
                if (finalFrame) {
                    renderer->DrawSprite({finalFrame, {}, {0,0,1280,100}});
                    Sprite bottomBar{finalFrame, {}, {0,620,1280,100}};
                    bottomBar.flipY = true;
                    renderer->DrawSprite(bottomBar);
                }
                if (finalTitleShown) deathleak::game::DrawBitmapText(*renderer, "DEATH LEAK", 200, 255, 16, {255,0,0,245});
            } else {
                loadFilmFrames();
                const auto filmNow = std::chrono::steady_clock::now();
                if (!gameplayFilmLastSwitch) gameplayFilmLastSwitch = filmNow;
                // UIAnimatedSprite::Update uses a strict, integer-millisecond
                // threshold and advances at most one frame per UI update.
                else if (filmNow - *gameplayFilmLastSwitch > std::chrono::milliseconds(1000 / 11)) {
                    gameplayFilmIndex = (gameplayFilmIndex + 1) % filmFrames.size();
                    gameplayFilmLastSwitch = filmNow;
                }
                if (filmFrames[gameplayFilmIndex])
                    renderer->DrawSprite({filmFrames[gameplayFilmIndex], {}, {0,0,1280,720}, {255,255,255,51}});
                // game_hud.xml: opaque 50-unit bands follow the film and precede HUD widgets.
                if (finalFrame) {
                    renderer->DrawSprite({finalFrame, {}, AuthoredUI(-960, 490, 1920, 50), {255,255,255,255}});
                    Sprite bottomBar{finalFrame, {}, AuthoredUI(-960, -540, 1920, 50), {255,255,255,255}};
                    bottomBar.flipY = true;
                    renderer->DrawSprite(bottomBar);
                }
                hud.Draw(*renderer, combat.Health(), combat.MaxHealth(), combat.Focus(), orbBalance, shaderFrame.totalTime - dt);
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
        }
        if (!capture.empty() && frame == frameLimit - 1 && !renderer->SaveFrame(capture, error)) {
            std::cerr << "Capture failed: " << error << '\n'; return 1;
        }
        renderer->EndFrame();
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
    return 0;
}

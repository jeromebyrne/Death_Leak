#include "../render/Renderer.h"
#include "../game/LevelScene.h"
#include "../game/Combat.h"
#include "../game/PlayerMotion.h"
#include "../game/Hud.h"
#include "../game/BitmapText.h"
#include "../audio/SoundPlayer.h"
#include <chrono>
#include <cstdint>
#include <unordered_map>
#include <algorithm>
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

using namespace deathleak::render;

int main(int argc, char** argv) {
    int frameLimit = 0;
    std::string capture;
    std::string levelPreview;
    std::string playLevel;
    std::string script;
    std::string saveFile, loadSaveFile;
    float savedX = 0, savedY = 0, savedHealth = 25, savedFocus = 100;
    bool restorePosition = false;
    std::unordered_set<std::string> collectedOrbs, ownedKeys, unlockedDoors;
    bool deflectionUnlocked = false;
    int orbBalance = 0;
    bool balanceLoaded = false;
    std::unordered_map<std::string, std::int64_t> triggerHistory;
    bool simulateOnly = false;
    bool profile = false;
    bool combatState = false;
    int startEncounter = -1;
    int startHazard = -1;
    bool exhaustedFocus = false;
    float fixedDt = 0;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--frames" && i + 1 < argc) frameLimit = std::atoi(argv[++i]);
        else if (arg == "--capture" && i + 1 < argc) capture = argv[++i];
        else if (arg == "--level-preview" && i + 1 < argc) levelPreview = argv[++i];
        else if (arg == "--play" && i + 1 < argc) playLevel = argv[++i];
        else if (arg == "--save-file" && i + 1 < argc) saveFile = argv[++i];
        else if (arg == "--load-save" && i + 1 < argc) loadSaveFile = argv[++i];
        else if (arg == "--script" && i + 1 < argc) script = argv[++i];
        else if (arg == "--fixed-dt" && i + 1 < argc) fixedDt = std::atof(argv[++i]);
        else if (arg == "--simulate-only") simulateOnly = true;
        else if (arg == "--profile") profile = true;
        else if (arg == "--combat-state") combatState = true;
        else if (arg == "--exhaust-focus") exhaustedFocus = true;
        else if (arg == "--start-encounter" && i + 1 < argc) startEncounter = std::atoi(argv[++i]);
        else if (arg == "--start-hazard" && i + 1 < argc) startHazard = std::atoi(argv[++i]);
        else { std::cerr << "Usage: deathleak-linux [--frames N] [--capture output.bmp] [--level-preview level.xml | --play level.xml]\n"; return 2; }
    }
    if (!playLevel.empty() && !levelPreview.empty()) { std::cerr << "Choose either --play or --level-preview\n"; return 2; }
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
            else if (line == "U:kDeflection") deflectionUnlocked = true;
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
    if ((simulateOnly || !script.empty()) && playLevel.empty()) { std::cerr << "Simulation options require --play\n"; return 2; }
    if (fixedDt < 0 || fixedDt > 0.05f) { std::cerr << "--fixed-dt must be between 0 and 0.05\n"; return 2; }
    std::vector<char> scriptedInput;
    if (!script.empty()) {
        std::istringstream parts(script); std::string token;
        while (std::getline(parts, token, ',')) {
            auto colon = token.find(':');
            if (colon == std::string::npos || token.empty() || std::string("LRJ.FBNTQEIHMPX").find(token[0]) == std::string::npos) { std::cerr << "Bad --script token\n"; return 2; }
            int count = std::atoi(token.c_str() + colon + 1);
            if (count < 1 || count > 100000 || scriptedInput.size() + count > 100000) { std::cerr << "Bad --script duration\n"; return 2; }
            scriptedInput.insert(scriptedInput.end(), count, token[0]);
        }
        if (!frameLimit) frameLimit = scriptedInput.size();
    }
    auto renderer = CreateRenderer();
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
        }
    }
    deathleak::game::LevelScene scene;
    const std::string levelPath = !playLevel.empty() ? playLevel : levelPreview;
    if (!levelPath.empty() && !scene.Load(levelPath, *renderer, error)) {
        std::cerr << error << '\n'; return 1;
    }
    if (restorePosition) scene.ApplyCollected(collectedOrbs);
    scene.ApplyKeys(ownedKeys);
    scene.ApplyDeflection(deflectionUnlocked);
    if (!levelPath.empty()) {
        std::cout << "Loaded " << scene.SpriteCount() << " original level sprites; "
                  << scene.SkippedCount() << " missing or unsupported assets; "
                  << scene.Ground().size() << " ground segments; " << scene.Triggers().size() << " level triggers\n";
    }
    auto background = levelPath.empty() ? renderer->LoadTexture("Game/Media/UI/main_menu_2.png", error) : InvalidTexture;
    if (levelPath.empty() && !background) { std::cerr << "Background load failed: " << error << '\n'; return 1; }
    auto player = levelPath.empty() ? renderer->LoadTexture("Game/Media/UI/player_silhouette.png", error) : InvalidTexture;
    if (levelPath.empty() && !player) { std::cerr << "Player art load failed: " << error << '\n'; return 1; }
    float playerX = 590, playerY = 430;
    deathleak::game::PlayerMotion motion;
    motion.Reset(restorePosition ? savedX : scene.PlayerX(), restorePosition ? savedY : scene.PlayerY());
    float worldX = motion.X(), worldY = motion.Y();
    float cameraX = scene.CameraX(), cameraY = scene.CameraY();
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
    auto deathArt = playLevel.empty() ? InvalidTexture : renderer->LoadTexture("Game/Media/UI/you_died.png", error);
    deathleak::game::Hud hud;
    if (!playLevel.empty()) hud.Load(*renderer);
    deathleak::game::Combat combat;
    combat.SetDeflectionUnlocked(deflectionUnlocked);
    combat.SetTriggerHistory(std::move(triggerHistory));
    if (restorePosition) combat.SetVitals(savedHealth, savedFocus);
    if (exhaustedFocus) combat.SetFocus(0);
    double updateSeconds = 0, drawSeconds = 0;
    bool wasDead = false, restartHeld = false;
    float levelTime = 0.f, interactHeldTime = 0.f, doorUnlockWait = 0.f;
    bool unlockingDoor = false;
    bool upgradeModal = false;
    float modalWarmup = 0.f;
    bool purchaseHeld = false, dismissHeld = false;
    auto modalBackground = playLevel.empty() ? InvalidTexture : renderer->LoadTexture("Game/Media/UI/blank_dark.png", error);
    auto modalOrb = playLevel.empty() ? InvalidTexture : renderer->LoadTexture("Game/Media/UI/orb.png", error);
    auto modalButton = playLevel.empty() ? InvalidTexture : renderer->LoadTexture("Game/Media/UI/gamepad_icons/y.png", error);
    std::string interactionTarget;
    int facing = 1;
    std::string currentLevel = playLevel;
    auto start = std::chrono::steady_clock::now();
    auto last = std::chrono::steady_clock::now();
    for (int frame = 0; !frameLimit || frame < frameLimit; ++frame) {
        if (renderer->PollQuit()) break;
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;
        if (fixedDt > 0) dt = fixedDt;
        if (dt > 0.05f) dt = 0.05f;
        if (renderer->KeyDown(Escape) && !upgradeModal && !dismissHeld) break;
        if (!renderer->KeyDown(Escape) && !upgradeModal) dismissHeld = false;
        if (playLevel.empty()) {
            if (renderer->KeyDown(Left)) playerX -= 250 * dt;
            if (renderer->KeyDown(Right)) playerX += 250 * dt;
            if (renderer->KeyDown(Up)) playerY -= 250 * dt;
            if (renderer->KeyDown(Down)) playerY += 250 * dt;
        } else {
            const char command = frame < static_cast<int>(scriptedInput.size()) ? scriptedInput[frame] : 0;
            if (upgradeModal) {
                modalWarmup += dt;
                const bool purchase = !script.empty() ? command == 'P' : renderer->KeyDown(Purchase);
                const bool dismiss = !script.empty() ? command == 'X' : renderer->KeyDown(Escape);
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
            } else {
            const float direction = !script.empty() ? static_cast<float>(command == 'R' || command == 'B' || command == 'E') - static_cast<float>(command == 'L' || command == 'N' || command == 'Q') :
                static_cast<float>(renderer->KeyDown(Right)) - static_cast<float>(renderer->KeyDown(Left));
            if (direction != 0) facing = direction < 0 ? -1 : 1;
            const bool restart = !script.empty() ? command == 'T' : renderer->KeyDown(Restart);
            if (combat.Health() <= 0) {
                if (!wasDead) { std::cout << "Player died at frame " << frame << " in " << currentLevel << '\n'; wasDead = true; }
                if (restart && !restartHeld) {
                    deathleak::game::LevelScene nextScene;
                    // Authored you_died.xml reloads grass_exploration_3.xml.
                    const std::string restartLevel = "Game/XmlFiles/levels/grass_exploration_3.xml";
                    if (nextScene.Load(restartLevel, *renderer, error)) {
                        nextScene.ApplyCollected(collectedOrbs);
                        nextScene.ApplyKeys(ownedKeys);
                        nextScene.ApplyDeflection(combat.DeflectionUnlocked());
                        scene = std::move(nextScene);
                        currentLevel = restartLevel;
                        motion.Reset(scene.PlayerX(), scene.PlayerY());
                        worldX = motion.X(); worldY = motion.Y();
                        cameraX = scene.CameraX(); cameraY = scene.CameraY();
                        combat.ClearLevel(); combat.SetVitals(25.f, 100.f);
                        wasDead = false; levelTime = interactHeldTime = doorUnlockWait = 0; unlockingDoor = false; interactionTarget.clear();
                        std::cout << "Restarted " << currentLevel << " at frame " << frame << '\n';
                    } else std::cerr << "Restart failed: " << error << '\n';
                }
                restartHeld = restart;
            } else {
            restartHeld = restart;
            const bool jump = !script.empty() ? (command == 'J' || command == 'Q' || command == 'E') : renderer->KeyDown(Up);
            const bool melee = !script.empty() ? command == 'M' : renderer->KeyDown(Melee);
            motion.Update(scene, dt, combat.MeleeActive() ? 0 : static_cast<int>(direction), jump && !combat.MeleeActive());
            worldX = motion.X(); worldY = motion.Y();
            const auto collectedNow = scene.CollectCurrencyOrbs(worldX, worldY, collectedOrbs);
            orbBalance += static_cast<int>(collectedNow);
            if (sound && collectedNow) sound->Play(deathleak::audio::Cue::OrbCollect);
            const bool fire = !script.empty() ? (command == 'F' || command == 'B' || command == 'N') : renderer->KeyDown(Down);
            const int firedBefore = combat.Fired();
            combat.Update(scene, worldX, worldY, dt, fire, melee, motion.Grounded(), facing, *renderer);
            // H remains a simulation-only direct hit for isolated combo/stun
            // checks; normal melee comes through Combat::Update above.
            if (simulateOnly && command == 'H') combat.DamageEnemy(0,1.5f);
            if (sound && combat.Fired() > firedBefore) sound->Play(deathleak::audio::Cue::KnifeThrow, firedBefore);
            cameraX += (worldX - cameraX) * std::min(1.f, 8.f * dt);
            cameraY += (worldY - cameraY) * std::min(1.f, 4.f * dt);
            levelTime += dt;
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
            std::string target;
            const deathleak::game::KeyPickup* nearbyKey = nullptr;
            const deathleak::game::DojoScroll* nearbyScroll = nullptr;
            const deathleak::game::Door* nearbyDoor = nullptr;
            for (const auto& key : scene.Keys()) if (!ownedKeys.count(key.keyId) && overlap(key.x,key.y,key.width,key.height)) {
                nearbyKey = &key; target = "K:" + key.keyId; break;
            }
            if (!nearbyKey && !combat.DeflectionUnlocked())
                for (const auto& scroll : scene.Scrolls()) if (scroll.feature == "kDeflection" && overlap(scroll.x,scroll.y,scroll.width,scroll.height)) {
                    nearbyScroll = &scroll; target = "S:" + scroll.id; break;
                }
            if (!nearbyKey && !nearbyScroll && levelTime >= 0.75f && motion.Grounded())
                for (const auto& door : scene.Doors()) if (overlap(door.x,door.y,door.width,door.height)) {
                    nearbyDoor = &door; target = "D:" + door.id + door.destination; break;
                }
            if (!interact || target != interactionTarget) interactHeldTime = 0;
            interactionTarget = target;
            if (interact && !target.empty()) interactHeldTime += dt;
            if (interactHeldTime >= 0.05f && nearbyKey) {
                ownedKeys.insert(nearbyKey->keyId);
                scene.ApplyKeys(ownedKeys);
                std::cout << "Collected key " << nearbyKey->keyId << " at frame " << frame << '\n';
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
                        nextScene.ApplyDeflection(combat.DeflectionUnlocked());
                        scene = std::move(nextScene);
                        std::cout << "Door at frame " << frame << " from " << currentLevel << " to " << door.destination << '\n';
                        currentLevel = door.destination;
                        motion.Reset(scene.PlayerX(),scene.PlayerY(), true);
                        worldX = motion.X(); worldY = motion.Y();
                        cameraX = scene.CameraX(); cameraY = scene.CameraY();
                        combat.ClearLevel(); levelTime = interactHeldTime = doorUnlockWait = 0; unlockingDoor = false; interactionTarget.clear();
                    } else std::cerr << "Door transition failed: " << error << '\n';
                    continue;
                }
            }
            for (const auto& trigger : scene.Triggers()) {
                if (std::abs(worldX - trigger.x) < trigger.width * 0.5f &&
                    std::abs(worldY - trigger.y) < trigger.height * 0.5f && trigger.destination != currentLevel) {
                    // Loading replaces scene.Triggers(); never retain a reference into it.
                    const std::string destination = trigger.destination;
                    std::cout << "Trigger at frame " << frame << " from " << currentLevel << " player=(" << worldX << "," << worldY << ") to " << destination << std::endl;
                    deathleak::game::LevelScene nextScene;
                    if (nextScene.Load(destination, *renderer, error, currentLevel)) {
                        nextScene.ApplyCollected(collectedOrbs);
                        nextScene.ApplyKeys(ownedKeys);
                        nextScene.ApplyDeflection(combat.DeflectionUnlocked());
                        scene = std::move(nextScene);
                        currentLevel = destination;
                        worldX = scene.PlayerX(); worldY = scene.PlayerY();
                        cameraX = scene.CameraX(); cameraY = scene.CameraY();
                        motion.Reset(worldX, worldY, true); combat.ClearLevel();
                        levelTime = interactHeldTime = doorUnlockWait = 0; unlockingDoor = false; interactionTarget.clear();
                        std::cout << "Entered " << currentLevel << '\n';
                    } else std::cerr << "Transition failed: " << error << '\n';
                    break;
                }
            }
            }
            }
        }
        auto afterUpdate = std::chrono::steady_clock::now();
        updateSeconds += std::chrono::duration<double>(afterUpdate-now).count();
        if (simulateOnly) continue;
        renderer->BeginFrame({8, 12, 20, 255});
        if (levelPath.empty()) {
            renderer->DrawSprite({background, {}, {0, 0, 1280, 720}});
            renderer->DrawSprite({player, {}, {playerX, playerY, 100, 170}});
        } else if (playLevel.empty())
            scene.Draw(*renderer, scene.CameraX() + (playerX - 590) * 1.5f,
                       scene.CameraY() + (430 - playerY) * 1.5f, scene.PlayerX(), scene.PlayerY());
        else {
            scene.Draw(*renderer, cameraX, cameraY, worldX, worldY);
            combat.Draw(*renderer, cameraX, cameraY);
            hud.Draw(*renderer, combat.Health(), combat.Focus());
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
        drawSeconds += std::chrono::duration<double>(std::chrono::steady_clock::now()-afterUpdate).count();
    }
    if (!playLevel.empty() && !saveFile.empty()) {
        const std::string temporary = saveFile + ".tmp";
        std::ofstream saved(temporary, std::ios::trunc);
        saved << "DL5\n" << currentLevel << '\n' << std::setprecision(9) << worldX << ' ' << worldY
              << ' ' << combat.Health() << ' ' << combat.Focus() << '\n';
        saved << "B:" << orbBalance << '\n';
        for (const auto& orb : collectedOrbs) saved << orb << '\n';
        for (const auto& key : ownedKeys) saved << "K:" << key << '\n';
        for (const auto& door : unlockedDoors) saved << "D:" << door << '\n';
        if (combat.DeflectionUnlocked()) saved << "U:kDeflection\n";
        for (const auto& [id, time] : combat.TriggerHistory()) saved << "N:" << id << '\t' << time << '\n';
        saved.close();
        if (!saved) { std::cerr << "Save write failed: " << temporary << '\n'; return 1; }
        std::error_code ec;
        std::filesystem::rename(temporary, saveFile, ec);
        if (ec) { std::cerr << "Save commit failed: " << ec.message() << '\n'; return 1; }
    }
    if (!playLevel.empty()) {
        const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        std::cout << "Simulation ended at " << currentLevel << " player=(" << worldX << "," << worldY
                  << ") orbs=" << collectedOrbs.size() << " keys=" << ownedKeys.size() << " health=" << combat.Health()
                  << " focus=" << combat.Focus() << " balance=" << orbBalance << " spawned=" << combat.Spawned() << " enemies=" << combat.EnemyCount()
                  << " hits=" << combat.Hits() << " kills=" << combat.Kills() << " fired=" << combat.Fired()
                  << " shots=" << combat.ShotCount() << " grounded=" << motion.Grounded() << " elapsed=" << elapsed << "s\n";
        if (combatState) {
            std::cout << "Player/NPC contacts=" << combat.PlayerContacts() << " contact_frames=" << combat.ContactFrames() << "\n";
            std::cout << "Melee active=" << combat.MeleeActive() << " strikes=" << combat.MeleeStrikes() << "\n";
            std::cout << "Deflection unlocked=" << combat.DeflectionUnlocked() << " count=" << combat.Deflections() << "\n";
            for (const auto& e : combat.Enemies()) std::cout << "Enemy " << e.type << " x=" << e.x << " y=" << e.y << " windup=" << e.windup << " vy=" << e.vy << " grounded=" << e.grounded << " combo=" << e.comboHits << " stun=" << e.stunTime << " teleport=" << e.teleportTime << " teleports=" << e.teleports << " jumps=" << e.jumps << " rolls=" << e.rolls << " roll=" << e.rollTime << "\n";
            for (const auto& s : combat.Shots()) std::cout << "Shot " << (s.enemy ? "enemy" : "player") << " x=" << s.x << " y=" << s.y << " vx=" << s.vx << " vy=" << s.vy << " deflected=" << s.wasDeflected << "\n";
        }
        if (profile) std::cout << "Profile update=" << updateSeconds << "s draw/present=" << drawSeconds << "s\n";
    }
    return 0;
}

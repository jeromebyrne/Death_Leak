#include "Backends/Mac/MacGameBootstrap.h"

#include "Animation/Animation.h"
#include "Core/DataValue.h"
#include "Core/Timing.h"
#include "Core/Vector2.h"
#include "Core/XmlDocument.h"
#include "Core/XmlUtilities.h"
#include "FeatureUnlockManager.h"
#include "GameObjects/MaterialManager.h"
#include "GameObjects/LevelObjectRecord.h"
#include "GameObjects/SpriteRecord.h"
#include "LevelProperties.h"
#include "PlayerLevelManager.h"
#include "SaveManager.h"
#include "Settings.h"
#include "SineWave.h"
#include "Engine/Runtime/GameLoopController.h"
#include "StringManager.h"

#include <chrono>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <thread>

MacGameBootstrap::MacGameBootstrap(std::filesystem::path repoRoot):
    mFiles(repoRoot),
    mRepoRoot(std::move(repoRoot)),
    mTime()
{
}

MacGameBootstrap::~MacGameBootstrap() = default;

int MacGameBootstrap::RunSmoke()
{
    Timing::Create();
    InitialiseLegacyData();

    PlatformWindowConfig config;
    config.Title = "Death Leak";
    config.Width = 1280;
    config.Height = 720;

    if (!mApp.Initialise(config))
    {
        return 1;
    }

    std::cout << "Death Leak SDL macOS smoke app initialised: "
              << config.Width << "x" << config.Height << "\n";

    mRenderer = std::make_unique<MacSdlRenderer>(mApp.Window());
    if (!mRenderer->Initialise(config.Width, config.Height))
    {
        return 1;
    }

    BuildStaticLevelPreview();
    RunFrameLoop();
    RunDataSmokeChecks();

    std::cout << "Death Leak macOS smoke app shutdown\n";
    return 0;
}

void MacGameBootstrap::InitialiseLegacyData()
{
    PlayerLevelManager::GetInstance()->Initialise();
    mAudio.Initialise();
    mAudio.SetMasterVolume(0.5f);
    mSteam.Initialise(995990);
    StringManager::Create();
    StringManager::GetInstance()->SetIsOnSteamDeck(mSteam.IsSteamDeck());
}

void MacGameBootstrap::RunFrameLoop()
{
    GameLoopController loop(mApp, mInput, mAudio, mSteam, mTime);
    GameLoopConfig config;
    config.TargetDeltaSeconds = 1.0 / 60.0;

    int frameCount = 0;
    loop.Run(config, [this, &frameCount](float delta) {
        UpdatePreviewCamera(delta);
        if (mInput.IsDown(InputButton::Pause))
        {
            mApp.RequestQuit();
        }

        mRenderer->BeginFrame(Color{0.02f, 0.015f, 0.01f, 1.0f});
        DrawStaticLevelPreview();

        RectDrawCommand centerDot;
        centerDot.X = 640.0f;
        centerDot.Y = 360.0f;
        centerDot.Width = 18.0f;
        centerDot.Height = 18.0f;
        centerDot.Fill = Color{0.95f, 0.9f, 0.15f, 0.9f};
        centerDot.Outline = Color{0.1f, 0.1f, 0.1f, 1.0f};
        mRenderer->DrawRect(centerDot);

        if (frameCount % 30 == 0)
        {
            std::cout << "macOS smoke frame at " << mTime.Seconds()
                      << "s leftX=" << mInput.LeftStickX()
                      << " jump=" << (mInput.IsDown(InputButton::Jump) ? "true" : "false")
                      << "\n";
        }
        mRenderer->EndFrame();
        ++frameCount;
    });
}

void MacGameBootstrap::BuildStaticLevelPreview()
{
    XmlDocument levelDocument;
    const std::string previewLevelPath = mFiles.AssetPath("XmlFiles\\levels\\dojo.xml");
    if (!levelDocument.Load(previewLevelPath))
    {
        return;
    }

    LevelProperties previewLevelProperties;
    previewLevelProperties.XmlRead(levelDocument.GetRoot()->FirstChildElement("LevelProperties"));
    const Vector2 initialCamera = previewLevelProperties.GetInitialCamPos();
    mPreviewCameraX = initialCamera.X;
    mPreviewCameraY = initialCamera.Y;
    mPreviewZoom = previewLevelProperties.GetZoomInPercent() > 0.0f ? previewLevelProperties.GetZoomInPercent() : 1.0f;

    const auto records = LevelObjectRecord::ReadLevelObjects(levelDocument.GetRoot());
    mStaticPreviewSprites.clear();
    mMissingTextureSprites.clear();
    mStaticPreviewSprites.reserve(records.size());
    mMissingTextureSprites.reserve(records.size());

    for (const LevelObjectRecord& record : records)
    {
        if (!record.IsDrawable || record.TextureFilename.empty())
        {
            continue;
        }

        if (record.IsAnimated && !record.AnimationFile.empty())
        {
            const std::string animationPath = mFiles.AssetPath(record.AnimationFile);
            if (std::filesystem::exists(animationPath))
            {
                continue;
            }
        }

        const std::string texturePath = mFiles.AssetPath(record.TextureFilename);
        SpriteRecord sprite = SpriteRecord::FromLevelObjectRecord(record, previewLevelPath, texturePath);
        sprite.TextureExists = std::filesystem::exists(texturePath);
        if (!sprite.TextureExists)
        {
            mMissingTextureSprites.push_back(sprite);
            continue;
        }

        sprite.Texture = mRenderer->LoadTexture(texturePath);
        if (sprite.Texture == 0)
        {
            mMissingTextureSprites.push_back(sprite);
            continue;
        }

        mStaticPreviewSprites.push_back(sprite);
    }

    std::sort(mStaticPreviewSprites.begin(), mStaticPreviewSprites.end(), [](const auto& a, const auto& b) {
        return a.Depth > b.Depth;
    });

    std::sort(mMissingTextureSprites.begin(), mMissingTextureSprites.end(), [](const auto& a, const auto& b) {
        return a.Depth > b.Depth;
    });

    if (!mStaticPreviewSprites.empty())
    {
        float minX = mStaticPreviewSprites.front().WorldPosition.X;
        float maxX = minX;
        float minY = mStaticPreviewSprites.front().WorldPosition.Y;
        float maxY = minY;

        for (const SpriteRecord& sprite : mStaticPreviewSprites)
        {
            const float halfWidth = sprite.Dimensions.X * 0.5f;
            const float halfHeight = sprite.Dimensions.Y * 0.5f;
            minX = std::min(minX, sprite.WorldPosition.X - halfWidth);
            maxX = std::max(maxX, sprite.WorldPosition.X + halfWidth);
            minY = std::min(minY, sprite.WorldPosition.Y - halfHeight);
            maxY = std::max(maxY, sprite.WorldPosition.Y + halfHeight);
        }

        mPreviewCameraX = (minX + maxX) * 0.5f;
        mPreviewCameraY = (minY + maxY) * 0.5f;

        const float contentWidth = std::max(1.0f, maxX - minX);
        const float contentHeight = std::max(1.0f, maxY - minY);
        const float fitZoomX = 1280.0f / (contentWidth * 1.15f);
        const float fitZoomY = 720.0f / (contentHeight * 1.15f);
        const float fitZoom = std::min(fitZoomX, fitZoomY);
        mPreviewZoom = std::clamp(std::min(mPreviewZoom, fitZoom), 0.05f, 3.0f);

        if (!mPrintedPreviewDebug)
        {
            std::cout << "preview bounds: min=(" << minX << "," << minY
                      << ") max=(" << maxX << "," << maxY
                      << ") camera=(" << mPreviewCameraX << "," << mPreviewCameraY
                      << ") zoom=" << mPreviewZoom << "\n";

            for (std::size_t i = 0; i < std::min<std::size_t>(5, mStaticPreviewSprites.size()); ++i)
            {
                const SpriteRecord& sprite = mStaticPreviewSprites[i];
                std::cout << "preview sprite " << i
                          << ": type=" << sprite.TypeName
                          << " id=" << sprite.Id
                          << " pos=(" << sprite.WorldPosition.X << "," << sprite.WorldPosition.Y << ")"
                          << " size=(" << sprite.Dimensions.X << "," << sprite.Dimensions.Y << ")"
                          << " depth=" << sprite.Depth
                          << " tex=" << sprite.TexturePath
                          << "\n";
            }

            mPrintedPreviewDebug = true;
        }
    }
}

void MacGameBootstrap::DrawStaticLevelPreview()
{
    auto toScreen = [this](const Vector2& worldPos) {
        Vector2 screen;
        screen.X = 640.0f + (worldPos.X - mPreviewCameraX) * mPreviewZoom;
        screen.Y = 360.0f - (worldPos.Y - mPreviewCameraY) * mPreviewZoom;
        return screen;
    };

    auto isVisible = [](float x, float y, float w, float h) {
        return x + w * 0.5f >= 0.0f &&
            x - w * 0.5f <= 1280.0f &&
            y + h * 0.5f >= 0.0f &&
            y - h * 0.5f <= 720.0f;
    };

    for (const SpriteRecord& sprite : mStaticPreviewSprites)
    {
        const Vector2 screenPos = toScreen(sprite.WorldPosition);
        const float width = sprite.Dimensions.X * mPreviewZoom;
        const float height = sprite.Dimensions.Y * mPreviewZoom;
        if (!isVisible(screenPos.X, screenPos.Y, width, height))
        {
            continue;
        }

        SpriteDrawCommand command;
        command.Texture = sprite.Texture;
        command.X = screenPos.X;
        command.Y = screenPos.Y;
        command.Width = width;
        command.Height = height;
        command.RotationRadians = -sprite.RotationRadians;
        command.Alpha = sprite.Alpha;
        command.Depth = static_cast<float>(sprite.Depth);
        command.Flip = sprite.Flip;
        mRenderer->DrawSprite(command);

        RectDrawCommand debugOutline;
        debugOutline.X = screenPos.X;
        debugOutline.Y = screenPos.Y;
        debugOutline.Width = width;
        debugOutline.Height = height;
        debugOutline.Fill = Color{0.0f, 0.0f, 0.0f, 0.0f};
        debugOutline.Outline = Color{0.0f, 1.0f, 1.0f, 0.9f};
        mRenderer->DrawRect(debugOutline);
    }

    for (const SpriteRecord& sprite : mMissingTextureSprites)
    {
        const Vector2 screenPos = toScreen(sprite.WorldPosition);
        const float width = std::max(8.0f, sprite.Dimensions.X * mPreviewZoom);
        const float height = std::max(8.0f, sprite.Dimensions.Y * mPreviewZoom);
        if (!isVisible(screenPos.X, screenPos.Y, width, height))
        {
            continue;
        }

        RectDrawCommand command;
        command.X = screenPos.X;
        command.Y = screenPos.Y;
        command.Width = width;
        command.Height = height;
        command.RotationRadians = -sprite.RotationRadians;
        command.Fill = Color{1.0f, 0.0f, 0.45f, 0.22f};
        command.Outline = Color{1.0f, 0.85f, 0.1f, 0.9f};
        mRenderer->DrawRect(command);
    }
}

void MacGameBootstrap::UpdatePreviewCamera(float delta)
{
    const float panSpeed = 620.0f / std::max(0.1f, mPreviewZoom);
    if (mInput.IsKeyDown(SDL_SCANCODE_LEFT) || mInput.IsKeyDown(SDL_SCANCODE_A))
    {
        mPreviewCameraX -= panSpeed * delta;
    }
    if (mInput.IsKeyDown(SDL_SCANCODE_RIGHT) || mInput.IsKeyDown(SDL_SCANCODE_D))
    {
        mPreviewCameraX += panSpeed * delta;
    }
    if (mInput.IsKeyDown(SDL_SCANCODE_UP) || mInput.IsKeyDown(SDL_SCANCODE_W))
    {
        mPreviewCameraY += panSpeed * delta;
    }
    if (mInput.IsKeyDown(SDL_SCANCODE_DOWN) || mInput.IsKeyDown(SDL_SCANCODE_S))
    {
        mPreviewCameraY -= panSpeed * delta;
    }

    if (mInput.IsKeyDown(SDL_SCANCODE_EQUALS) || mInput.IsKeyDown(SDL_SCANCODE_KP_PLUS))
    {
        mPreviewZoom += 1.25f * delta;
    }
    if (mInput.IsKeyDown(SDL_SCANCODE_MINUS) || mInput.IsKeyDown(SDL_SCANCODE_KP_MINUS))
    {
        mPreviewZoom -= 1.25f * delta;
    }
    mPreviewZoom = std::clamp(mPreviewZoom, 0.08f, 3.0f);
}

void MacGameBootstrap::WriteTextureReport() const
{
    const std::filesystem::path reportPath = mRepoRoot / "build" / "mac" / "texture-report.txt";
    std::filesystem::create_directories(reportPath.parent_path());

    std::ofstream report(reportPath);
    report << "Death Leak texture validation report\n";
    report << "drawable_records=" << mValidatedDrawableCount << "\n";
    report << "textures_found=" << mValidatedTextureFoundCount << "\n";
    report << "textures_missing=" << mValidatedTextureMissingCount << "\n\n";
    report << "animations_found=" << mValidatedAnimationFoundCount << "\n";
    report << "animations_missing=" << mValidatedAnimationMissingCount << "\n\n";
    for (const std::string& line : mTextureReportLines)
    {
        report << line << "\n";
    }
}

void MacGameBootstrap::RunDataSmokeChecks()
{
    Vector2 vectorCheck(3.0f, 4.0f);
    DataValue dataValueCheck("12.3456789");
    SineWave sineWave;
    sineWave.Initialise(0.0f, 10.0f, 1.0f, 100.0f, 50.0f, 5.0f);
    sineWave.Update(0.5f);

    const std::string settingsPath = mFiles.AssetPath("XmlFiles\\settings.xml");
    Settings::GetInstance()->ReadSettingsFile(settingsPath);

    const std::string savePath = mFiles.SavePath("mac_smoke_save.xml");
    std::filesystem::create_directories(std::filesystem::path(savePath).parent_path());
    SaveManager::GetInstance()->ReadSaveFile(savePath);
    SaveManager::GetInstance()->SetNumTimesGameCompleted(1);
    FeatureUnlockManager::GetInstance()->SetFeatureUnlocked(FeatureUnlockManager::kRoll);
    SaveManager::GetInstance()->WriteSaveFile(savePath);
    mAudio.PlaySound(mFiles.AssetPath("Media\\Audio\\dummysound.wav"), false);
    mSteam.UnlockAchievement("ACH_ROLL_UNLOCK");

    XmlDocument xmlDocument;
    const bool loadedSettings = xmlDocument.Load(settingsPath);
    bool musicEnabled = false;
    if (loadedSettings)
    {
        musicEnabled = XmlUtilities::ReadAttributeAsBool(xmlDocument.GetRoot(), "audio", "music_enabled");
    }

    XmlDocument levelDocument;
    LevelProperties levelProperties;
    const bool loadedLevel = levelDocument.Load(mFiles.AssetPath("XmlFiles\\levels\\dojo.xml"));
    if (loadedLevel)
    {
        levelProperties.XmlRead(levelDocument.GetRoot()->FirstChildElement("LevelProperties"));
    }
    const auto levelObjects = LevelObjectRecord::ReadLevelObjects(loadedLevel ? levelDocument.GetRoot() : nullptr);

    std::size_t scannedLevelCount = 0;
    std::size_t scannedObjectCount = 0;
    mValidatedDrawableCount = 0;
    mValidatedTextureFoundCount = 0;
    mValidatedTextureMissingCount = 0;
    mValidatedAnimationFoundCount = 0;
    mValidatedAnimationMissingCount = 0;
    mTextureReportLines.clear();
    std::set<std::string> uniqueMissingTextures;
    std::set<std::string> uniqueMissingAnimations;
    const std::filesystem::path levelsRoot = mFiles.AssetPath("XmlFiles\\levels");
    for (const auto& entry : std::filesystem::directory_iterator(levelsRoot))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".xml")
        {
            continue;
        }

        XmlDocument currentLevelDocument;
        if (!currentLevelDocument.Load(entry.path().string()))
        {
            continue;
        }

        ++scannedLevelCount;
        const auto currentRecords = LevelObjectRecord::ReadLevelObjects(currentLevelDocument.GetRoot());
        scannedObjectCount += currentRecords.size();

        for (const LevelObjectRecord& record : currentRecords)
        {
            if (!record.IsDrawable || record.TextureFilename.empty())
            {
                continue;
            }

            ++mValidatedDrawableCount;
            if (record.IsAnimated && !record.AnimationFile.empty())
            {
                const std::string resolvedAnimation = mFiles.AssetPath(record.AnimationFile);
                if (std::filesystem::exists(resolvedAnimation))
                {
                    ++mValidatedAnimationFoundCount;
                }
                else
                {
                    ++mValidatedAnimationMissingCount;
                    uniqueMissingAnimations.insert(record.AnimationFile);
                    mTextureReportLines.push_back(
                        entry.path().filename().string() +
                        " id=" + std::to_string(record.Id) +
                        " type=" + record.TypeName +
                        " missing_animation=" + record.AnimationFile);
                }
                continue;
            }

            const std::string resolvedTexture = mFiles.AssetPath(record.TextureFilename);
            if (std::filesystem::exists(resolvedTexture))
            {
                ++mValidatedTextureFoundCount;
            }
            else
            {
                ++mValidatedTextureMissingCount;
                uniqueMissingTextures.insert(record.TextureFilename);
                mTextureReportLines.push_back(
                    entry.path().filename().string() +
                    " id=" + std::to_string(record.Id) +
                    " type=" + record.TypeName +
                    " texture=" + record.TextureFilename);
            }
        }
    }
    WriteTextureReport();

    Animation animation(mFiles.AssetPath("XmlFiles\\animation\\player_anim_default.xml").c_str());
    const auto bodySequences = animation.GetSequenceNamesForBodyPart("body");

    MaterialManager::Instance()->Initialise(mFiles.AssetPath("XmlFiles\\materials.xml").c_str());
    Material* softwood = MaterialManager::Instance()->GetMaterial("softwood");

    StringManager::GetInstance()->LoadStringsFile(mFiles.AssetPath("XmlFiles\\strings.xml").c_str());

    std::cout << "portable core sample: length=" << vectorCheck.Length()
              << ", value=" << dataValueCheck.asFloat() << "\n";
    std::cout << "sine sample: x=" << sineWave.GetValueX()
              << ", y=" << sineWave.GetValueY()
              << "\n";
    std::cout << "player level sample: percent="
              << PlayerLevelManager::GetInstance()->GetPercentTowardsLevelUp(1, 125)
              << "\n";
    std::cout << "save sample value: completed="
              << SaveManager::GetInstance()->GetNumTimesGameCompleted() << "\n";
    std::cout << "feature sample: roll="
              << (FeatureUnlockManager::GetInstance()->IsFeatureUnlocked(FeatureUnlockManager::kRoll) ? "true" : "false")
              << "\n";
    std::cout << "audio sample: volume=" << mAudio.MasterVolume()
              << ", playCalls=" << mAudio.PlayCallCount()
              << "\n";
    std::cout << "steam sample: appId=" << mSteam.AppId()
              << ", achievements=" << mSteam.UnlockedAchievements().size()
              << "\n";
    std::cout << "level sample: loaded=" << (loadedLevel ? "true" : "false")
              << ", zoom=" << levelProperties.GetZoomInPercent()
              << ", followX=" << (levelProperties.ShouldFollowX() ? "true" : "false")
              << ", objects=" << levelObjects.size()
              << "\n";
    std::cout << "level scan sample: levels=" << scannedLevelCount
              << ", objects=" << scannedObjectCount
              << "\n";
    std::cout << "texture validation sample: drawable=" << mValidatedDrawableCount
              << ", found=" << mValidatedTextureFoundCount
              << ", missing=" << mValidatedTextureMissingCount
              << ", uniqueMissing=" << uniqueMissingTextures.size()
              << ", animationsFound=" << mValidatedAnimationFoundCount
              << ", animationsMissing=" << mValidatedAnimationMissingCount
              << ", uniqueMissingAnimations=" << uniqueMissingAnimations.size()
              << "\n";
    if (!levelObjects.empty())
    {
        std::cout << "level object sample: type=" << levelObjects.front().TypeName
                  << ", id=" << levelObjects.front().Id
                  << ", depth=" << levelObjects.front().DepthLayer
                  << "\n";
    }
    std::cout << "static preview sample: sprites=" << mStaticPreviewSprites.size()
              << ", placeholders=" << mMissingTextureSprites.size()
              << ", camera=(" << mPreviewCameraX << "," << mPreviewCameraY << ")"
              << ", zoom=" << mPreviewZoom
              << "\n";
    std::cout << "animation sample: bodySequences=" << bodySequences.size()
              << ", currentFrame=" << animation.CurrentFrame("body")
              << "\n";
    std::cout << "material sample: softwood="
              << (softwood != nullptr ? softwood->GetMaterialName() : "missing")
              << ", vibrate=" << (softwood != nullptr && softwood->ShouldVibrate() ? "true" : "false")
              << "\n";
    std::cout << "string sample: game_saved="
              << StringManager::GetInstance()->GetLocalisedString("game_saved")
              << "\n";
    std::cout << "xml sample: settings loaded=" << (loadedSettings ? "true" : "false")
              << ", music=" << (musicEnabled ? "true" : "false") << "\n";
    std::cout << "asset sample: " << mFiles.AssetPath("Media\\UI\\title.png") << "\n";
    std::cout << "save sample: " << savePath << "\n";
}

double MacGameBootstrap::ChronoTimeSource::Seconds() const
{
    const auto now = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed = now - mStart;
    return elapsed.count();
}

void MacGameBootstrap::ChronoTimeSource::SleepUntilNextFrame(double targetDeltaSeconds)
{
    std::this_thread::sleep_for(std::chrono::duration<double>(targetDeltaSeconds));
}

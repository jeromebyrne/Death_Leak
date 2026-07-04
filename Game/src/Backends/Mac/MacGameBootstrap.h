#ifndef DEATHLEAK_BACKENDS_MAC_MACGAMEBOOTSTRAP_H
#define DEATHLEAK_BACKENDS_MAC_MACGAMEBOOTSTRAP_H

#include "Backends/Mac/MacFileSystem.h"
#include "Backends/Mac/MacNullAudioSystem.h"
#include "Backends/Mac/MacNullSteamService.h"
#include "Backends/Mac/MacSdlInputSystem.h"
#include "Backends/Mac/MacSdlPlatformApp.h"
#include "Backends/Mac/MacSdlRenderer.h"
#include "Engine/Platform/ITimeSource.h"

#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

struct SpriteRecord;

class MacGameBootstrap
{
public:
    explicit MacGameBootstrap(std::filesystem::path repoRoot);
    ~MacGameBootstrap();

    int RunSmoke();

private:
    void InitialiseLegacyData();
    void RunFrameLoop();
    void RunDataSmokeChecks();
    void BuildStaticLevelPreview();
    void DrawStaticLevelPreview();
    void UpdatePreviewCamera(float delta);
    void WriteTextureReport() const;

    class ChronoTimeSource final : public ITimeSource
    {
    public:
        double Seconds() const override;
        void SleepUntilNextFrame(double targetDeltaSeconds) override;

    private:
        std::chrono::steady_clock::time_point mStart = std::chrono::steady_clock::now();
    };

    MacFileSystem mFiles;
    MacSdlPlatformApp mApp;
    MacSdlInputSystem mInput;
    MacNullAudioSystem mAudio;
    MacNullSteamService mSteam;
    std::unique_ptr<MacSdlRenderer> mRenderer;
    ChronoTimeSource mTime;
    std::filesystem::path mRepoRoot;
    std::vector<SpriteRecord> mStaticPreviewSprites;
    std::vector<SpriteRecord> mMissingTextureSprites;
    float mPreviewCameraX = 0.0f;
    float mPreviewCameraY = 0.0f;
    float mPreviewZoom = 1.0f;
    bool mPrintedPreviewDebug = false;
    std::size_t mValidatedDrawableCount = 0;
    std::size_t mValidatedTextureFoundCount = 0;
    std::size_t mValidatedTextureMissingCount = 0;
    std::size_t mValidatedAnimationFoundCount = 0;
    std::size_t mValidatedAnimationMissingCount = 0;
    std::vector<std::string> mTextureReportLines;
};

#endif

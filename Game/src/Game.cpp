#include "precompiled.h"
#include "Game.h"
#include "drawableobject.h"
#include "sprite.h"
#include "collisionManager.h"
#include "EffectParticleSpray.h"
#include "ParticleEmitterManager.h"
#include "Projectile.h"
#include "AudioManager.h"
#include "MaterialManager.h"
#include "DXWindow.h"
#include "UIManager.h"
#include "screenalignedtexture.h"
#include <time.h>
#include "Graphics.h"
#include "EffectBasic.h"
#include "EffectLightTextureVertexWobble.h"
#include "EffectReflection.h"
#include "player.h"
#include "particleSpray.h"
#include "effectbloodparticlespray.h"
#include "effectsepia.h"
#include "effectmonochrome.h"
#include "effectmonochromered.h"
#include "effectLightTextureBump.h"
#include "EffectNoise.h"
#include "LevelEditor.h"
#include "GamePad.h"
#include "WeatherManager.h"
#include "EffectLightTexturePixelWobble.h"
#include "EffectFoliageSway.h"
#include "Settings.h"
#include "SaveManager.h"
#include "DataValue.h"
#include "PlayerLevelManager.h"
#include "FeatureUnlockManager.h"
#include "Engine/Platform/IFileSystem.h"
#include "Core/XmlDocument.h"
#include <cmath>
#include "GameObjects/LevelObjectRecord.h"
#include "GameObjects/SpriteRecord.h"
#include "LevelProperties.h"
#include "UITextModal.h"
#include "UIUpgradeModal.h"
#include "NinjaSpawner.h"
#include "ParticleEmitterManager.h"
#include "TextureManager.h"

#if defined(DEATHLEAK_PLATFORM_MAC) && DEATHLEAK_PLATFORM_MAC
#include "Backends/Mac/MacSdlPlatformApp.h"
#include "Backends/Mac/MacSdlRenderer.h"
#include "GameObjects/GameObjectManager.h"
#include <algorithm>
#include <filesystem>
#include <cstdlib>
#include <memory>
#include <SDL.h>
#endif

Game * Game::mInstance = nullptr;

static const float kPixelWobbleReverseDelay = 10.0f;
static const float kPauseDamageEffectDelay = 0.04f;
static const float kPauseDamageEffectLongerDelay = 0.2f;

bool Game::mPaused = false;
bool Game::mLevelEditMode = false;
bool Game::mIsDisplayingTextModal = false;

Vector2 Game::mGameScale = Vector2(1.0f, 1.0f);

#if defined(DEATHLEAK_PLATFORM_MAC) && DEATHLEAK_PLATFORM_MAC
namespace
{
struct MacMenuButton
{
    std::string Name;
    std::string Action;
    std::string TextureNormalPath;
    std::string TextureFocusPath;
    std::string TextureClickPath;
    float UiX = 0.0f;
    float UiY = 0.0f;
    float UiWidth = 0.0f;
    float UiHeight = 0.0f;
    TextureHandle NormalTexture = 0;
    TextureHandle FocusTexture = 0;
    TextureHandle ClickTexture = 0;
};

struct MacMenuSprite
{
    std::string TexturePath;
    float UiX = 0.0f;
    float UiY = 0.0f;
    float UiWidth = 0.0f;
    float UiHeight = 0.0f;
    float Alpha = 1.0f;
    TextureHandle Texture = 0;
};

struct MacMenuState
{
    bool Loaded = false;
    std::vector<MacMenuSprite> Sprites;
    std::vector<MacMenuButton> Buttons;
    bool PreviousLeftDown = false;
};

struct MacPreviewState
{
    std::string LoadedLevelFile;
    std::vector<SpriteRecord> StaticSprites;
    std::vector<SpriteRecord> MissingTextureSprites;
    float FitCameraX = 0.0f;
    float FitCameraY = 0.0f;
    float FitZoom = 1.0f;
    bool HasFitCamera = false;
};

MacPreviewState& PreviewState()
{
    static MacPreviewState state;
    return state;
}

MacMenuState& MenuState()
{
    static MacMenuState state;
    return state;
}

float UiLeftToScreenX(float uiX, int screenWidth)
{
    return (uiX + 960.0f) * (static_cast<float>(screenWidth) / 1920.0f);
}

float UiBottomToScreenY(float uiY, int screenHeight)
{
    return (540.0f - uiY) * (static_cast<float>(screenHeight) / 1080.0f);
}

void LoadMacMainMenuIfNeeded(const IFileSystem* fileSystem, MacSdlRenderer* renderer)
{
    if (fileSystem == nullptr || renderer == nullptr)
    {
        return;
    }

    MacMenuState& state = MenuState();
    if (state.Loaded)
    {
        return;
    }

    state.Loaded = true;
    state.Sprites.clear();
    state.Buttons.clear();

    const auto addSprite = [&](const char* texturePath, float x, float y, float w, float h, float alpha) {
        MacMenuSprite sprite;
        sprite.TexturePath = fileSystem->AssetPath(texturePath);
        sprite.UiX = x;
        sprite.UiY = y;
        sprite.UiWidth = w;
        sprite.UiHeight = h;
        sprite.Alpha = alpha;
        sprite.Texture = renderer->LoadTexture(sprite.TexturePath);
        state.Sprites.push_back(sprite);
    };

    addSprite("Media\\UI\\main_menu_2.png", -970.0f, -540.0f, 1939.2f, 1090.8f, 1.0f);
    addSprite("Media\\UI\\main_menu_blur.png", -990.0f, -540.0f, 1969.2f, 1115.8f, 0.45f);
    addSprite("Media\\UI\\title_blurred.png", -750.0f, 300.0f, 753.0f, 194.0f, 0.5f);
    addSprite("Media\\UI\\title.png", -754.0f, 307.0f, 753.0f, 194.0f, 1.0f);
    addSprite("Media\\UI\\frame.png", -960.0f, -540.0f, 1920.0f, 1080.0f, 1.0f);
    addSprite("Media\\UI\\noise_layer.png", -960.0f, -540.0f, 1920.0f, 1080.0f, 0.10f);

    const auto addButton = [&](const char* name, const char* action, const char* normal, const char* focus, const char* click, float x, float y, float w, float h) {
        MacMenuButton button;
        button.Name = name;
        button.Action = action;
        button.TextureNormalPath = fileSystem->AssetPath(normal);
        button.TextureFocusPath = fileSystem->AssetPath(focus);
        button.TextureClickPath = fileSystem->AssetPath(click);
        button.UiX = x;
        button.UiY = y;
        button.UiWidth = w;
        button.UiHeight = h;
        button.NormalTexture = renderer->LoadTexture(button.TextureNormalPath);
        button.FocusTexture = renderer->LoadTexture(button.TextureFocusPath);
        button.ClickTexture = renderer->LoadTexture(button.TextureClickPath);
        state.Buttons.push_back(button);
    };

    addButton("new_game", "loadlevel", "Media\\UI\\buttons\\play1.png", "Media\\UI\\buttons\\play2.png", "Media\\UI\\buttons\\play2.png", -435.0f, -100.0f, 160.0f, 160.0f);
    addButton("options", "options", "Media\\UI\\buttons\\settings1.png", "Media\\UI\\buttons\\settings2.png", "Media\\UI\\buttons\\settings2.png", -410.0f, -275.0f, 100.0f, 100.0f);
    addButton("quit", "quit", "Media\\UI\\buttons\\back1.png", "Media\\UI\\buttons\\back2.png", "Media\\UI\\buttons\\back2.png", -395.0f, -435.0f, 90.0f, 90.0f);
}

bool HitTestButton(const MacMenuButton& button, float mouseUiX, float mouseUiY)
{
    return mouseUiX >= button.UiX &&
        mouseUiX <= button.UiX + button.UiWidth &&
        mouseUiY >= button.UiY &&
        mouseUiY <= button.UiY + button.UiHeight;
}

void DrawMacMainMenu(MacSdlRenderer* renderer, int screenWidth, int screenHeight)
{
    MacMenuState& state = MenuState();
    for (const MacMenuSprite& sprite : state.Sprites)
    {
        if (sprite.Texture == 0)
        {
            continue;
        }

        const float left = UiLeftToScreenX(sprite.UiX, screenWidth);
        const float bottom = UiBottomToScreenY(sprite.UiY, screenHeight);
        const float width = sprite.UiWidth * (static_cast<float>(screenWidth) / 1920.0f);
        const float height = sprite.UiHeight * (static_cast<float>(screenHeight) / 1080.0f);

        SpriteDrawCommand command;
        command.Texture = sprite.Texture;
        command.X = left + width * 0.5f;
        command.Y = bottom - height * 0.5f;
        command.Width = width;
        command.Height = height;
        command.Alpha = sprite.Alpha;
        renderer->DrawSprite(command);
    }

    int mouseX = 0;
    int mouseY = 0;
    const Uint32 mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
    const bool leftDown = (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    const float mouseUiX = (static_cast<float>(mouseX) / static_cast<float>(screenWidth)) * 1920.0f - 960.0f;
    const float mouseUiY = 540.0f - (static_cast<float>(mouseY) / static_cast<float>(screenHeight)) * 1080.0f;

    for (const MacMenuButton& button : state.Buttons)
    {
        const bool hovered = HitTestButton(button, mouseUiX, mouseUiY);
        const TextureHandle texture = leftDown && hovered ? button.ClickTexture : hovered ? button.FocusTexture : button.NormalTexture;
        if (texture == 0)
        {
            continue;
        }

        const float left = UiLeftToScreenX(button.UiX, screenWidth);
        const float bottom = UiBottomToScreenY(button.UiY, screenHeight);
        const float width = button.UiWidth * (static_cast<float>(screenWidth) / 1920.0f);
        const float height = button.UiHeight * (static_cast<float>(screenHeight) / 1080.0f);

        SpriteDrawCommand command;
        command.Texture = texture;
        command.X = left + width * 0.5f;
        command.Y = bottom - height * 0.5f;
        command.Width = width;
        command.Height = height;
        command.Alpha = 1.0f;
        renderer->DrawSprite(command);
    }

    state.PreviousLeftDown = leftDown;
}

void HandleMacMainMenuInput(const IFileSystem* fileSystem)
{
    if (fileSystem == nullptr)
    {
        return;
    }

    static bool sAutoLoadedLevel = false;
    const char* autoStartLevel = std::getenv("DEATHLEAK_MAC_AUTOSTART_LEVEL");
    if (!sAutoLoadedLevel && autoStartLevel != nullptr && autoStartLevel[0] == '1')
    {
        sAutoLoadedLevel = true;
        GameObjectManager::Instance()->DeleteGameObjects();
        GameObjectManager::Instance()->LoadObjectsFromFile(fileSystem->AssetPath("XmlFiles\\levels\\grass_exploration_3.xml"));
        return;
    }

    if (GameObjectManager::Instance()->IsLevelLoaded())
    {
        return;
    }

    const Uint8* keyboard = SDL_GetKeyboardState(nullptr);
    if (keyboard != nullptr)
    {
        if (keyboard[SDL_SCANCODE_RETURN] || keyboard[SDL_SCANCODE_KP_ENTER])
        {
            GameObjectManager::Instance()->DeleteGameObjects();
            GameObjectManager::Instance()->LoadObjectsFromFile(fileSystem->AssetPath("XmlFiles\\levels\\grass_exploration_3.xml"));
            return;
        }

        if (keyboard[SDL_SCANCODE_ESCAPE])
        {
            SDL_Event quitEvent;
            SDL_zero(quitEvent);
            quitEvent.type = SDL_QUIT;
            SDL_PushEvent(&quitEvent);
            return;
        }
    }

    int mouseX = 0;
    int mouseY = 0;
    const Uint32 mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
    const bool leftDown = (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    MacMenuState& state = MenuState();
    if (!state.PreviousLeftDown && leftDown)
    {
        Graphics* graphics = Graphics::GetInstance();
        const float screenWidth = graphics != nullptr ? static_cast<float>(graphics->BackBufferWidth()) : 1280.0f;
        const float screenHeight = graphics != nullptr ? static_cast<float>(graphics->BackBufferHeight()) : 720.0f;
        const float mouseUiX = (static_cast<float>(mouseX) / screenWidth) * 1920.0f - 960.0f;
        const float mouseUiY = 540.0f - (static_cast<float>(mouseY) / screenHeight) * 1080.0f;
        for (const MacMenuButton& button : state.Buttons)
        {
            if (!HitTestButton(button, mouseUiX, mouseUiY))
            {
                continue;
            }

            if (button.Action == "loadlevel")
            {
                GameObjectManager::Instance()->DeleteGameObjects();
                GameObjectManager::Instance()->LoadObjectsFromFile(fileSystem->AssetPath("XmlFiles\\levels\\grass_exploration_3.xml"));
            }
            else if (button.Action == "quit")
            {
                SDL_Event quitEvent;
                SDL_zero(quitEvent);
                quitEvent.type = SDL_QUIT;
                SDL_PushEvent(&quitEvent);
            }

            break;
        }
    }

    state.PreviousLeftDown = leftDown;
}

void LoadPreviewLevelIfNeeded(const IFileSystem* fileSystem, MacSdlRenderer* renderer)
{
    if (fileSystem == nullptr || renderer == nullptr)
    {
        return;
    }

    MacPreviewState& state = PreviewState();
    const std::string levelFile = GameObjectManager::Instance()->GetCurrentLevelFile();
    if (levelFile.empty())
    {
        return;
    }

    if (state.LoadedLevelFile == levelFile && !state.StaticSprites.empty())
    {
        return;
    }

    state.LoadedLevelFile = levelFile;
    state.StaticSprites.clear();
    state.MissingTextureSprites.clear();
    state.HasFitCamera = false;
    XmlDocument levelDocument;
    if (!levelDocument.Load(levelFile))
    {
        return;
    }

    TiXmlElement* root = levelDocument.GetRoot();
    if (root == nullptr)
    {
        return;
    }

    LevelProperties previewLevelProperties;
    if (TiXmlElement* properties = root->FirstChildElement("LevelProperties"))
    {
        previewLevelProperties.XmlRead(properties);
    }

    const auto records = LevelObjectRecord::ReadLevelObjects(root);
    state.StaticSprites.reserve(records.size());
    state.MissingTextureSprites.reserve(records.size());

    for (const LevelObjectRecord& record : records)
    {
        if (!record.IsDrawable || record.TextureFilename.empty())
        {
            continue;
        }

        if (record.IsAnimated && !record.AnimationFile.empty())
        {
            const std::string animationPath = fileSystem->AssetPath(record.AnimationFile);
            if (std::filesystem::exists(animationPath))
            {
                continue;
            }
        }

        const std::string texturePath = fileSystem->AssetPath(record.TextureFilename);
        SpriteRecord sprite = SpriteRecord::FromLevelObjectRecord(record, levelFile, texturePath);
        sprite.TextureExists = std::filesystem::exists(texturePath);
        if (!sprite.TextureExists)
        {
            state.MissingTextureSprites.push_back(sprite);
            continue;
        }

        sprite.Texture = renderer->LoadTexture(texturePath);
        if (sprite.Texture == 0)
        {
            state.MissingTextureSprites.push_back(sprite);
            continue;
        }

        state.StaticSprites.push_back(sprite);
    }

    std::sort(state.StaticSprites.begin(), state.StaticSprites.end(), [](const auto& a, const auto& b) {
        return a.Depth > b.Depth;
    });
    std::sort(state.MissingTextureSprites.begin(), state.MissingTextureSprites.end(), [](const auto& a, const auto& b) {
        return a.Depth > b.Depth;
    });

    if (!state.StaticSprites.empty())
    {
        float minX = state.StaticSprites.front().WorldPosition.X;
        float maxX = minX;
        float minY = state.StaticSprites.front().WorldPosition.Y;
        float maxY = minY;

        for (const SpriteRecord& sprite : state.StaticSprites)
        {
            const float halfWidth = sprite.Dimensions.X * 0.5f;
            const float halfHeight = sprite.Dimensions.Y * 0.5f;
            minX = std::min(minX, sprite.WorldPosition.X - halfWidth);
            maxX = std::max(maxX, sprite.WorldPosition.X + halfWidth);
            minY = std::min(minY, sprite.WorldPosition.Y - halfHeight);
            maxY = std::max(maxY, sprite.WorldPosition.Y + halfHeight);
        }

        const float contentWidth = std::max(1.0f, maxX - minX);
        const float contentHeight = std::max(1.0f, maxY - minY);
        state.FitCameraX = (minX + maxX) * 0.5f;
        state.FitCameraY = (minY + maxY) * 0.5f;
        state.FitZoom = std::clamp(std::min(1280.0f / (contentWidth * 1.15f), 720.0f / (contentHeight * 1.15f)), 0.05f, 3.0f);
        state.HasFitCamera = true;
    }

    (void)previewLevelProperties;
}

void DrawPreviewLevel(MacSdlRenderer* renderer, const MacPreviewState& state, int screenWidth, int screenHeight, Camera2D* camera)
{
    static bool sPrintedPreviewTrace = false;
    static int sPreviewTraceFrame = 0;
    const float runtimeCameraX = camera != nullptr ? camera->X() : 0.0f;
    const float runtimeCameraY = camera != nullptr ? camera->Y() : 0.0f;
    const float runtimeZoom = camera != nullptr ? camera->GetZoomLevel() : 1.0f;
    int visibleSpriteCount = 0;
    auto toScreen = [&](const Vector2& worldPos) {
        Vector2 screen;
        screen.X = screenWidth * 0.5f + (worldPos.X - runtimeCameraX) * runtimeZoom;
        screen.Y = screenHeight * 0.5f - (worldPos.Y - runtimeCameraY) * runtimeZoom;
        return screen;
    };

    auto isVisible = [&](float x, float y, float w, float h) {
        return x + w * 0.5f >= 0.0f &&
            x - w * 0.5f <= static_cast<float>(screenWidth) &&
            y + h * 0.5f >= 0.0f &&
            y - h * 0.5f <= static_cast<float>(screenHeight);
    };

    for (const SpriteRecord& sprite : state.StaticSprites)
    {
        const Vector2 screenPos = toScreen(sprite.WorldPosition);
        const float width = sprite.Dimensions.X * runtimeZoom;
        const float height = sprite.Dimensions.Y * runtimeZoom;
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
        renderer->DrawSprite(command);
        ++visibleSpriteCount;
    }

    for (const SpriteRecord& sprite : state.MissingTextureSprites)
    {
        const Vector2 screenPos = toScreen(sprite.WorldPosition);
        const float width = std::max(8.0f, sprite.Dimensions.X * runtimeZoom);
        const float height = std::max(8.0f, sprite.Dimensions.Y * runtimeZoom);
        if (!isVisible(screenPos.X, screenPos.Y, width, height))
        {
            continue;
        }

        RectDrawCommand command;
        command.X = screenPos.X;
        command.Y = screenPos.Y;
        command.Width = width;
        command.Height = height;
        command.Fill = Color{1.0f, 0.0f, 0.45f, 0.22f};
        command.Outline = Color{1.0f, 0.85f, 0.1f, 0.9f};
        renderer->DrawRect(command);
        ++visibleSpriteCount;
    }

    if (!sPrintedPreviewTrace || sPreviewTraceFrame < 5 || (sPreviewTraceFrame % 60) == 0)
    {
        LOG_INFO("mac draw trace: level=%s camera=(%f,%f) zoom=%f visible=%d static=%zu missing=%zu",
            state.LoadedLevelFile.c_str(),
            runtimeCameraX,
            runtimeCameraY,
            runtimeZoom,
            visibleSpriteCount,
            state.StaticSprites.size(),
            state.MissingTextureSprites.size());
        sPrintedPreviewTrace = true;
    }

    ++sPreviewTraceFrame;
}

void DrawSepiaOverlay(MacSdlRenderer* renderer, int screenWidth, int screenHeight)
{
    RectDrawCommand sepiaOverlay;
    sepiaOverlay.X = screenWidth * 0.5f;
    sepiaOverlay.Y = screenHeight * 0.5f;
    sepiaOverlay.Width = static_cast<float>(screenWidth);
    sepiaOverlay.Height = static_cast<float>(screenHeight);
    sepiaOverlay.Fill = Color{0.30f, 0.22f, 0.12f, 0.10f};
    sepiaOverlay.Outline = Color{0.0f, 0.0f, 0.0f, 0.0f};
    renderer->DrawRect(sepiaOverlay);
}
}
#endif

Game::Game(Graphics * pGraphics, const IFileSystem* fileSystem) : 
	m_pGraphics(pGraphics),
	mFileSystem(fileSystem),
	m_pCam2d(nullptr),
	m_effectLightTexture(nullptr),
	m_effectLightTextureVertexWobble(nullptr),
	m_effectParticleSpray(nullptr),
	m_effectBloodParticleSpray(nullptr),
	m_effectLightTextureBump(nullptr),
	m_effectBasic(nullptr),
	mlevelEditor(nullptr),
	m_effectSepia(nullptr),
	m_effectMonochrome(nullptr),
	m_effectMonochromeRed(nullptr),
	m_screenAlignedPostProcTex1(nullptr),
	m_effectNoise(nullptr),
	m_effectPixelWobble(nullptr),
	m_effectFoliageSway(nullptr),
	mLastTimeDamagePauseEffect(0.0f)
{
}

Game::~Game(void)
{
}

void Game::Initialise()
{
	HRESULT hr = S_OK;

	SaveManager::GetInstance()->ReadSaveFile(ResolveSavePath("save.xml"));

	mGOMInstance = GameObjectManager::Instance();
	mUIManagerInstance = UIManager::Instance();

	Settings::GetInstance()->ReadSettingsFile(ResolveSettingsPath("settings.xml"));

	// initialise audio
	AudioManager::Instance()->Initialise();

	m_pCam2d = new Camera2D(m_pGraphics->BackBufferWidth(), m_pGraphics->BackBufferHeight(), -8000, 0, 200);

#if defined(DEATHLEAK_PLATFORM_MAC) && DEATHLEAK_PLATFORM_MAC
	mGOMInstance = GameObjectManager::Instance();
	mUIManagerInstance = UIManager::Instance();
	TextureManager::Instance()->Initialise(m_pGraphics->Device());
	ParticleEmitterManager::Instance()->Initialise(m_pGraphics);
	MaterialManager::Instance()->Initialise(ResolveAssetPath("XmlFiles\\materials.xml").c_str());
	Settings::GetInstance()->ApplySettings();
	CollisionManager::Instance()->Initialise(m_pGraphics->BackBufferWidth() * 4, m_pGraphics->BackBufferHeight() * 5, 8, 4);
	PlayerLevelManager::GetInstance()->Initialise();
	FeatureUnlockManager::GetInstance()->Initialise();
	LoadCachedObjectsForPerformance();
	mGOMInstance->QuitLevel();
	mGameScale.X = (float)m_pGraphics->BackBufferWidth() / 1920.f;
	mGameScale.Y = (float)m_pGraphics->BackBufferHeight() / 1080.f;
	return;
#else
	// initialise the texture manager
	TextureManager::Instance()->Initialise(m_pGraphics->Device());
	
	// initialise our shaders
	EffectManager::Instance()->Initialise(m_pGraphics);

	// update our effect pointers
	m_effectLightTexture = static_cast<EffectLightTexture*>(EffectManager::Instance()->GetEffect("effectlighttexture"));
	m_effectLightTextureVertexWobble = static_cast<EffectLightTextureVertexWobble*>(EffectManager::Instance()->GetEffect("effectlighttexturevertexwobble"));
	m_effectParticleSpray = static_cast<EffectParticleSpray*>(EffectManager::Instance()->GetEffect("effectparticlespray"));
	m_effectBloodParticleSpray = static_cast<EffectBloodParticleSpray*>(EffectManager::Instance()->GetEffect("effectbloodparticlespray"));
	m_effectBasic = static_cast<EffectBasic*>(EffectManager::Instance()->GetEffect("effectbasic"));
#if _DEBUG
	mlevelEditor = new LevelEditor();
#endif
	m_effectSepia = static_cast<EffectSepia*>(EffectManager::Instance()->GetEffect("effectsepia"));
	m_effectMonochrome = static_cast<EffectMonochrome*>(EffectManager::Instance()->GetEffect("effectmonochrome"));
	m_effectMonochromeRed = static_cast<EffectMonochromeRed*>(EffectManager::Instance()->GetEffect("effectmonochromered"));
	m_effectLightTextureBump = static_cast<EffectLightTextureBump*>(EffectManager::Instance()->GetEffect("effectlighttexturebump"));
	m_effectNoise = static_cast<EffectNoise*>(EffectManager::Instance()->GetEffect("effectnoise"));
	m_effectPixelWobble = static_cast<EffectLightTexturePixelWobble*>(EffectManager::Instance()->GetEffect("effectpixelwobble"));
	m_effectFoliageSway = static_cast<EffectFoliageSway*>(EffectManager::Instance()->GetEffect("effectfoliagesway"));

	m_effectNoise->SetSeed(98765);
	m_effectLightTextureVertexWobble->SetWobbleIntensity(30.0f);

	// initialise Materials
	MaterialManager::Instance()->Initialise(ResolveAssetPath("XmlFiles\\materials.xml").c_str());

	Settings::GetInstance()->ApplySettings();

	// initialise the collision manager, set to 1280 * 4 and 720 * 5 as default, is reset in gameobjectmanager load function
	CollisionManager::Instance()->Initialise(m_pGraphics->BackBufferWidth() * 4, m_pGraphics->BackBufferHeight() * 5, 8, 4);

	// initialise the particle manager
	ParticleEmitterManager::Instance()->Initialise(m_pGraphics);

	// initialise the UI
	mUIManagerInstance->XmlRead(ResolveAssetPath("XmlFiles\\UI\\UI.xml").c_str()); // read in all of the UI components
	mUIManagerInstance->LoadContent(m_pGraphics);
	mUIManagerInstance->Initialise();

	// create a screen aligned texture for post processing 
	m_screenAlignedPostProcTex1 = new ScreenAlignedTexture();
	m_screenAlignedPostProcTex1->SetDimensions(m_pGraphics->BackBufferWidth(), m_pGraphics->BackBufferHeight());
	m_screenAlignedPostProcTex1->Initialise();
	m_screenAlignedPostProcTex1->SetEffect(m_effectSepia);

	mGameScale.X = (float)m_pGraphics->BackBufferWidth() / 1920.f;
	mGameScale.Y = (float)m_pGraphics->BackBufferHeight() / 1080.f;

	PlayerLevelManager::GetInstance()->Initialise();

	FeatureUnlockManager::GetInstance()->Initialise();

	STEAM_CALLBACK(Game, OnSteamGameOverlayActivated, GameOverlayActivated_t);

	LoadCachedObjectsForPerformance();
#endif
}

void Game::Update(float delta)
{
	AudioManager::Instance()->Update();

#if defined(DEATHLEAK_PLATFORM_MAC) && DEATHLEAK_PLATFORM_MAC
	if (!mGOMInstance->IsLevelLoaded())
	{
		HandleMacMainMenuInput(mFileSystem);
		return;
	}
#endif

	bool damageEffectPauseActive = Timing::Instance()->GetTotalTimeSeconds() < (mLastTimeDamagePauseEffect + 
																			(mPauseEffectDelay * Timing::Instance()->GetTimeModifier()));

	if (mGOMInstance->IsLevelLoaded())
	{
		const LevelProperties& levelProps = mGOMInstance->GetCurrentLevelProperties();
		m_pCam2d->SetTargetOffset(levelProps.GetTargetOffset());
		m_pCam2d->SetTargetLag(levelProps.GetTargetLag());
		m_pCam2d->SetShouldFollowX(levelProps.ShouldFollowX());
		m_pCam2d->SetShouldFollowY(levelProps.ShouldFollowY());

		static int sMacPlayerTraceFrames = 0;
		if (sMacPlayerTraceFrames < 8)
		{
			Player* player = mGOMInstance->GetPlayer();
			if (player != nullptr)
			{
				LOG_INFO("mac player trace before update: pos=(%f,%f) vel=(%f,%f)", player->X(), player->Y(), player->VelocityX(), player->VelocityY());
			}
		}

#if _DEBUG
		if (!mLevelEditMode)
		{
#endif
			if (!damageEffectPauseActive)
			{
				// update all of our game objects
				mGOMInstance->Update(mPaused, delta);

				Player* player = mGOMInstance->GetPlayer();
				if (player != nullptr && (!std::isfinite(player->X()) || !std::isfinite(player->Y())))
				{
					LOG_ERROR("Player became non-finite after update: pos=(%f,%f)", player->X(), player->Y());
				}
				++sMacPlayerTraceFrames;
			}
#if _DEBUG
		}
		else
		{
			static bool hasUpdatedOnce = false;
			if (!hasUpdatedOnce)
			{
				// update all of our game objects once
				auto objects = mGOMInstance->GetGameObjectList();
				for (auto & obj : objects)
				{
					obj->Update(delta);
				}
				hasUpdatedOnce = true;
			}

			if (mlevelEditor)
			{
				mlevelEditor->Update();
			}
		}
#endif

		if (!mPaused)
		{
			const bool cameraInvalid = !std::isfinite(m_pCam2d->X()) || !std::isfinite(m_pCam2d->Y());
			if (cameraInvalid)
			{
				const Vector2 initialCamPos = levelProps.GetInitialCamPos();
				m_pCam2d->SetPositionX(initialCamPos.X);
				m_pCam2d->SetPositionY(initialCamPos.Y);
			}

			m_pCam2d->FollowTargetObjectWithLag(cameraInvalid);
			m_pCam2d->CheckBoundaryCollisions();

			// do collision detection
			if (!damageEffectPauseActive)
			{
				CollisionManager::Instance()->DetectAndResolve((int)m_pCam2d->X(), (int)m_pCam2d->Y());
			}
			
			mGOMInstance->PostUpdate(mPaused, delta);

			// NOTE: NEED to check input AFTER we do collision detection, MUST BE IN THIS ORDER
			mInputManager.ProcessGameplayInput();
		}

		if (mIsDisplayingTextModal == false)
		{
			bool pressing_pause_gamepad = false;
			GamePad * game_pad = GamePad::GetPad1();
			if (game_pad && game_pad->IsConnected())
			{
				pressing_pause_gamepad = game_pad->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_START;
			}

			static bool pressingPause = false;
			if ((GetAsyncKeyState(VK_ESCAPE) < 0 || pressing_pause_gamepad) && !pressingPause)
			{
				Player * player = GameObjectManager::Instance()->GetPlayer();

				if (player && !player->IsDead())
				{
					pressingPause = true;

					if (mPaused)
					{
						UnPauseGame();
					}
					else
					{
						PauseGame();
					}
				}
			}
			else if (!(GetAsyncKeyState(VK_ESCAPE) < 0) && !pressing_pause_gamepad)
			{
				pressingPause = false;
			}
		}
	}
	
	// update the UI
	mUIManagerInstance->Update();
	mUIManagerInstance->HandleEvents();
	
	// m_pCam2d->CheckBoundaryCollisions();

	m_pCam2d->Update();

	mInputManager.Update(delta);
}

void Game::Vibrate(float leftPercent, float rightPercent, float time)
{
	mInputManager.Vibrate(leftPercent, rightPercent, time);
}

void Game::PauseGame(bool pushPauseScreen)
{
	if (mIsDisplayingTextModal)
	{
		return;
	}

	Player* player = GameObjectManager::Instance()->GetPlayer();

	if (player)
	{
		if (player->IsInFinalLevel())
		{
			return;
		}
	}

	mPaused = true;

	if (pushPauseScreen)
	{
		UIManager::Instance()->PushUI("pause_menu");
	}
}
void Game::UnPauseGame(bool popPauseScreen )
{
	if (mIsDisplayingTextModal)
	{
		return;
	}

	mPaused = false;

	if (popPauseScreen)
	{
		UIManager::Instance()->PopUI("pause_menu");
	}
}

void Game::DisplayTextModal(const string & localizedTitle, const string & localizedText)
{
	mPaused = true;

	UIScreen * screen = UIManager::Instance()->PushUI("text_modal");
	UITextModal * textScreen = static_cast<UITextModal*>(screen);

	textScreen->SetLocalizedTitle(localizedTitle);
	textScreen->SetLocalizedDescription(localizedText);

	mIsDisplayingTextModal = true;
}

void Game::DisplayUpgradeModal(const string& localizedTitle, const string& localizedText, FeatureUnlockManager::FeatureType upgradeType, int upgradeCost)
{
	mPaused = true;

	UIScreen* screen = UIManager::Instance()->PushUI("upgrade_modal");
	UIUpgradeModal* upgradeScreen = static_cast<UIUpgradeModal*>(screen);

	upgradeScreen->SetLocalizedTitle(localizedTitle);
	upgradeScreen->SetLocalizedDescription(localizedText);
	upgradeScreen->SetUpgradeType(upgradeType);
	upgradeScreen->SetUpgradeCost(upgradeCost);

	mIsDisplayingTextModal = true;
}

void Game::DismissTextModal()
{
	mPaused = false;

	UIManager::Instance()->PopUIDeferred("text_modal");

	mIsDisplayingTextModal = false;
}

void Game::DismissUpgradeModal()
{
	mPaused = false;

	UIManager::Instance()->PopUIDeferred("upgrade_modal");

	mIsDisplayingTextModal = false;
}

void Game::Draw()
{
#if defined(DEATHLEAK_PLATFORM_MAC) && DEATHLEAK_PLATFORM_MAC
    static std::unique_ptr<MacSdlRenderer> s_macRenderer;
    SDL_Window* window = MacSdlPlatformApp::CurrentWindow();
    if (window == nullptr)
    {
        return;
    }

    if (!s_macRenderer)
    {
        s_macRenderer = std::make_unique<MacSdlRenderer>(window);
        if (!s_macRenderer->Initialise(m_pGraphics->BackBufferWidth(), m_pGraphics->BackBufferHeight()))
        {
            return;
        }
    }

    Camera2D* camera = Camera2D::GetInstance();
    if (camera == nullptr)
    {
        return;
    }

    const float screenWidth = static_cast<float>(m_pGraphics->BackBufferWidth());
    const float screenHeight = static_cast<float>(m_pGraphics->BackBufferHeight());
    s_macRenderer->BeginFrame(Color{0.02f, 0.02f, 0.025f, 1.0f});

    if (!mGOMInstance->IsLevelLoaded())
    {
        LoadMacMainMenuIfNeeded(mFileSystem, s_macRenderer.get());
        DrawMacMainMenu(s_macRenderer.get(), static_cast<int>(screenWidth), static_cast<int>(screenHeight));
    }
    else
    {
        LoadPreviewLevelIfNeeded(mFileSystem, s_macRenderer.get());
        DrawPreviewLevel(s_macRenderer.get(), PreviewState(), static_cast<int>(screenWidth), static_cast<int>(screenHeight), camera);
        DrawSepiaOverlay(s_macRenderer.get(), static_cast<int>(screenWidth), static_cast<int>(screenHeight));
    }

    s_macRenderer->EndFrame();
    return;
#else
	// update our effect variables
	D3DXMATRIX camWorld = m_pCam2d->World();
	D3DXMATRIX camView = m_pCam2d->View();
	D3DXMATRIX camProjection = m_pCam2d->Projection();

	m_effectLightTexture->SetWorldViewProjection((float*)camWorld,(float*)camView, (float*)camProjection);
	m_effectLightTextureVertexWobble->SetWorldViewProjection((float*)camWorld,(float*)camView, (float*)camProjection);
	m_effectParticleSpray->SetWorldViewProjection((float*)camWorld,(float*)camView, (float*)camProjection);
	m_effectBloodParticleSpray->SetWorldViewProjection((float*)camWorld,(float*)camView, (float*)camProjection);
	m_effectLightTextureBump->SetWorldViewProjection((float*)camWorld,(float*)camView, (float*)camProjection);
	m_effectLightTextureBump->SetLightColor((float*)D3DXVECTOR4(1.0,1.0,1.0,1.0f));
	m_effectNoise->SetWorldViewProjection((float*)camWorld,(float*)camView, (float*)camProjection);
	m_effectPixelWobble->SetWorldViewProjection((float*)camWorld,(float*)camView, (float*)camProjection);
	m_effectFoliageSway->SetWorldViewProjection((float*)camWorld, (float*)camView, (float*)camProjection);

	float shaderTime = Timing::Instance()->GetTotalTimeSeconds();

	m_effectLightTextureVertexWobble->SetTimeVariable(shaderTime);
	m_effectNoise->SetTimer(shaderTime);
	m_effectFoliageSway->SetTimeVariable(shaderTime);

	m_effectFoliageSway->SetGlobalTimeMultiplier(WeatherManager::GetInstance()->GetFoliageSwayMultiplier());

	static float pixelWobbleShaderTime = 0;
	static bool inReverse = false;

	if (!inReverse)
	{
		pixelWobbleShaderTime += (float)Timing::Instance()->GetLastUpdateDelta();
		if (pixelWobbleShaderTime > kPixelWobbleReverseDelay)
		{
			m_effectPixelWobble->ReverseMotion();
			inReverse = true;
		}
	}
	else
	{
		pixelWobbleShaderTime -= (float)Timing::Instance()->GetLastUpdateDelta();
		if (pixelWobbleShaderTime < kPixelWobbleReverseDelay * 0.5f)
		{
			m_effectPixelWobble->ReverseMotion();
			inReverse = false;
		}
	}

	m_effectPixelWobble->SetTimeVariable(pixelWobbleShaderTime);

#if _DEBUG
	m_effectBasic->SetWorldViewProjection((float*)camWorld,(float*)camView, (float*)camProjection);
#endif

	// draw all of our gameObjects
	mGOMInstance->Draw(m_pGraphics->Device());

#if _DEBUG

	if (mInputManager.ShowDebugInfoEnabled())
	{
		mGOMInstance->DebugDraw();
	}

	if (mLevelEditMode && mlevelEditor)
	{
		mlevelEditor->Draw();
	}
	
	// Vector3 camPos = Camera2D::GetInstance()->Position();
	// Graphics::GetInstance()->DrawDebugText(Utilities::getFormattedString("Cam X,Y: %f %f", camPos.X, camPos.Y).c_str(), 100, 400);

#endif
#endif
}

void Game::PostDraw() // post processsing effects here
{
#if defined(DEATHLEAK_PLATFORM_MAC) && DEATHLEAK_PLATFORM_MAC
	return;
#else
	// m_pGraphics->DisableAlphaBlending();

	// m_effectLightTexture->SetTexture(m_pGraphics->GetPreProcessSRV());
	// m_effectLightTextureVertexWobble->SetTexture(m_pGraphics->GetPreProcessSRV());
	m_effectSepia->SetTexture(m_pGraphics->GetPreProcessSRV());
	// m_effectMonochrome->SetTexture(m_pGraphics->GetPreProcessSRV());
	// m_effectMonochromeRed->SetTexture(m_pGraphics->GetPreProcessSRV());

	// draw the scene to a texture
	//m_screenAlignedPostProcTex1->SetEffect(m_effectLightTexture);
	//m_screenAlignedPostProcTex1->SetEffect(m_effectLightTextureVertexWobble);
	m_screenAlignedPostProcTex1->SetEffect(m_effectSepia);
	// m_screenAlignedPostProcTex1->SetEffect(m_effectMonochrome);
	// m_screenAlignedPostProcTex1->SetEffect(m_effectMonochromeRed);

	m_screenAlignedPostProcTex1->Draw();


	// we need to unbind the texture as we will be writing to it on the next frame
	//m_effectLightTexture->SetTexture(NULL);
	//m_effectLightTexture->CurrentTechnique->GetPassByIndex(0)->Apply(0);

	m_effectSepia->SetTexture(nullptr);
	m_effectSepia->CurrentTechnique->GetPassByIndex( 0 )->Apply(0);

	// m_effectMonochrome->SetTexture(NULL);
	// m_effectMonochrome->CurrentTechnique->GetPassByIndex( 0 )->Apply(0);

	// m_effectMonochromeRed->SetTexture(nullptr);
	// m_effectMonochromeRed->CurrentTechnique->GetPassByIndex( 0 )->Apply(0);

	//m_effectLightTextureVertexWobble->SetTexture(NULL);
	//m_effectLightTextureVertexWobble->CurrentTechnique->GetPassByIndex( 0 )->Apply(0);
	
	// re-enable alpha blending for drawing the UI
	// m_pGraphics->EnableAlphaBlending();
#endif
}

void Game::Cleanup()
{
	// delete all of our game objects
	mGOMInstance->DeleteGameObjects();

	// delete our textures
#if !(defined(DEATHLEAK_PLATFORM_MAC) && DEATHLEAK_PLATFORM_MAC)
	TextureManager::Instance()->Release();
	
	// delete our effects
	EffectManager::Instance()->Release();
#endif

	// cleanup audio
	AudioManager::Instance()->Release();

	// cleanup materials
	MaterialManager::Instance()->Release();

	// cleanup UI
	if (mUIManagerInstance != nullptr)
	{
		mUIManagerInstance->Release();
	}
	
	// release the screen aligned texture
	if (m_screenAlignedPostProcTex1 != nullptr)
	{
		m_screenAlignedPostProcTex1->Release();
		m_screenAlignedPostProcTex1 = nullptr;
	}
}

void Game::SetLevelEditFilename(const string & file) 
{ 
	GAME_ASSERT(mlevelEditor);
	if (mlevelEditor)
	{
		mlevelEditor->SetLevelFile(file);
	}
}

void Game::Create(const IFileSystem* fileSystem)
{
	GAME_ASSERT(!mInstance);

	mInstance = new Game(Graphics::GetInstance(), fileSystem);
}

void Game::Destroy()
{
	GAME_ASSERT(mInstance);

	if (mInstance)
	{
		mInstance->Cleanup();
	}

	delete mInstance;
	mInstance = nullptr;
}

bool Game::IsLevelEditTerrainMode() const
{
	if (mlevelEditor && mlevelEditor->IsTerrainEditing())
	{
		return true;
	}

	return false;
}

void Game::ResetLevelEditor()
{
	if (mlevelEditor)
	{
		mlevelEditor->Reset();
	}
}

void Game::ResetLevelEditorSelectedObject()
{
	if (mlevelEditor)
	{
		mlevelEditor->ResetSelectedObject();
	}
}

void Game::DoDamagePauseEffect()
{
	if (!mDamagePauseEnabled)
	{
		return;
	}
	mPauseEffectDelay = kPauseDamageEffectDelay;
	mLastTimeDamagePauseEffect = Timing::Instance()->GetTotalTimeSeconds();
}

void Game::DoDamagePauseEffectLonger()
{
	if (!mDamagePauseEnabled)
	{
		return;
	}
	mPauseEffectDelay = kPauseDamageEffectLongerDelay;
	mLastTimeDamagePauseEffect = Timing::Instance()->GetTotalTimeSeconds();
}

#if !(defined(DEATHLEAK_PLATFORM_MAC) && DEATHLEAK_PLATFORM_MAC)
void Game::OnSteamGameOverlayActivated(GameOverlayActivated_t* pCallback)
{
	mPaused = pCallback->m_bActive;
}
#endif

void Game::LoadCachedObjectsForPerformance()
{
	if (GameObjectManager::Instance() == nullptr)
	{
		return;
	}

	// the following loads anims from disk and caches them
	Animation * preloadAnim = new Animation(ResolveAssetPath("XmlFiles\\animation\\ninjaAnimation.xml").c_str());
	delete preloadAnim;
	preloadAnim = new Animation(ResolveAssetPath("XmlFiles\\animation\\ghost_enemy_anim.xml").c_str());
	delete preloadAnim;
	preloadAnim = new Animation(ResolveAssetPath("XmlFiles\\animation\\player_anim_default.xml").c_str());
	delete preloadAnim;
	preloadAnim = new Animation(ResolveAssetPath("XmlFiles\\bird_anim.xml").c_str());
	delete preloadAnim;
	preloadAnim = new Animation(ResolveAssetPath("XmlFiles\\bird_anim_2.xml").c_str());
	delete preloadAnim;
	preloadAnim = new Animation(ResolveAssetPath("XmlFiles\\butterfly_anim.xml").c_str());
	delete preloadAnim;
	preloadAnim = new Animation(ResolveAssetPath("XmlFiles\\crate_anim.xml").c_str());
	delete preloadAnim;
	preloadAnim = new Animation(ResolveAssetPath("XmlFiles\\orb_anim.xml").c_str());
	delete preloadAnim;
	preloadAnim = new Animation(ResolveAssetPath("XmlFiles\\pot_anim.xml").c_str());
	delete preloadAnim;
	preloadAnim = new Animation(ResolveAssetPath("XmlFiles\\rat_anim.xml").c_str());
	delete preloadAnim;
	preloadAnim = new Animation(ResolveAssetPath("XmlFiles\\small_rat_anim.xml").c_str());
	delete preloadAnim;
	preloadAnim = new Animation(ResolveAssetPath("XmlFiles\\stone_smashable.xml").c_str());
	delete preloadAnim;
}

std::string Game::ResolveAssetPath(const std::string& relativePath) const
{
	return mFileSystem != nullptr ? mFileSystem->AssetPath(relativePath) : relativePath;
}

std::string Game::ResolveSavePath(const std::string& relativePath) const
{
	return mFileSystem != nullptr ? mFileSystem->SavePath(relativePath) : relativePath;
}

std::string Game::ResolveSettingsPath(const std::string& relativePath) const
{
	return mFileSystem != nullptr ? mFileSystem->SettingsPath(relativePath) : relativePath;
}

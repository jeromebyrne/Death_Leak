#include "precompiled.h"
#include "Windows/WindowsGameRuntime.h"

#include "Achievements/Achievement.h"
#include "DXWindow.h"
#include "Game.h"
#include "Graphics.h"
#include "StringManager.h"
#include "Timing.h"
#include "UIManager.h"
#include "steam_api.h"

namespace
{
unsigned int gBackBufferWidth = 1920;
unsigned int gBackBufferHeight = 1080;
unsigned int gWindowWidth = 1920;
unsigned int gWindowHeight = 1080;
static int kSteamAppId = 995990;

enum EAchievements
{
    ACH_PULL_SWORD_ONCE = 0,
    ACH_ACCESS_DOJO = 1,
    ACH_REPAIR_BOAT = 2,
    ACH_STUN_KILL = 3,
    ACH_HEALING_WATER = 4,
    ACH_PLAY_KOTO = 5,
    ACH_FIRE_DEATH = 6,
    ACH_THREE_DEVILS = 7,
    ACH_FOCUS_UNLOCK = 8,
    ACH_STONE_SMASH = 9,
    ACH_ROLL_UNLOCK = 10,
    ACH_DEFLECT_KILL = 11,
    ACH_BOMB_5_KILL = 12
};

Achievement_t g_Achievements[] =
{
    _ACH_ID(ACH_PULL_SWORD_ONCE, "Sword in the Stomach"),
    _ACH_ID(ACH_ACCESS_DOJO, "Stop Trying to Hit Me"),
    _ACH_ID(ACH_REPAIR_BOAT, "I'm on a Boat"),
    _ACH_ID(ACH_STUN_KILL, "Dazed and Confused"),
    _ACH_ID(ACH_HEALING_WATER, "Holy Dip!"),
    _ACH_ID(ACH_PLAY_KOTO, "Strummin' My Pain"),
    _ACH_ID(ACH_FIRE_DEATH, "Yes, It Does Kill You"),
    _ACH_ID(ACH_THREE_DEVILS, "He�s Buried in Killarney"),
    _ACH_ID(ACH_FOCUS_UNLOCK, "And Hit Me!"),
    _ACH_ID(ACH_STONE_SMASH, "Stoneybatter"),
    _ACH_ID(ACH_ROLL_UNLOCK, "Now Move in, Now move out"),
    _ACH_ID(ACH_DEFLECT_KILL, "The Art of Deflection"),
    _ACH_ID(ACH_BOMB_5_KILL, "Fatal Frame(rate)")
};

WindowsGameRuntime* gWindowsGameRuntime = nullptr;
}

WindowsGameRuntime::WindowsGameRuntime() = default;
WindowsGameRuntime::~WindowsGameRuntime() = default;

int WindowsGameRuntime::Run(HINSTANCE hInstance, int nCmdShow)
{
    Timing::Create();
    gWindowsGameRuntime = this;

    if (SteamAPI_RestartAppIfNecessary(kSteamAppId))
    {
        return 0;
    }

    mSteamInitialised = SteamAPI_Init();
    if (mSteamInitialised)
    {
        mAchievements = new CSteamAchievements(g_Achievements, 13);
    }

    bool isSteamDeck = SteamUtils() != nullptr ? SteamUtils()->IsSteamRunningOnSteamDeck() : false;
    if (isSteamDeck)
    {
        gWindowWidth = 1280;
        gWindowHeight = 720;
    }

    if (FAILED(Initialise(hInstance, nCmdShow)))
    {
        Shutdown();
        return 0;
    }

    MSG mssg;
    const double targetDelta = 1.0 / 60.0;
    Timing::Instance()->SetTargetDelta(targetDelta);

    double lastUpdateTime = 0.0;
    bool firstUpdate = true;

    PeekMessage(&mssg, NULL, 0, 0, PM_NOREMOVE);

    while (mssg.message != WM_QUIT)
    {
        if (mDestroyGame)
        {
            Shutdown();
            break;
        }

        if (firstUpdate)
        {
            lastUpdateTime = timeGetTime();
            firstUpdate = false;
            continue;
        }

        const double currentTime = timeGetTime();
        Timing* timing = Timing::Instance();
        double delta = (currentTime - lastUpdateTime) * 0.001;
        if (delta < timing->GetTargetDelta())
        {
            continue;
        }

        if (delta > timing->GetTargetDelta())
        {
            delta = timing->GetTargetDelta();
        }

        timing->Update(delta);
        Update(static_cast<float>(delta * timing->GetTimeModifier()));
        lastUpdateTime = currentTime;
        timing->IncrementTotalTimeSeconds(static_cast<float>(delta * timing->GetTimeModifier()));

        Render();
        timing->SetLastUpdateDelta(static_cast<float>(delta * timing->GetTimeModifier()));
        Present();

        if (PeekMessage(&mssg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&mssg);
            DispatchMessage(&mssg);
        }
    }

    Shutdown();
    gWindowsGameRuntime = nullptr;
    return 1;
}

LRESULT WindowsGameRuntime::HandleWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:
            mDestroyGame = true;
            break;
        case WM_SETCURSOR:
        {
            WORD ht = LOWORD(lParam);
            static bool hiddencursor = false;
            if (HTCLIENT == ht && !hiddencursor)
            {
                hiddencursor = true;
                ShowCursor(false);
            }
            else if (HTCLIENT != ht && hiddencursor)
            {
                hiddencursor = false;
                ShowCursor(true);
            }
            break;
        }
        case WM_CHAR:
        {
            char c = static_cast<char>(wParam);
            if (isprint(c))
            {
                auto uiManager = UIManager::Instance();
                if (uiManager)
                {
                    uiManager->HandleKeyPressInKeyboardInputMode(c);
                }
            }
            else if (c == '\b')
            {
                auto uiManager = UIManager::Instance();
                if (uiManager)
                {
                    uiManager->HandleBackspaceInKeyboardInputMode();
                }
            }
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

HRESULT WindowsGameRuntime::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    mWindow = new DXWindow(gWindowWidth, gWindowHeight);
    mGraphics = new Graphics();
    Game::Create(&mFileSystem);
    mGame = Game::GetInstance();

    StringManager::Create();
    StringManager::GetInstance()->LoadStringsFile(mFileSystem.AssetPath("XmlFiles\\strings.xml").c_str());

    HRESULT result = S_OK;
    result = mWindow->Initialise(hInstance, nCmdShow, [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT {
        return gWindowsGameRuntime != nullptr ? gWindowsGameRuntime->HandleWindowMessage(hWnd, message, wParam, lParam)
                                              : DefWindowProc(hWnd, message, wParam, lParam);
    });

    if (FAILED(result = mGraphics->Initialise(mWindow->Hwnd(), gBackBufferWidth, gBackBufferHeight, 0.0f, 1.0f, 0.0f, 0.0f)))
    {
        MessageBox(mWindow->Hwnd(), L"Failed to init graphics", L"FAILED", NULL);
    }

    mGame->Initialise();
    return result;
}

void WindowsGameRuntime::Update(float delta)
{
    SteamAPI_RunCallbacks();
    mGame->Update(delta);
}

void WindowsGameRuntime::Render()
{
    mGraphics->Clear();
    mGraphics->EnableAlphaBlending();

    bool postProcessingEnabled = true;
#ifdef DEBUG
    postProcessingEnabled = Game::GetInstance()->GetInputManager().GraphicsPostProcessingEnabled();
#endif

    if (postProcessingEnabled)
    {
        mGraphics->SwitchToPreProcessRenderTarget();
        mGame->Draw();
        mGraphics->SwitchToBackBufferRenderTarget();
        mGame->PostDraw();
    }
    else
    {
        mGraphics->SwitchToBackBufferRenderTarget();
        mGame->Draw();
    }

    UIManager::Instance()->Draw(mGraphics->Device());
}

void WindowsGameRuntime::Present()
{
    mGraphics->SwapBuffers();
}

void WindowsGameRuntime::Shutdown()
{
    if (mGraphics != nullptr)
    {
        mGraphics->CleanupDevice();
        delete mGraphics;
        mGraphics = nullptr;
    }

    if (mGame != nullptr)
    {
        Game::Destroy();
        mGame = nullptr;
    }

    if (mSteamInitialised)
    {
        SteamAPI_Shutdown();
        mSteamInitialised = false;
    }

    if (mAchievements != nullptr)
    {
        delete mAchievements;
        mAchievements = nullptr;
    }

    delete mWindow;
    mWindow = nullptr;
}

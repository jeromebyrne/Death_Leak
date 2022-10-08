#include "precompiled.h"
#include "DXWindow.h"
#include "Graphics.h"
#include "Game.h"
#include "Timing.h"
#include "UIManager.h"
#include "StringManager.h"
#include "steam_api.h"
#include <Achievements\Achievement.h>

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------

unsigned int gBackBufferWidth = 1920;
unsigned int gBackBufferHeight = 1080;
unsigned int gWindowWidth = 1920;
unsigned int gWindowHeight = 1080;

static int kSteamAppId = 995990;

DXWindow * g_pWindow = nullptr;
Graphics * g_pGraphics = nullptr;
Game * g_pGame = nullptr;


// ------------------------------------
// 11th hour achievement code that should really not live here
// ------------------------------------

// Defining our achievements
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

// Achievement array which will hold data about the achievements and their state
Achievement_t g_Achievements[] =
{
	_ACH_ID(ACH_PULL_SWORD_ONCE, "Sword in the Stomach"),
	_ACH_ID(ACH_ACCESS_DOJO, "Stop Trying to Hit Me"),
	_ACH_ID(ACH_REPAIR_BOAT, "I'm on a Boat"),
	_ACH_ID(ACH_STUN_KILL, "Dazed and Confused"),
	_ACH_ID(ACH_HEALING_WATER, "Holy Dip!"),
	_ACH_ID(ACH_PLAY_KOTO, "Strummin' My Pain"),
	_ACH_ID(ACH_FIRE_DEATH, "Yes, It Does Kill You"),
	_ACH_ID(ACH_THREE_DEVILS, "He’s Buried in Killarney"),
	_ACH_ID(ACH_FOCUS_UNLOCK, "And Hit Me!"),
	_ACH_ID(ACH_STONE_SMASH, "Stoneybatter"),
	_ACH_ID(ACH_ROLL_UNLOCK, "Now Move in, Now move out"),
	_ACH_ID(ACH_DEFLECT_KILL, "The Art of Deflection"),
	_ACH_ID(ACH_BOMB_5_KILL, "Fatal Frame(rate)")
};

CSteamAchievements* g_SteamAchievements = nullptr;

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);
void Update(float delta);
void Render();
void Present();
void KillGame();

bool gDestroyGame = false;
void PostDestroyMessage();

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	Timing::Create();

	// TODO: maybe this needs to be called after init?
	if (SteamAPI_RestartAppIfNecessary(kSteamAppId));
	{
		// exit(0);
	}

	// Initialize Steam
	bool steamInit = SteamAPI_Init();
	if (steamInit)
	{
		g_SteamAchievements = new CSteamAchievements(g_Achievements, 13);
	}

	bool isSteamDeck = SteamUtils() != nullptr ? SteamUtils()->IsSteamRunningOnSteamDeck() : false;

	if (isSteamDeck)
	{
		gWindowWidth = 1280;
		gWindowHeight = 720; // in conjunction with fullscreen off, this will mean the game is not stretched vertically
	}

	if( FAILED(  Initialise(hInstance, nCmdShow) ))
        return 0;

	MSG mssg;                // message from queue
	double  target_delta = 1.0/60.0; 

	Timing::Instance()->SetTargetDelta(target_delta);

	double last_update_time = 0.0;
	bool first_update = true;

	// timeBeginPeriod(1); // set timing resolution to 1 millisecond
#if _DEBUG
	// _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	// _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif

	// prime the message structure
	PeekMessage( &mssg, NULL, 0, 0, PM_NOREMOVE);

	// run till completed
	while (mssg.message!=WM_QUIT) 
	{
		// check if we should destroy everything
		if (gDestroyGame)
		{
			KillGame();
			break;
		}

		if (first_update)
		{
			last_update_time = timeGetTime();
			first_update = false;
			continue;
		}

		double currentTime = timeGetTime();
		Timing * timing = Timing::Instance();

		double delta = (currentTime - last_update_time) * 0.001;

		if (delta < timing->GetTargetDelta())
		{
			continue;
		}

		double gameSpeed = 1.0; 

		delta *= gameSpeed;

		// just force this to 1.0 as lots of issues
		if (delta > timing->GetTargetDelta())
		{
			delta = timing->GetTargetDelta();
		}

		timing->Update(delta);
		Update(delta * timing->GetTimeModifier());
		last_update_time = currentTime;
		timing->IncrementTotalTimeSeconds(delta * timing->GetTimeModifier());

		Render();

		timing->SetLastUpdateDelta(delta * timing->GetTimeModifier());
		
		Present();

		// is there a message to process?
		if (PeekMessage(&mssg, NULL, 0, 0, PM_REMOVE))
		{
			// dispatch the message
			TranslateMessage(&mssg);
			DispatchMessage(&mssg);
		}
	}

	return 1;
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
        case WM_PAINT:
		{
			hdc = BeginPaint( hWnd, &ps );
			EndPaint( hWnd, &ps );			
			break;
		}
        case WM_DESTROY:
		{
			gDestroyGame = true;
			break;
		}
		case WM_SETCURSOR:
		{
			WORD ht = LOWORD(lParam); 
			static bool hiddencursor = false;
			if (HTCLIENT==ht && !hiddencursor)
			{
				hiddencursor = true;
				ShowCursor(false);
			}
			else if (HTCLIENT!=ht && hiddencursor) 
			{
				hiddencursor = false;
				ShowCursor(true);
			}
			break;
		}
		case WM_CHAR:
		{
			char c = wParam;

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
				// handle backspace
				auto uiManager = UIManager::Instance();

				if (uiManager)
				{
					uiManager->HandleBackspaceInKeyboardInputMode();
				}
			}
		}

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}

HRESULT Initialise(HINSTANCE hInstance, int nCmdShow)
{
	g_pWindow = new DXWindow(gWindowWidth, gWindowHeight);
	g_pGraphics = new Graphics();
	Game::Create();
	g_pGame = Game::GetInstance();

	StringManager::Create();
	StringManager::GetInstance()->LoadStringsFile("XmlFiles\\strings.xml");

	HRESULT result = S_OK;
	
	// initialise window
	result = g_pWindow->Initialise(hInstance, nCmdShow, WndProc);

	//initialise graphics device and plug into window
	if(FAILED(result = g_pGraphics->Initialise(g_pWindow->Hwnd(), gBackBufferWidth, gBackBufferHeight, 0.0f, 1.0f, 0.0f,0.0f)))
	{
		MessageBox(g_pWindow->Hwnd(),L"Failed to init graphics",L"FAILED",NULL);
	}

	// initialise the game
	g_pGame->Initialise();

	return result;
}
void Update(float delta)
{
	SteamAPI_RunCallbacks();

	g_pGame->Update(delta);
}
void Render()
{
	g_pGraphics->Clear();
	g_pGraphics->EnableAlphaBlending(); // Required?

	bool postProcessingEnabled = true;

#ifdef DEBUG
	postProcessingEnabled = Game::GetInstance()->GetInputManager().GraphicsPostProcessingEnabled();
#endif

	if (postProcessingEnabled)
	{
		g_pGraphics->SwitchToPreProcessRenderTarget();
		g_pGame->Draw();
		g_pGraphics->SwitchToBackBufferRenderTarget();
		g_pGame->PostDraw();
	}
	else
	{
		g_pGraphics->SwitchToBackBufferRenderTarget();
		g_pGame->Draw();
	}

	UIManager::Instance()->Draw(g_pGraphics->Device());
}

void Present()
{
	g_pGraphics->SwapBuffers();
}

void KillGame()
{
	g_pGraphics->CleanupDevice();
	delete g_pGraphics;
	g_pGame->Destroy();
    PostQuitMessage( 0 );
	SteamAPI_Shutdown();
	if (g_SteamAchievements)
		delete g_SteamAchievements;
}

void PostDestroyMessage()
{
	gDestroyGame = true;
}



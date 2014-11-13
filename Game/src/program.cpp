#include "precompiled.h"
#include "DXWindow.h"
#include "Graphics.h"
#include "Game.h"
#include "Timing.h"
#include "UIManager.h"

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------

unsigned int gBackBufferWidth = 1920;
unsigned int gBackBufferHeight = 1080;
DXWindow * g_pWindow = nullptr;
Graphics * g_pGraphics = nullptr;
Game * g_pGame = nullptr;

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);
void Update(float delta);
void Render();
void KillGame();

bool gDestroyGame = false;
void PostDestroyMessage();


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	if( FAILED(  Initialise(hInstance, nCmdShow) ))
        return 0;

	MSG mssg;                // message from queue
	// LONGLONG cur_time;       // current time
	float  target_delta = 1.0f/60.0f; 
	//LONGLONG perf_cnt;       // performance timer frequency
	//BOOL perf_flag=FALSE;    // flag determining which timer to use
	//LONGLONG next_time=0;    // time to render next frame

	Timing::Instance()->SetTargetDelta(target_delta);

	double last_update_time = 0;
	bool first_update = true;

	timeBeginPeriod(1); // set timing resolution to 1 millisecond

	/*double freq;
    unsigned __int64 base_time; // what happens if hardware doesnt support this?

	// is there a performance counter available?
	if (QueryPerformanceFrequency((LARGE_INTEGER *) &perf_cnt)) 
	{
		// yes, set time_count and timer choice flag
		perf_flag=TRUE;
		time_count=perf_cnt/frames_per_second;        // calculate time per frame based on frequency
		QueryPerformanceCounter((LARGE_INTEGER *) &next_time);

		unsigned __int64 pf;
		QueryPerformanceFrequency( (LARGE_INTEGER *)&pf );
		freq = 1.0 / (double)pf;
		QueryPerformanceCounter( (LARGE_INTEGER *)& base_time);
	} 
	else 
	{
		// no performance counter, read in using timeGetTime
		next_time=timeGetTime();
	}*/

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

		// is there a message to process?
		if (PeekMessage( &mssg, NULL, 0, 0, PM_REMOVE)) 
		{
			// dispatch the message
			TranslateMessage(&mssg);
			DispatchMessage(&mssg);
		} 
		
		/*float delta = 0;

		// use the appropriate method to get time
		if (perf_flag)
		{
			QueryPerformanceCounter((LARGE_INTEGER *) &cur_time);

			double milliseconds = ((cur_time - base_time) * freq) * 1000;
				
			Timing::Instance()->SetTotalGameTime(milliseconds);
		}
		else
		{
			cur_time=timeGetTime();
			Timing::Instance()->SetTotalGameTime(cur_time);
		}

		// is it time to render and update
		if (cur_time > next_time) 
		{
			// yes, render the frame
			Update(delta);
			Render();

			// set time for next frame
			next_time += time_count;
		}*/

		if (first_update)
		{
			last_update_time = timeGetTime();
			first_update = false;
			continue;
		}

		double currentTime = timeGetTime();
		Timing * timing = Timing::Instance();

		double delta = (currentTime - last_update_time) * 0.001f;
		timing->SetLastUpdateDelta(delta * timing->GetTimeModifier());

		if (delta > target_delta)
		{
			Update(delta * timing->GetTimeModifier());
			last_update_time = currentTime;
			timing->IncrementTotalTimeSeconds(delta * timing->GetTimeModifier());
		}

		// draw every frame
		Render();
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
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );			
            break;
		
        case WM_DESTROY:
			gDestroyGame = true;
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}

HRESULT Initialise(HINSTANCE hInstance, int nCmdShow)
{
	g_pWindow = new DXWindow("Death Leak", "Death Leak", 1920, 1080);
	g_pGraphics = new Graphics();
	Game::Create();
	g_pGame = Game::GetInstance();

	HRESULT result = S_OK;
	
	// initialise window
	result = g_pWindow->Initialise(hInstance, nCmdShow, WndProc);

	//initialise graphics device and plug into window
	if(FAILED(result = g_pGraphics->Initialise(g_pWindow->Hwnd(), gBackBufferWidth, gBackBufferHeight, 0.0f,1.0f, 0,0)))
	{
		MessageBox(g_pWindow->Hwnd(),L"Failed to init graphics",L"FAILED",NULL);
	}

	// initialise the game
	g_pGame->Initialise();

	return result;
}
void Update(float delta)
{
	g_pGame->Update(delta);
}
void Render()
{
	//
    // Clear the backbuffer
    //
	g_pGraphics->Clear();
	g_pGraphics->EnableAlphaBlending();
	//g_pGraphics->SwitchToBackBufferRenderTarget();
	g_pGraphics->SwitchToPreProcessRenderTarget();
	// draw the game
	g_pGame->Draw();
	
	// Do Post processing 
	g_pGraphics->SwitchToBackBufferRenderTarget();
	g_pGame->PostDraw();

	// draw the UI last
	UIManager::Instance()->Draw(g_pGraphics->Device());

    g_pGraphics->SwapBuffers();
}
void KillGame()
{
	g_pGraphics->CleanupDevice();
	delete g_pGraphics;
	g_pGame->Destroy();
    PostQuitMessage( 0 );
}

void PostDestroyMessage()
{
	gDestroyGame = true;
}



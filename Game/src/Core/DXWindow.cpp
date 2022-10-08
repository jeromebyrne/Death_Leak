#include "precompiled.h"
#include "DXWindow.h"

DXWindow * DXWindow::mInstance = 0;

DXWindow::DXWindow(int width, int height) : 
	m_width(width),
	m_height(height),
	m_topLeftX(0),
	m_topLeftY(0)
{
	m_pGraphicsDevice = nullptr;
	m_pGraphicsSwapChain = nullptr;

	mInstance = this;
}

DXWindow::~DXWindow(void)
{
	Destroy();
}

HRESULT DXWindow::Initialise(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc)
{
	// Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = wndproc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"Death Leak";
    wcex.hIconSm = NULL;
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    m_hInstance = hInstance;
    RECT rc = { 0, 0, m_width, m_height };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	
	DWORD dwStyle;

	dwStyle = WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_MAXIMIZE;


	m_hWnd = CreateWindow(
							L"Death Leak",
							L"Death Leak",
							dwStyle,
							CW_USEDEFAULT, 
							CW_USEDEFAULT, 
							rc.right - rc.left, 
							rc.bottom - rc.top, 
							NULL, 
							NULL,
							hInstance,
							NULL );
    if( !m_hWnd )
        return E_FAIL;

    ShowWindow( m_hWnd, nCmdShow );

	return S_OK;
}

void DXWindow::Destroy()
{
	DestroyWindow(m_hWnd);
}

Vector2 DXWindow::GetWindowDimensions()
{
	RECT dimensions;
	GetClientRect(m_hWnd, &dimensions);

	Vector2 retVal = Vector2(dimensions.right, dimensions.bottom);

	return retVal;
}



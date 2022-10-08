#ifndef DXWINDOW_H
#define DXWINDOW_H

#include <d3d10.h>
#include <d3dx10.h>

class DXWindow
{
public:

	DXWindow( int windowWidth = 640, int windowHeight = 480);
	~DXWindow(void);
	
	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc);

	inline HWND Hwnd()
	{
		return m_hWnd;
	}
	inline HINSTANCE HInstance()
	{
		return m_hInstance;
	}
	inline int Width()
	{
		return m_width;
	}
	inline int Height()
	{
		return m_height;
	}
	void Destroy();

	static DXWindow * GetInstance() { return mInstance; }
	
	Vector2 GetWindowDimensions();

private:

	HINSTANCE m_hInstance;
	HWND m_hWnd;

	ID3D10Device*           m_pGraphicsDevice; // pointer to the graphics device
	IDXGISwapChain*         m_pGraphicsSwapChain; // pointer to the object for switching front and back buffers

	int m_width;
	int m_height;
	int m_topLeftX;
	int m_topLeftY;

	static DXWindow * mInstance;
};

#endif

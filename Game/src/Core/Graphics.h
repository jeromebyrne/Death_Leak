#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <d3d10.h>
#include <d3dx10.h>
#include "InputLayoutDescriptions.h"

class SimpleFontManager;

class Graphics
{
private:
	 ID3D10Device*				m_pd3dDevice; // pointer to the graphics device
	 IDXGISwapChain*			m_pSwapChain; // pointer to the object for switching front and back buffers
	 ID3D10RenderTargetView*	m_pBackBufferRenderTargetView; // render target view
	 ID3D10RenderTargetView*	m_pPreProcessRenderTargetView;
	 D3D10_DRIVER_TYPE			m_driverType;// driver type
	 ID3D10Texture2D*			m_pBackBuffer;// back buffer
	 ID3D10Texture2D*           m_pDepthStencil; // depth buffer
	 ID3D10Texture2D*			m_pPreProcessTexture;
	 ID3D10DepthStencilView*    m_pDepthStencilView;// depth buffer view
	 ID3D10ShaderResourceView*  m_preProcessSRV;
	 ID3D10ShaderResourceView*  m_depthStencilSRV; // get the depth stencil texture's shader resource view
	 HWND m_hWnd;

	 ID3D10BlendState* m_alphaEnabledBlendState; // alpha blending enabled state
	 ID3D10BlendState* m_alphaDisabledBlendState; // alpha blending disabled state

	 ID3D10RasterizerState * m_defaultRasterState;

	 HRESULT SetupDevice(HWND hWnd, int bufferWidth, int bufferHeight);

	 DXGI_SWAP_CHAIN_DESC m_swapChainDescription; // swap chain properties
	 void SetSwapChainProperties(HWND hWnd,int bufferWidth, int bufferHeight);
	 HRESULT CreateRenderTargetViews(int bufferWidth, int bufferHeight);
	 HRESULT CreateDepthStencilBuffer(int bufferWidth, int bufferHeight);

	 void CreateAlphaEnabledBlendState();
	 void CreateAlphaDisabledBlendState();

	 void CreateDefaultRasterState();

	 int m_backBufferWidth;
	 int m_backBufferHeight;

	 bool m_alphaToCoverageEnabled; // is alpha to coverage enabled

	 // adding this at a late stage, you'll see alot of passing of this object before I made this change
	 static Graphics * mInstance;

	 SimpleFontManager * mSimpleFontManager;
public:
	InputLayoutDescriptions InputDescriptions;
	Graphics(void);
	~Graphics(void);
	HRESULT Initialise(HWND hWnd, int backBufferWidth, int backBufferHeight, float minDepth, float maxDepth, int topLeftX, int topLeftY);
	void CleanupDevice();
	void SetViewPort(int width, int height, float minDepth, float maxDepth, int topLeftX, int topLeftY);

	void Clear();
	void SwapBuffers();

	inline int BackBufferWidth()
	{
		return m_backBufferWidth;
	}

	inline int BackBufferHeight()
	{
		return m_backBufferHeight;
	}

	inline ID3D10Device* Device()
	{
		return m_pd3dDevice;
	}
	inline IDXGISwapChain* SwapChain()
	{
		return m_pSwapChain;
	}
	inline ID3D10RenderTargetView* RenderTargetView()
	{
		return m_pBackBufferRenderTargetView;
	}
	inline D3D10_DRIVER_TYPE DriverType()
	{
		return m_driverType;
	}

	void ShowMessageBox(wchar_t* message, wchar_t* title);

	// enable alpha blending
	void EnableAlphaBlending();
	void DisableAlphaBlending();

	// enable and disable alpha to coverage - enabling alpha to coverage increases performance but creates 
	// a bad effect on sprites that are transparent
	void EnableAlphaToCoverage();
	void DisableAlphaToCoverage();	

	static Graphics * GetInstance() { return mInstance; }

	void SetDefaultRasterState();

	void SwitchToPreProcessRenderTarget();

	void SwitchToBackBufferRenderTarget();

	ID3D10ShaderResourceView* GetPreProcessSRV() { return m_preProcessSRV; }

	ID3D10ShaderResourceView* GetDepthStencilSRV() { return m_depthStencilSRV; }

	void DrawDebugText(const char * text, float top, float left);
};

#endif
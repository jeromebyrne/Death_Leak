#include "precompiled.h"
#include "Graphics.h"
#include "simplefontmanager.h"

Graphics * Graphics::mInstance = nullptr;

static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

Graphics::Graphics(void):
	mSimpleFontManager(nullptr),
	mIsFullScreen(false),
	mVSyncEnabled(true),
	m_pd3dDevice(nullptr),
	m_pBackBufferRenderTargetView(nullptr),
	m_pSwapChain(nullptr),
	m_driverType(D3D10_DRIVER_TYPE_HARDWARE),
	m_pPreProcessRenderTargetView(nullptr),
	m_pPreProcessTexture(nullptr),
	m_preProcessSRV(nullptr),
	m_pBackBuffer(nullptr),
	m_alphaDisabledBlendState(nullptr),
	m_alphaEnabledBlendState(nullptr),
	m_defaultRasterState(nullptr),
	m_alphaToCoverageEnabled(false)
{
	GAME_ASSERT(!mInstance);
	mInstance = this;
}

Graphics::~Graphics(void)
{
	CleanupDevice();
	if (mSimpleFontManager != nullptr)
	{
		mSimpleFontManager->Release();
		delete mSimpleFontManager;
		mSimpleFontManager = nullptr;
	}
}

HRESULT Graphics::Initialise(HWND, int backBufferWidth, int backBufferHeight, float, float, int, int)
{
	m_backBufferWidth = backBufferWidth;
	m_backBufferHeight = backBufferHeight;

	if (m_pd3dDevice == nullptr)
	{
		m_pd3dDevice = new ID3D10Device();
	}
	if (m_pSwapChain == nullptr)
	{
		m_pSwapChain = new IDXGISwapChain();
	}
	if (mSimpleFontManager == nullptr)
	{
		mSimpleFontManager = new SimpleFontManager();
		mSimpleFontManager->Init(this);
	}
	return S_OK;
}

void Graphics::CleanupDevice()
{
	if (m_pBackBufferRenderTargetView != nullptr) { m_pBackBufferRenderTargetView->Release(); m_pBackBufferRenderTargetView = nullptr; }
	if (m_pPreProcessRenderTargetView != nullptr) { m_pPreProcessRenderTargetView->Release(); m_pPreProcessRenderTargetView = nullptr; }
	if (m_preProcessSRV != nullptr) { m_preProcessSRV->Release(); m_preProcessSRV = nullptr; }
	if (m_pSwapChain != nullptr) { m_pSwapChain->Release(); m_pSwapChain = nullptr; }
	if (m_pd3dDevice != nullptr) { m_pd3dDevice->Release(); m_pd3dDevice = nullptr; }
}

void Graphics::SetViewPort(int, int, float, float, int, int)
{
}

void Graphics::Clear()
{
	if (m_pd3dDevice != nullptr)
	{
		m_pd3dDevice->ClearRenderTargetView(m_pBackBufferRenderTargetView, clearColor);
		m_pd3dDevice->ClearRenderTargetView(m_pPreProcessRenderTargetView, clearColor);
	}
}

void Graphics::SwapBuffers()
{
	if (m_pSwapChain != nullptr)
	{
		m_pSwapChain->Present(mVSyncEnabled ? 1 : 0, 0);
	}
}

void Graphics::ShowMessageBox(wchar_t*, wchar_t*)
{
}

void Graphics::EnableAlphaBlending() {}
void Graphics::DisableAlphaBlending() {}
void Graphics::EnableAlphaToCoverage() {}
void Graphics::DisableAlphaToCoverage() {}
void Graphics::SetDefaultRasterState() {}
void Graphics::SwitchToPreProcessRenderTarget() {}
void Graphics::SwitchToBackBufferRenderTarget() {}

void Graphics::DrawDebugText(const char * text, float top, float left)
{
	if (mSimpleFontManager != nullptr)
	{
		mSimpleFontManager->DrawDebugText(text, top, left);
	}
}

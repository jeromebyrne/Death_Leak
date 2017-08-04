#include "precompiled.h"
#include "Graphics.h"
#include "simplefontmanager.h"

Graphics * Graphics::mInstance = nullptr;

// clear color
static const float clearColor[4] = { 0.3f, 0.3f, 0.3f, 1.0f };

Graphics::Graphics(void):
	mSimpleFontManager(nullptr),
	mIsFullScreen(true),
	mVSyncEnabled(true)
{
	// TODO: add these to member initialiser
	m_pd3dDevice = nullptr;
	m_pBackBufferRenderTargetView = nullptr;
	m_pSwapChain = nullptr;
	m_driverType= D3D10_DRIVER_TYPE_NULL;
	m_pPreProcessRenderTargetView = nullptr;
	m_pPreProcessTexture = nullptr;
	m_preProcessSRV = nullptr;
	
	m_pBackBuffer = nullptr;
	m_alphaDisabledBlendState = nullptr;
	m_alphaEnabledBlendState = nullptr;
	m_defaultRasterState = nullptr;
	m_alphaToCoverageEnabled = false;

	GAME_ASSERT(!mInstance);
	mInstance = this;
}

Graphics::~Graphics(void)
{
	mSimpleFontManager->Release();
	mSimpleFontManager = 0;
}

HRESULT Graphics::Initialise(HWND hWnd, int backBufferWidth, int backBufferHeight, float minDepth, float maxDepth, int topLeftX, int topLeftY)
{
	HRESULT result = S_OK;

	m_backBufferWidth = backBufferWidth;
	m_backBufferHeight = backBufferHeight;

	m_hWnd = hWnd;

	result = SetupDevice(hWnd, backBufferWidth, backBufferHeight);
	SetViewPort(backBufferWidth, backBufferHeight, minDepth, maxDepth,topLeftX,topLeftY);

	CreateAlphaEnabledBlendState();
	CreateAlphaDisabledBlendState();

	CreateDefaultRasterState();
	SetDefaultRasterState();

	mSimpleFontManager = new SimpleFontManager();
	mSimpleFontManager->Init(this);

	return S_OK;
}

HRESULT Graphics::CreateDepthStencilBuffer(int bufferWidth, int bufferHeight)
{
	HRESULT hr = S_OK;

	// create depth stencil state ===========================
	D3D10_DEPTH_STENCIL_DESC dsDesc;

	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D10_COMPARISON_ALWAYS;

	// Stencil test parameters
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D10_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D10_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D10_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D10_COMPARISON_ALWAYS;

	// Create depth stencil state
	ID3D10DepthStencilState * pDSState;
	m_pd3dDevice->CreateDepthStencilState(&dsDesc, &pDSState);

	// Bind depth stencil state
	m_pd3dDevice->OMSetDepthStencilState(pDSState, 1);
	// =================================================================

	// Create depth stencil texture
    D3D10_TEXTURE2D_DESC descDepth;
    descDepth.Width = bufferWidth;
    descDepth.Height = bufferHeight;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_R32_TYPELESS;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D10_USAGE_DEFAULT;
    descDepth.BindFlags = D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
	hr = m_pd3dDevice->CreateTexture2D( &descDepth, NULL, &m_pDepthStencil );
    if( FAILED(hr) )
	{
		LOG_ERROR("Depth Stencil Buffer create failure");
		GAME_ASSERT(false);
	}

    // Create the depth stencil view
    D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
    descDSV.Format = DXGI_FORMAT_D32_FLOAT;
    descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
	hr = m_pd3dDevice->CreateDepthStencilView( m_pDepthStencil, &descDSV, &m_pDepthStencilView );
    if( FAILED(hr) )
	{
		LOG_ERROR("Depth Stencil View create failure");
		GAME_ASSERT(false);
	}
	
	// create a shader resource view for the depth stencil buffer
	D3D10_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderResourceViewDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	hr = m_pd3dDevice->CreateShaderResourceView(m_pDepthStencil, &shaderResourceViewDesc, &m_depthStencilSRV);
	if(FAILED(hr))
	{
		LOG_ERROR("Depth Stencil Resource View create failure");
		GAME_ASSERT(false);
		return false;
	}

	return hr;
}

HRESULT Graphics::SetupDevice(HWND hWnd, int bufferWidth, int bufferHeight)
{
	HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
#endif

    D3D10_DRIVER_TYPE driverTypes[] =
    {
        D3D10_DRIVER_TYPE_HARDWARE,
		D3D10_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = sizeof( driverTypes ) / sizeof( driverTypes[0] );

	SetSwapChainProperties(hWnd, bufferWidth,bufferHeight);

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        m_driverType = driverTypes[driverTypeIndex];
        hr = D3D10CreateDeviceAndSwapChain( nullptr, 
											m_driverType,
											nullptr, 
											createDeviceFlags,
											D3D10_SDK_VERSION,
											&m_swapChainDescription,
											&m_pSwapChain, 
											&m_pd3dDevice );
        if( SUCCEEDED( hr ) )
            break;
    }

    if( FAILED( hr ) )
	{
		LOG_ERROR("Device Setup failed. If a developer check you have all Windows/VS SDK components installed.");
		GAME_ASSERT(false);

        return hr;
	}

	// Create a render target view
    CreateRenderTargetViews(m_backBufferWidth, m_backBufferHeight);

	// create stencil buffer view, use the native buffer width and height
	CreateDepthStencilBuffer(m_backBufferWidth, m_backBufferHeight);

	// set render targets
	m_pd3dDevice->OMSetRenderTargets( 1, &m_pPreProcessRenderTargetView, m_pDepthStencilView );

	return S_OK;
}
void Graphics::CleanupDevice()
{
    if( m_pBackBufferRenderTargetView ) m_pBackBufferRenderTargetView->Release();
	if( m_pPreProcessRenderTargetView ) m_pPreProcessRenderTargetView->Release();
	if ( m_preProcessSRV ) m_preProcessSRV->Release();

	if (m_pSwapChain)
	{
		m_pSwapChain->SetFullscreenState(false, nullptr);
		m_pSwapChain->Release();
	}
	if (m_pd3dDevice) m_pd3dDevice->ClearState();
    if( m_pd3dDevice ) m_pd3dDevice->Release();
}

void Graphics::SetViewPort(int width, int height, float minDepth, float maxDepth, int topLeftX, int topLeftY)
{
	D3D10_VIEWPORT vp;
    vp.Width = width;
    vp.Height = height;
    vp.MinDepth = minDepth;
    vp.MaxDepth = maxDepth;
    vp.TopLeftX = topLeftX;
    vp.TopLeftY = topLeftY;
    m_pd3dDevice->RSSetViewports( 1, &vp );
}

void Graphics::Clear()
{
	// clear the back buffer
	m_pd3dDevice->ClearRenderTargetView( m_pBackBufferRenderTargetView, clearColor );

	// clear preprocess render target
	m_pd3dDevice->ClearRenderTargetView( m_pPreProcessRenderTargetView, clearColor );

	// clear the depth buffer
	m_pd3dDevice->ClearDepthStencilView( m_pDepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0 );
}
void Graphics::SwapBuffers()
{
	m_pSwapChain->Present(mVSyncEnabled ? 1 : 0, 0);
}

void Graphics::SetSwapChainProperties(HWND hWnd, int bufferWidth, int bufferHeight)
{
	// set back buffer properties
    ZeroMemory( &m_swapChainDescription, sizeof(m_swapChainDescription) );

    m_swapChainDescription.BufferCount = 3;
    m_swapChainDescription.BufferDesc.Width = bufferWidth;
    m_swapChainDescription.BufferDesc.Height = bufferHeight;
    m_swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_swapChainDescription.BufferDesc.RefreshRate.Numerator = 0;
    m_swapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
    m_swapChainDescription.BufferUsage = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
    m_swapChainDescription.OutputWindow = hWnd;
    m_swapChainDescription.SampleDesc.Count = 1;
    m_swapChainDescription.SampleDesc.Quality = 0;
	m_swapChainDescription.Windowed = !mIsFullScreen;
	m_swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
	m_swapChainDescription.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	m_swapChainDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
}

HRESULT Graphics::CreateRenderTargetViews(int bufferWidth, int bufferHeight)
{
	HRESULT hr = S_OK;

	// create back buffer render target view
    // Get the pointer to the back buffer.
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)&m_pBackBuffer);
	if(FAILED(hr))
	{
		LOG_ERROR("Swap Chain get back buffer failure");
		GAME_ASSERT(false);
		return false;
	}

	hr = m_pd3dDevice->CreateRenderTargetView( m_pBackBuffer, NULL , &m_pBackBufferRenderTargetView );
	if( FAILED( hr ) )
	{
		LOG_ERROR("Render Target View create failure");
		GAME_ASSERT(false);
        return false;
	}

	D3D10_TEXTURE2D_DESC desc1;
	m_pBackBuffer->GetDesc(&desc1);
	desc1.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
    m_pBackBuffer->Release();

	hr = m_pd3dDevice->CreateTexture2D( &desc1, NULL, &m_pPreProcessTexture );

	if (FAILED(hr))
	{
		LOG_ERROR("Pre process render texture create failure");
		GAME_ASSERT(false);
		return false;
	}

	D3D10_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = desc1.Format;
	renderTargetViewDesc.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	hr = m_pd3dDevice->CreateRenderTargetView( m_pPreProcessTexture, &renderTargetViewDesc, &m_pPreProcessRenderTargetView );
	if ( FAILED( hr ) )
	{
		LOG_ERROR("Render Target View create failure");
		GAME_ASSERT(false);
		return false;
	}

	D3D10_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = desc1.Format;
	shaderResourceViewDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	hr = m_pd3dDevice->CreateShaderResourceView(m_pPreProcessTexture, &shaderResourceViewDesc, &m_preProcessSRV);
	if(FAILED(hr))
	{
		LOG_ERROR("Pre process texture shader resource view create failure");
		GAME_ASSERT(false);
		return false;
	}

	return hr;
}

void Graphics::ShowMessageBox(wchar_t* message, wchar_t* title)
{
	MessageBoxExW(m_hWnd, message, title, NULL, NULL);
}

void Graphics::CreateDefaultRasterState()
{
	// rasteriser settings - TODO: move this somewhere else
	D3D10_RASTERIZER_DESC rasterizerState;
	rasterizerState.CullMode = D3D10_CULL_NONE;
	rasterizerState.FillMode = D3D10_FILL_SOLID;
	rasterizerState.FrontCounterClockwise = true;
	rasterizerState.DepthBias = false;
	rasterizerState.DepthBiasClamp = 0;
	rasterizerState.SlopeScaledDepthBias = 0;
	rasterizerState.DepthClipEnable = true;
	rasterizerState.ScissorEnable = false;
	rasterizerState.MultisampleEnable = false;
	rasterizerState.AntialiasedLineEnable = true;
	m_pd3dDevice->CreateRasterizerState( &rasterizerState, &m_defaultRasterState);
}

void Graphics::SetDefaultRasterState()
{
	m_pd3dDevice->RSSetState(m_defaultRasterState);
}

void Graphics::EnableAlphaBlending()
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_pd3dDevice->OMSetBlendState(m_alphaEnabledBlendState, blendFactor, 0xffffffff);
}

void Graphics::DisableAlphaBlending()
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_pd3dDevice->OMSetBlendState(m_alphaDisabledBlendState, blendFactor, 0xffffffff);
}

void Graphics::CreateAlphaEnabledBlendState()
{
	D3D10_BLEND_DESC BlendState;
	ZeroMemory(&BlendState, sizeof(D3D10_BLEND_DESC));
	 
	BlendState.BlendEnable[0] = TRUE;
	BlendState.SrcBlend = D3D10_BLEND_SRC_ALPHA;
	BlendState.DestBlend = D3D10_BLEND_INV_SRC_ALPHA;
	BlendState.BlendOp = D3D10_BLEND_OP_ADD;
	BlendState.SrcBlendAlpha = D3D10_BLEND_ZERO;
	BlendState.DestBlendAlpha = D3D10_BLEND_ZERO;
	BlendState.BlendOpAlpha = D3D10_BLEND_OP_ADD;
	BlendState.AlphaToCoverageEnable = false; // false as default 
	BlendState.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;

	m_pd3dDevice->CreateBlendState(&BlendState, &m_alphaEnabledBlendState);
}

void Graphics::CreateAlphaDisabledBlendState()
{
	// create blend state
	D3D10_BLEND_DESC BlendState;
	ZeroMemory(&BlendState, sizeof(D3D10_BLEND_DESC));
	 
	BlendState.BlendEnable[0] = FALSE;
	BlendState.SrcBlend = D3D10_BLEND_SRC_ALPHA;
	BlendState.DestBlend = D3D10_BLEND_INV_SRC_ALPHA;
	BlendState.BlendOp = D3D10_BLEND_OP_ADD;
	BlendState.SrcBlendAlpha = D3D10_BLEND_ZERO;
	BlendState.DestBlendAlpha = D3D10_BLEND_ZERO;
	BlendState.BlendOpAlpha = D3D10_BLEND_OP_ADD;
	BlendState.AlphaToCoverageEnable = false; // false as default 
	BlendState.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;

	m_pd3dDevice->CreateBlendState(&BlendState, &m_alphaDisabledBlendState);
}

void Graphics::EnableAlphaToCoverage()
{
	/*if(!m_alphaToCoverageEnabled)
	{
		D3D10_BLEND_DESC blendState;
		m_currentBlendState->GetDesc(&blendState);

		m_currentBlendState->Release();

		blendState.AlphaToCoverageEnable = true;

		m_pd3dDevice->CreateBlendState(&blendState, &m_currentBlendState);
		m_pd3dDevice->OMSetBlendState(m_currentBlendState, 0, 0xffffffff);

		m_alphaToCoverageEnabled = true;
	}*/
}

void Graphics::DisableAlphaToCoverage()
{
	/*if(m_alphaToCoverageEnabled)
	{
		D3D10_BLEND_DESC blendState;
		m_currentBlendState->GetDesc(&blendState);
		
		m_currentBlendState->Release();

		blendState.AlphaToCoverageEnable = false;

		m_pd3dDevice->CreateBlendState(&blendState, &m_currentBlendState);
		m_pd3dDevice->OMSetBlendState(m_currentBlendState, 0, 0xffffffff);

		m_alphaToCoverageEnabled = false;
	}*/
}

void Graphics::SwitchToPreProcessRenderTarget()
{
	m_pd3dDevice->OMSetRenderTargets( 1, &m_pPreProcessRenderTargetView, m_pDepthStencilView );
}

void Graphics::SwitchToBackBufferRenderTarget()
{
	m_pd3dDevice->OMSetRenderTargets( 1, &m_pBackBufferRenderTargetView , nullptr );
}

void Graphics::DrawDebugText(const char * text, float top, float left)
{
	mSimpleFontManager->DrawDebugText(text, top, left);
}

#include "precompiled.h"
#include "EffectLightTextureBump.h"

EffectLightTextureBump::EffectLightTextureBump(void): EffectLightTexture(),
	m_pTextureBumpRV(0),
	m_pBumpVariable(0)
{
	FileName = L"ShaderFiles\\LightTextureBump.fx";

	// default texture name
	// TextureFileName  = "Media\\texture.dds";
}

EffectLightTextureBump::~EffectLightTextureBump(void)
{
	if(m_pTextureBumpRV)
	{
		// release the texture resource
		m_pTextureBumpRV->Release();
		m_pTextureBumpRV = NULL;
	}
}

void EffectLightTextureBump::Load(ID3D10Device* device)
{
	HRESULT hr = S_OK;

	// load our base class first
	EffectLightTexture::Load(device);

	// set a default light direction and color
	SetLightDirection((float*)D3DXVECTOR4(0,1,0,1));
	SetLightColor((float*)D3DXVECTOR4(1,1,1,1));

	m_pBumpVariable =m_pEffect->GetVariableByName( "txBump" )->AsShaderResource();

	// set the current technique to the default "render"
	CurrentTechnique = m_pEffect->GetTechniqueByName("Render");
}

void EffectLightTextureBump::SetBumpTexture(ID3D10ShaderResourceView * texBump)
{
	if(texBump != m_pTextureBumpRV)
	{
		m_pTextureBumpRV = texBump;
		m_pBumpVariable->SetResource(m_pTextureBumpRV);
	}
}

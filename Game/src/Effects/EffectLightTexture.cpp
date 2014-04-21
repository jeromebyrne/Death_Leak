#include "precompiled.h"
#include "EffectLightTexture.h"

EffectLightTexture::EffectLightTexture(void):EffectBasic()
{
	// set the shader name
	FileName = L"ShaderFiles\\BasicLightTexture.fx";

	m_pTextureRV = 0;
	m_pDiffuseVariable = 0;
	m_pLightDirVariable = 0;
	m_pLightColorVariable = 0;

	// default texture name
	TextureFileName  = "Media\\texture.dds";

}

EffectLightTexture::~EffectLightTexture(void)
{
	if(m_pTextureRV)
	{
		// release the texture resource
		m_pTextureRV->Release();
		m_pTextureRV = NULL;
	}
}

void EffectLightTexture::Load(ID3D10Device* device)
{
	HRESULT hr = S_OK;

	// load our base class first
	EffectBasic::Load(device);
	
	// get the lighting variables
	m_pLightDirVariable = m_pEffect->GetVariableByName( "lightDir" )->AsVector();
    m_pLightColorVariable = m_pEffect->GetVariableByName( "lightColor" )->AsVector();

	// get the texture diffuse variable
	m_pDiffuseVariable =m_pEffect->GetVariableByName( "txDiffuse" )->AsShaderResource();

	// set a default light direction and color
	SetLightDirection((float*)D3DXVECTOR4(0,1,0,1));
	SetLightColor((float*)D3DXVECTOR4(1,1,1,1));

	// set the current technique to the default "render"
	CurrentTechnique = m_pEffect->GetTechniqueByName("Render");
}

void EffectLightTexture::SetTexture(ID3D10ShaderResourceView * tex)
{
	if(tex != m_pTextureRV)
	{
		m_pTextureRV = tex;
		m_pDiffuseVariable->SetResource(m_pTextureRV);
	}
}


#include "precompiled.h"
#include "EffectLightTextureVertexWobble.h"

EffectLightTextureVertexWobble::EffectLightTextureVertexWobble(void):EffectLightTexture()
{
	FileName = L"ShaderFiles\\BasicLightTexture_VertexWobble.fx";

	ID3D10EffectScalarVariable*         m_pWavinessVariable = NULL;
	ID3D10EffectScalarVariable*         m_pTimeVariable = NULL;
}

EffectLightTextureVertexWobble::~EffectLightTextureVertexWobble(void)
{
}

void EffectLightTextureVertexWobble:: Load(ID3D10Device * device)
{
	EffectLightTexture::Load(device);

	// get the new vars for this effect
	m_pWavinessVariable = m_pEffect->GetVariableByName( "Waviness" )->AsScalar();
    m_pTimeVariable = m_pEffect->GetVariableByName( "Time" )->AsScalar();

	// set a value
	SetWobbleIntensity(5);
}

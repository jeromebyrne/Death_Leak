#include "precompiled.h"
#include "EffectLightTexturePixelWobble.h"

EffectLightTexturePixelWobble::EffectLightTexturePixelWobble(void) : EffectLightTexture()
{
	FileName = L"ShaderFiles\\BasicLightTexture_PixelWobble.fx";

	m_pWavinessVariable = nullptr;
	m_pTimeVariable = nullptr;
	m_pSpeedMod = nullptr;
}

EffectLightTexturePixelWobble::~EffectLightTexturePixelWobble(void)
{
}

void EffectLightTexturePixelWobble::Load(ID3D10Device * device)
{
	EffectLightTexture::Load(device);

	// get the new vars for this effect
	m_pWavinessVariable = m_pEffect->GetVariableByName( "Intensity" )->AsScalar();
    m_pTimeVariable = m_pEffect->GetVariableByName( "Time" )->AsScalar();
	m_pSpeedMod = m_pEffect->GetVariableByName( "SpeedMod" )->AsScalar();
}

void EffectLightTexturePixelWobble::ReverseMotion()
{
	float waviness = 0.0f;
	m_pWavinessVariable->GetFloat(&waviness);
	m_pWavinessVariable->SetFloat(waviness * -1);

	float speedMod = 0.0f;
	m_pSpeedMod->GetFloat(&speedMod);
	m_pSpeedMod->SetFloat(speedMod * -1);
}

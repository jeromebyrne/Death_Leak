#include "precompiled.h"
#include "EffectNoise.h"


EffectNoise::EffectNoise(void)
{
	FileName = L"ShaderFiles\\Noise.fx";
}


EffectNoise::~EffectNoise(void)
{
}


void EffectNoise::Load(ID3D10Device* device)
{
	EffectLightTexture::Load(device);

	mTimer = m_pEffect->GetVariableByName( "fTimer" )->AsScalar();
    mNoiseAmount = m_pEffect->GetVariableByName( "fNoiseAmount" )->AsScalar();
	mSeed = m_pEffect->GetVariableByName( "iSeed" )->AsScalar();
}


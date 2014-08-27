#include "precompiled.h"
#include "EffectFoliageSway.h"

EffectFoliageSway::EffectFoliageSway(void) : EffectLightTexture()
{
	FileName = L"ShaderFiles\\FoliageSway.fx";

	mTimeVariable = nullptr;
	mTimeMultiplier = nullptr;
	mGlobalTimeMultiplier = nullptr;
}

EffectFoliageSway::~EffectFoliageSway(void)
{
}

void EffectFoliageSway::Load(ID3D10Device * device)
{
	EffectLightTexture::Load(device);

	mTimeVariable = m_pEffect->GetVariableByName("Time")->AsScalar();
	mTimeMultiplier = m_pEffect->GetVariableByName("TimeMultiplier")->AsScalar();
	mGlobalTimeMultiplier = m_pEffect->GetVariableByName("GlobalTimeMultiplier")->AsScalar();
}

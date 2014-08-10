#include "precompiled.h"
#include "EffectFoliageSway.h"

EffectFoliageSway::EffectFoliageSway(void) :EffectLightTexture()
{
	FileName = L"ShaderFiles\\FoliageSway.fx";

	ID3D10EffectScalarVariable*         m_pTimeVariable = NULL;
}

EffectFoliageSway::~EffectFoliageSway(void)
{
}

void EffectFoliageSway::Load(ID3D10Device * device)
{
	EffectLightTexture::Load(device);

    m_pTimeVariable = m_pEffect->GetVariableByName( "Time" )->AsScalar();
}

#include "precompiled.h"
#include "EffectSepia.h"

EffectSepia::EffectSepia(void) : EffectLightTexture()
{
	FileName = L"ShaderFiles\\Sepia.fx";
}

EffectSepia::~EffectSepia(void)
{
	
}

void EffectSepia::Load(ID3D10Device* device)
{
	EffectLightTexture::Load(device);
}



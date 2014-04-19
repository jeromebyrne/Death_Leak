#include "precompiled.h"
#include "EffectMonochrome.h"

EffectMonochrome::EffectMonochrome(void):EffectLightTexture()
{
	FileName = L"ShaderFiles\\Monochrome.fx";
}

EffectMonochrome::~EffectMonochrome(void)
{
}

void EffectMonochrome:: Load(ID3D10Device * device)
{
	EffectLightTexture::Load(device);
}
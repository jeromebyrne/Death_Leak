#include "precompiled.h"
#include "EffectManager.h"
#include "EffectBasic.h"
#include "EffectLightTexture.h"
#include "EffectLightTextureVertexWobble.h"
#include "EffectReflection.h"
#include "EffectParticleSpray.h"
#include "EffectSepia.h"
#include "EffectMonochrome.h"
#include "EffectMonochromeRed.h"
#include "EffectBloodParticleSpray.h"
#include "EffectLightTextureBump.h"
#include "EffectNoise.h"
#include "EffectLightTexturePixelWobble.h"

EffectManager* EffectManager::m_instance = nullptr;

EffectManager::EffectManager(void)
{
}

void EffectManager::Release()
{
	// just call the destructor
	delete this;
}

EffectManager::~EffectManager(void)
{
	// release all of our texture resources
	for(auto current : m_effectMap)
	{
		if(current.second)
		{
			delete current.second;
			current.second = nullptr;
		}
	}
}

void EffectManager::Initialise(Graphics * graphicsSystem)
{
	// get the graphics device
	ID3D10Device * device = graphicsSystem->Device();

	// create
	EffectBasic * effectBasic = new EffectBasic();
	EffectLightTexture * effectLightTexture = new EffectLightTexture();
	EffectLightTextureVertexWobble * effectLightTextureWobble = new EffectLightTextureVertexWobble();
	EffectReflection * effectReflection = new EffectReflection();
	EffectParticleSpray * effectParticleSpray = new EffectParticleSpray();
	EffectSepia * effectSepia = new EffectSepia();
	EffectMonochrome * effectMonochrome = new EffectMonochrome();
	EffectMonochromeRed * effectMonochromeRed = new EffectMonochromeRed();
	EffectBloodParticleSpray * effectBloodParticleSpray = new EffectBloodParticleSpray();
	EffectLightTextureBump * effectLightTextureBump = new EffectLightTextureBump();
	EffectNoise * effectNoise = new EffectNoise();
	EffectLightTexturePixelWobble * effectLightTexturePIxelWobble = new EffectLightTexturePixelWobble();

	// load them
	effectBasic->Load(device);
	effectLightTexture->Load(device);
	effectLightTextureWobble->Load(device);
	effectReflection->Load(device);
	effectParticleSpray->Load(device);
	effectSepia->Load(device);
	effectMonochrome->Load(device);
	effectMonochromeRed->Load(device);
	effectBloodParticleSpray->Load(device);
	effectLightTextureBump->Load(device);
	effectNoise->Load(device);
	effectLightTexturePIxelWobble->Load(device);

	// set input layouts
	effectBasic->SetInputLayout(effectBasic->CurrentTechnique, device, graphicsSystem->InputDescriptions.POS_COLOR, 2); 
	effectLightTexture->SetInputLayout(effectLightTexture->CurrentTechnique, device, graphicsSystem->InputDescriptions.POS_TEXCOORD_NORM, 3);
	effectLightTextureWobble->SetInputLayout(effectLightTextureWobble->CurrentTechnique, device, graphicsSystem->InputDescriptions.POS_TEXCOORD_NORM, 3);
	effectReflection->SetInputLayout(effectReflection->CurrentTechnique, device, graphicsSystem->InputDescriptions.POS_TEXCOORD_NORM, 3);
	effectParticleSpray->SetInputLayout(effectParticleSpray->CurrentTechnique, device, graphicsSystem->InputDescriptions.POS_TEXCOORD_NORM, 3);
	effectSepia->SetInputLayout(effectSepia->CurrentTechnique, device, graphicsSystem->InputDescriptions.POS_TEXCOORD_NORM, 3);
	effectMonochrome->SetInputLayout(effectMonochrome->CurrentTechnique, device, graphicsSystem->InputDescriptions.POS_TEXCOORD_NORM, 3);
	effectMonochromeRed->SetInputLayout(effectMonochromeRed->CurrentTechnique, device, graphicsSystem->InputDescriptions.POS_TEXCOORD_NORM, 3);
	effectBloodParticleSpray->SetInputLayout(effectBloodParticleSpray->CurrentTechnique, device, graphicsSystem->InputDescriptions.POS_TEXCOORD_NORM, 3);
	effectLightTextureBump->SetInputLayout(effectLightTextureBump->CurrentTechnique, device, graphicsSystem->InputDescriptions.POS_TEXCOORD_NORM_TAN_BINORM, 5);
	effectNoise->SetInputLayout(effectNoise->CurrentTechnique, device, graphicsSystem->InputDescriptions.POS_TEXCOORD_NORM, 3);
	effectLightTexturePIxelWobble->SetInputLayout(effectLightTexture->CurrentTechnique, device, graphicsSystem->InputDescriptions.POS_TEXCOORD_NORM, 3);

	// add to map
	m_effectMap["effectbasic"] = effectBasic;
	m_effectMap["effectlighttexture"] = effectLightTexture;
	m_effectMap["effectlighttexturevertexwobble"] = effectLightTextureWobble;
	m_effectMap["effectreflection"] = effectReflection;
	m_effectMap["effectparticlespray"] = effectParticleSpray;
	m_effectMap["effectsepia"] = effectSepia;
	m_effectMap["effectmonochrome"] = effectMonochrome;
	m_effectMap["effectmonochromered"] = effectMonochromeRed;
	m_effectMap["effectbloodparticlespray"] = effectBloodParticleSpray;
	m_effectMap["effectlighttexturebump"] = effectLightTextureBump;
	m_effectMap["effectnoise"] = effectNoise;
	m_effectMap["effectpixelwobble"] = effectLightTexturePIxelWobble;
}

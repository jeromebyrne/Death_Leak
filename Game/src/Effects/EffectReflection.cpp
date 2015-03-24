#include "precompiled.h"
#include "EffectReflection.h"

EffectReflection::EffectReflection(void) : EffectLightTexture()
{
	// set the shader file
	FileName = L"ShaderFiles\\BasicReflection.fx";

	// default environment map
	EnvironmentMapFile = "Media\\Lobby\\LobbyCube.dds";

	m_pEnvMapSRV = 0;
	m_pEnvMapVariable = 0;
}

EffectReflection::~EffectReflection(void)
{
	if(m_pEnvMapSRV)
		m_pEnvMapSRV->Release();
}

void EffectReflection::Load(ID3D10Device * device)
{
	EffectLightTexture::Load(device);
	
	// get the environment map variable
	m_pEnvMapVariable = m_pEffect->GetVariableByName( "txEnvMap" )->AsShaderResource();
	
	// load the environment map file
	// m_pEnvMapSRV = TextureManager::Instance()->LoadTexture(EnvironmentMapFile);

    // Set the Environment Map
    // m_pEnvMapVariable->SetResource( m_pEnvMapSRV );
}

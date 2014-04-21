#ifndef EFFECTREFLECTION_H
#define EFFECTREFLECTION_H

#include "effectlighttexture.h"

// contains all the same properties of EffectLightTexture and some more for reflection
class EffectReflection : public EffectLightTexture
{
private:
	// our environment map texture
	ID3D10ShaderResourceView* m_pEnvMapSRV;
	ID3D10EffectShaderResourceVariable* m_pEnvMapVariable;
public:

	char* EnvironmentMapFile;
	EffectReflection(void);
	virtual ~EffectReflection(void);
	virtual void Load(ID3D10Device * device);
};

#endif

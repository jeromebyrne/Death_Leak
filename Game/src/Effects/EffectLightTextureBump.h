#ifndef EFFECTLIGHTTEXTUREBUMP_H
#define EFFECTLIGHTTEXTUREBUMP_H

#include "effectlighttexture.h"
class EffectLightTextureBump :
	public EffectLightTexture
{
private:

	ID3D10ShaderResourceView* m_pTextureBumpRV;
	ID3D10EffectShaderResourceVariable* m_pBumpVariable;

public:
	EffectLightTextureBump(void);
	virtual ~EffectLightTextureBump(void);
	virtual void Load(ID3D10Device* device);

	void SetBumpTexture(ID3D10ShaderResourceView * tex);
};

#endif

#ifndef EFFECTLIGHTTEXTUREVERTEXWOBBLE_H
#define EFFECTLIGHTTEXTUREVERTEXWOBBLE_H

#include "effectlighttexture.h"

class EffectLightTextureVertexWobble : public EffectLightTexture
{
private:
	ID3D10EffectScalarVariable*         m_pWavinessVariable;
	ID3D10EffectScalarVariable*         m_pTimeVariable;
public:
	EffectLightTextureVertexWobble(void);
	virtual ~EffectLightTextureVertexWobble(void);
	virtual void Load(ID3D10Device * device);

	void SetWobbleIntensity(float value)
	{
		m_pWavinessVariable->SetFloat(value);
	}

	void SetTimeVariable(float delta)
	{
		m_pTimeVariable->SetFloat( delta );
	}
};

#endif

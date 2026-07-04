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
		if (m_pWavinessVariable != nullptr)
		{
			m_pWavinessVariable->SetFloat(value);
		}
	}

	void SetTimeVariable(float delta)
	{
		if (m_pTimeVariable != nullptr)
		{
			m_pTimeVariable->SetFloat(delta);
		}
	}
};

#endif

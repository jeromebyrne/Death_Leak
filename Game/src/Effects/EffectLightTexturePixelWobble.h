#ifndef EFFECTLIGHTTEXTUREPIXELWOBBLE_H
#define EFFECTLIGHTTEXTUREPIXELWOBBLE_H

#include "effectlighttexture.h"

class EffectLightTexturePixelWobble : public EffectLightTexture
{
public:
	EffectLightTexturePixelWobble(void);
	virtual ~EffectLightTexturePixelWobble(void);
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

	void SetSpeedMod(float value)
	{
		if (m_pSpeedMod != nullptr)
		{
			m_pSpeedMod->SetFloat(value);
		}
	}

	void ReverseMotion();

private:
	ID3D10EffectScalarVariable*         m_pWavinessVariable;
	ID3D10EffectScalarVariable*         m_pTimeVariable;
	ID3D10EffectScalarVariable*			m_pSpeedMod;
};

#endif

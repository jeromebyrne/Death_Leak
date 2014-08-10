#ifndef EFFECTFOLIAGESWAY_H
#define EFFECTFOLIAGESWAY_H

#include "effectlighttexture.h"

class EffectFoliageSway : public EffectLightTexture
{
public:

	EffectFoliageSway(void);
	virtual ~EffectFoliageSway(void);
	virtual void Load(ID3D10Device * device);

	void SetTimeVariable(float delta)
	{
		m_pTimeVariable->SetFloat(delta);
	}

private:

	ID3D10EffectScalarVariable*         m_pTimeVariable;
};

#endif

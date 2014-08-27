#ifndef EFFECTFOLIAGESWAY_H
#define EFFECTFOLIAGESWAY_H

#include "effectlighttexture.h"

class EffectFoliageSway : public EffectLightTexture
{
public:

	EffectFoliageSway(void);
	virtual ~EffectFoliageSway(void);
	virtual void Load(ID3D10Device * device);

	void SetTimeVariable(float time)
	{
		mTimeVariable->SetFloat(time);
	}

	void SetTimeMultiplier(float multiplier)
	{
		mTimeMultiplier->SetFloat(multiplier);
	}

	void SetGlobalTimeMultiplier(float multiplier)
	{
		mGlobalTimeMultiplier->SetFloat(multiplier);
	}

private:

	ID3D10EffectScalarVariable*         mTimeVariable;
	ID3D10EffectScalarVariable*         mTimeMultiplier;
	ID3D10EffectScalarVariable*         mGlobalTimeMultiplier;
};

#endif

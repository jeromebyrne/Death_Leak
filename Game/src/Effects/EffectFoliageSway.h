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
		if (mTimeVariable != nullptr)
		{
			mTimeVariable->SetFloat(time);
		}
	}

	void SetTimeMultiplier(float multiplier)
	{
		if (mTimeMultiplier != nullptr)
		{
			mTimeMultiplier->SetFloat(multiplier);
		}
	}

	void SetGlobalTimeMultiplier(float multiplier)
	{
		if (mGlobalTimeMultiplier != nullptr)
		{
			mGlobalTimeMultiplier->SetFloat(multiplier);
		}
	}

private:

	ID3D10EffectScalarVariable*         mTimeVariable;
	ID3D10EffectScalarVariable*         mTimeMultiplier;
	ID3D10EffectScalarVariable*         mGlobalTimeMultiplier;
};

#endif

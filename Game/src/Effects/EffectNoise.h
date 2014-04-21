#ifndef EFFECTNOISE_H
#define EFFECTNOISE_H

#include "effectlighttexture.h"

class EffectNoise : public EffectLightTexture
{
public:
	EffectNoise(void);
	virtual ~EffectNoise(void);

	virtual void Load(ID3D10Device* device) override;

	void SetTimer(float value)
	{
		mTimer->SetFloat(value);
	}

	void SetNoiseAmount(float value)
	{
		mNoiseAmount->SetFloat(value);
	}

	void SetSeed(int value)
	{
		mSeed->SetInt(value);
	}

private:

	ID3D10EffectScalarVariable* mTimer;
	ID3D10EffectScalarVariable* mNoiseAmount;
	ID3D10EffectScalarVariable* mSeed;
};

#endif


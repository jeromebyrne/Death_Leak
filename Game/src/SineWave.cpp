#include "precompiled.h"
#include "SineWave.h"

void SineWave::Initialise(float currentStep, float yOffset, float speed, float initialPoisitionY)
{
	mStep = currentStep;
	mOffsetY = yOffset;
	mSpeed = speed;
	mInitialPositionY = initialPoisitionY;
}

void SineWave::Update(float delta)
{
	mStep += mSpeed * delta;
}

float SineWave::GetValue() const
{
	return mInitialPositionY + (mOffsetY * sin(mStep));
}

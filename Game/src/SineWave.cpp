#include "precompiled.h"
#include "SineWave.h"

void SineWave::Initialise(float currentStep, float yOffset, float speed, float initialPoisitionY, float initialPositionX, float offsetX)
{
	mStep = currentStep;
	mOffsetY = yOffset;
	mSpeed = speed;
	mInitialPositionY = initialPoisitionY;
	mOffsetX = offsetX;
	mInitialPositionX = initialPositionX;
}

void SineWave::Update(float delta)
{
	mStep += mSpeed * delta;
}

float SineWave::GetValueY() const
{
	return mInitialPositionY + (mOffsetY * sin(mStep));
}

float SineWave::GetValueX() const
{
	return mInitialPositionX + (mOffsetX * sin(mStep));
}
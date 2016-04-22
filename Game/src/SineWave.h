#ifndef SINEWAVE_H
#define SINEWAVE_H

class SineWave
{
public:

	void Initialise(float currentStep, float yOffset, float speed, float initialPoisitionY, float initialPositionX, float offsetX);

	void Update(float delta);

	float GetValueY() const;

	float GetValueX() const;

private:

	float mStep;
	float mOffsetY;
	float mOffsetX;
	float mSpeed;
	float mInitialPositionY;
	float mInitialPositionX;
};

#endif

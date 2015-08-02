#ifndef SINEWAVE_H
#define SINEWAVE_H

class SineWave
{
public:

	void Initialise(float currentStep, float yOffset, float speed, float initialPoisitionY);

	void Update(float delta);

	float GetValue() const;

private:

	float mStep;
	float mOffsetY;
	float mSpeed;
	float mInitialPositionY;
};

#endif

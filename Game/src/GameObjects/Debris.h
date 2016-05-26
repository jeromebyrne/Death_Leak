#ifndef DEBRIS_H
#define DEBRIS_H

#include "solidmovingsprite.h"

class Debris :public SolidMovingSprite
{

public:

	enum DebrisState
	{
		kActive,
		kFadingOut
	};

	Debris(SolidMovingSprite * target,
		Vector3 position,
		Vector3 dimensions,
		Vector3 collisionDimensions,
		const char * textureFile,
		bool nativeDimensions,
		float speedMultiplier);
	virtual ~Debris(void);

	virtual void Update(float delta) override;

	virtual bool OnCollision(SolidMovingSprite* object) override;

	virtual void Initialise() override;

private:

	float mCreationTime;

	static unsigned long mLastTimePlayedSFX;

	DebrisState mCurrentDebrisState = kActive;

	float mTimeUntilFadeOutFully = 3.0f;

	Vector3 mOriginalDimensions;
};

#endif

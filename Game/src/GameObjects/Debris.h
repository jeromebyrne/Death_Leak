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
		Vector2 position,
		DepthLayer depthLayer,
		Vector2 dimensions,
		Vector2 collisionDimensions,
		const char * textureFile,
		bool nativeDimensions,
		float speedMultiplier);
	virtual ~Debris(void);

	virtual void Update(float delta) override;

	virtual bool OnCollision(SolidMovingSprite* object) override;

	virtual void Initialise() override;

	void SetCollidesWithOtherDebris(bool value) { mCollidesWithOtherDebris = value; }

private:

	float mCreationTime;

	static unsigned long mLastTimePlayedSFX;

	DebrisState mCurrentDebrisState = kActive;

	float mTimeUntilFadeOutFully = 4.0f;

	bool mHasUpdatedOnce = false;

	bool mCollidesWithOtherDebris = true;
};

#endif

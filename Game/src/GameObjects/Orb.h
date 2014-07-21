#ifndef ORB_H
#define ORB_H

#include "solidmovingsprite.h"

class Orb :public MovingSprite
{

public:

	Orb(void);
	Orb(SolidMovingSprite * target,
		Vector3 position,
		Vector3 dimensions,
		Vector3 collisionDimensions,
		const char * textureFile,
		bool nativeDimensions,
		float speedMultiplier);
	virtual ~Orb(void);

	virtual void Update(float delta) override;
	const float GetValue() const { return mValue; }

protected:

	SolidMovingSprite * m_physicalTarget;

private:
	
	// the closest distance the orb has come to it's target
	Vector3 m_closestDistanceToTarget;

	float mValue; // how much is this orb worth

	float mCreationTime;

	static unsigned long mLastTimePlayedSFX;

	virtual void OnCollideWithTarget();
};

#endif

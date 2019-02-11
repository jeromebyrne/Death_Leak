#ifndef FOLIAGE_H
#define FOLIAGE_H

#include "solidmovingsprite.h"

class Foliage : public SolidMovingSprite
{
public:
	Foliage(void);
	virtual ~Foliage(void);

	virtual bool OnCollision(SolidMovingSprite * object) override;

	virtual void LoadContent(ID3D10Device * graphicsdevice) override;

	virtual void Update(float delta) override;

private:

	float mOriginalSwayMultiplier = 1.0f;
	float mCollisionSwayMultiplier = 1.0f;
	float mCollisionSwayWindDownTime = 0.0f;
};

#endif


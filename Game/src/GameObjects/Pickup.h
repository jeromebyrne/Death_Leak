#ifndef PICKUP_H
#define PICKUP_H

#include "solidmovingsprite.h"

class Pickup : public SolidMovingSprite
{
public:

	virtual ~Pickup(void);

	virtual void DoPickup() {}

	virtual void Update(float delta) override;

	virtual void Initialise() override;

	virtual bool OnCollision(SolidMovingSprite * object) override;

	virtual void OnInteracted() override;

protected:

	bool mIsPickedUp = false;
};

#endif

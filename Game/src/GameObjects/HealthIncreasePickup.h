#ifndef HEALTHINCREASEPICKUP_H
#define HEALTHINCREASEPICKUP_H

#include "pickup.h"

class HealthIncreasePickup : public Pickup
{
public:

	virtual void DoPickup() override;

protected:

	void DoPickupEffects(Player * player);

	int mHealthIncreaseAmount = 20;
};

#endif

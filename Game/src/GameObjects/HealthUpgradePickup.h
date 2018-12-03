#ifndef HEALTHUPGRADEPICKUP_H
#define HEALTHUPGRADEPICKUP_H

#include "pickup.h"

class HealthUpgradePickup : public Pickup
{
public:

	virtual void DoPickup() override;

	virtual void Initialise() override;

protected:

	void DoPickupEffects(Player * player);

	int mHealthUpgradeAmount = 20;
};

#endif

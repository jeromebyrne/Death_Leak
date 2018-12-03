#ifndef FOCUSUPGRADEPICKUP_H
#define FOCUSUPGRADEPICKUP_H

#include "pickup.h"

class FocusUpgradePickup : public Pickup
{
public:

	virtual void DoPickup() override;
	virtual void Initialise() override;

protected:

	void DoPickupEffects(Player * player);

	int mFirstFocusUpgradeAmount = 100;
	int mFocusUpgradeAmount = 20;
};

#endif

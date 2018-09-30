#ifndef FOCUSUPGRADEPICKUP_H
#define FOCUSUPGRADEPICKUP_H

#include "pickup.h"

class FocusUpgradePickup : public Pickup
{
public:

	virtual void DoPickup() override;

protected:

	void DoPickupEffects(Player * player);

	int mFocusUpgradeAmount = 20;
};

#endif

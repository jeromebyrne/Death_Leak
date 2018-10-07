#ifndef KEYPICKUP_H
#define KEYPICKUP_H

#include "pickup.h"

class KeyPickup : public Pickup
{
public:

	virtual void DoPickup() override;

	void SetKeyId(const string & keyId) { mKeyId = keyId; }

protected:

	void DoPickupEffects(Player * player);

	string mKeyId;
};

#endif

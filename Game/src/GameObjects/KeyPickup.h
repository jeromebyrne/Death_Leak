#ifndef KEYPICKUP_H
#define KEYPICKUP_H

#include "pickup.h"

class KeyPickup : public Pickup
{
public:

	virtual void DoPickup() override;

	void SetKeyId(const string & keyId) { mKeyId = keyId; }

	void XmlRead(TiXmlElement * element) override;

	void XmlWrite(TiXmlElement * element) override;

	void Update(float delta) override;

protected:

	void DoPickupEffects(Player * player);

	string mKeyId;
	string mKeyNameLocId;
	string mKeyDescLocId;
};

#endif

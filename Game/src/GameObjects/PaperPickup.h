#ifndef PAPERPICKUP_H
#define PAPERPICKUP_H

#include "pickup.h"

class PaperPickup : public Pickup
{
public:

	virtual void DoPickup() override;

	virtual void Initialise() override;

	virtual void XmlRead(TiXmlElement * element) override;

	virtual void XmlWrite(TiXmlElement * element) override;

	virtual void Update(float delta) override;

protected:

	string mLocTitleId;
	string mLocTitleString;
	string mLocDescId;
	string mLocDescString;

	bool mHasInitCheckedCollected = false;
};

#endif

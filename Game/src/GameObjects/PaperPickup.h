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

protected:

	string mLocalizationId;
	string mLocalizedString;
};

#endif

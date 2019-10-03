#ifndef SFXPICKUP_H
#define SFXPICKUP_H

#include "pickup.h"

class SfxPickup : public Pickup
{
public:

	virtual void DoPickup() override;

	void XmlRead(TiXmlElement * element) override;

	void XmlWrite(TiXmlElement * element) override;

	void Update(float delta) override;

	void Initialise() override;

	bool CanInteract() override;

private:

	void PlayRandomSfx();

	vector<string> mSfxList;
};

#endif

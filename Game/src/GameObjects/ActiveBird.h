#ifndef ACTIVEBIRD_H
#define ACTIVEBIRD_H

#include "AmbientBird.h"

class ActiveBird : public AmbientBird
{
public:

	ActiveBird(void);
	virtual ~ActiveBird(void);

	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector3 pointOfContact, bool shouldExplode) override;
	virtual void Initialise() override;
};

#endif
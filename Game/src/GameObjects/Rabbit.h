#ifndef RABBIT_H
#define RABBIT_H

#include "npc.h"

class Rabbit : public NPC
{
public:
	Rabbit(void);
	virtual ~Rabbit(void);

	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode) override;
	virtual void Initialise() override;
};

#endif
#ifndef BUTTERFLY_H
#define BUTTERFLY_H

#include "npc.h"

class Butterfly : public NPC
{
public:
	Butterfly(void);
	virtual ~Butterfly(void);

	virtual void OnDamage(float damageAmount, Vector3 pointOfContact, bool shouldExplode) override;
	virtual void Initialise() override;
	virtual void UpdateAnimations() override;
};

#endif
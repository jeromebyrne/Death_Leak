#ifndef BUTTERFLY_H
#define BUTTERFLY_H

#include "npc.h"

class Butterfly : public NPC
{
public:
	Butterfly(void);
	virtual ~Butterfly(void);

	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode) override;
	virtual void Initialise() override;
	virtual void UpdateAnimations() override;
	virtual bool OnCollision(SolidMovingSprite * object) override;
};

#endif
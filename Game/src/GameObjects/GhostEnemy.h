#ifndef GHOSTENEMY_H
#define GHOSTENEMY_H

#include "npc.h"

class GhostEnemy : public NPC
{
public:

	friend class GhostEnemySpawner;

	GhostEnemy(void);
	virtual ~GhostEnemy(void);

	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode) override;
	virtual void Initialise() override;
	virtual bool OnCollision(SolidMovingSprite * object) override;
	virtual void Update(float delta) override;

	Projectile * FireWeapon(Vector2 direction, float speedMultiplier) override;

private:

	float mProjectileSpeed = 12.0f;

};

#endif
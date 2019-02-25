#ifndef SKELETONENEMY_H
#define SKELETONENEMY_H

#include "npc.h"

class SkeletonEnemy : public NPC
{
public:

	friend class SkeletonEnemySpawner;

	SkeletonEnemy(void);
	virtual ~SkeletonEnemy(void);

	virtual void Initialise() override;
	virtual void Update(float delta) override;

	bool CanJump() const override { return false; }
	bool CanRoll() const override { return false; }
	bool CanTeleport() const override { return false; }

	Projectile * FireWeapon(Vector2 direction) override;

private:

};

#endif
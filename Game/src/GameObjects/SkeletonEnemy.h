#ifndef SKELETONENEMY_H
#define SKELETONENEMY_H

#include "npc.h"

class SkeletonEnemy : public NPC
{
public:

	friend class SkeletonEnemySpawner;

	SkeletonEnemy(void);
	virtual ~SkeletonEnemy(void);

	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode) override;
	virtual void Initialise() override;
	// virtual void UpdateAnimations() override;
	virtual bool OnCollision(SolidMovingSprite * object) override;
	virtual void Update(float delta) override;

	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

private:

	float mProjectileSpeed = 6.0f;

};

#endif
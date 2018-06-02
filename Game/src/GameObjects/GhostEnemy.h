#ifndef GHOSTENEMY_H
#define GHOSTENEMY_H

#include "npc.h"

class GhostEnemy : public NPC
{
public:

	friend class GhostEnemySpawner;

	GhostEnemy(void);
	virtual ~GhostEnemy(void);

	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector3 pointOfContact, bool shouldExplode) override;
	virtual void Initialise() override;
	// virtual void UpdateAnimations() override;
	virtual bool OnCollision(SolidMovingSprite * object) override;
	virtual void Update(float delta) override;

	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

	Projectile * FireWeapon(Vector2 direction) override;

private:

	float mProjectileSpeed = 6.0f;

};

#endif
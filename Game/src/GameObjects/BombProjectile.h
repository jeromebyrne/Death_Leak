#ifndef BOMBPROJECTILE_H
#define BOMBPROJECTILE_H

#include "projectile.h"
class BombProjectile : public Projectile
{

public:

	BombProjectile(ProjectileOwnerType ownerType, 
					const char* textureFileName,
					const char * impactTextureFilename,
					Vector2 position,
					Vector2 dimensions,
					Vector2 collisionDimensions,
					Vector2 direction,
					float damage = 50,
					float speed = 3,
					int maxTimeInActive = 2000);

	virtual ~BombProjectile(void);

	virtual bool OnCollision(SolidMovingSprite* object) override;

	virtual void Update(float delta) override;

	void HandleSolidLineStripCollision(SolidLineStrip * solidLineStrip);

private:

	float mTimeUntilNextParticleSpray;
};

#endif

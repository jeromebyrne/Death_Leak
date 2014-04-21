#ifndef BOMBPROJECTILE_H
#define BOMBPROJECTILE_H

#include "projectile.h"
class BombProjectile : public Projectile
{

public:

	BombProjectile(const char* textureFileName , const char * impactTextureFilename, GameObject * owner, Vector3 position, Vector2 dimensions, Vector2 collisionDimensions,
				Vector2 direction,float damage = 50, float speed = 3, int maxTimeInActive = 2000);
	virtual ~BombProjectile(void);

	virtual void OnCollision(SolidMovingSprite* object) override;

	virtual void Update(float delta) override;
};

#endif

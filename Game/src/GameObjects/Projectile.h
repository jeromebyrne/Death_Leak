#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "solidmovingsprite.h"
#define MAX_PROJECTLES_ALLOWED 20 // the maximum number of projectiles we want alive

class Projectile : public SolidMovingSprite
{
protected:
	static int NUM_PROJECTILES_ACTIVE; // the number of projectiles currently alive

	GameObject * m_owner; // who owns this projectile
	bool m_isActive; // are we active (ie: do update and onCollision)
	bool m_wasActiveLastFrame; // we were active in the last frame
	float m_timeBecameInactive; // the time at which this projectile became inactive 
	float m_maxTimeInActive; // the maximum time to be inactive (when we exceed the max we just dissappear)
	float m_damage; // the amount of damage this projectile inflicts
	
	string m_impactTextureFilename;
	ID3D10ShaderResourceView * m_impactTexture; // change to this texture on impact

	bool mCollidedWithProjectile;
	bool mSpinningMovement; 
public:

	friend class BombProjectile;

	Projectile(const char* textureFileName , const char * impactTextureFilename, GameObject * owner, Vector3 position, Vector2 dimensions, Vector2 collisionDimensions,
				Vector2 direction,float damage = 5, float speed = 10, int maxTimeInActive = 2.0f);
	virtual ~Projectile(void);

	virtual void OnCollision(SolidMovingSprite* object) override;
	virtual void Update(float delta) override;
	virtual void Draw(ID3D10Device * device, Camera2D * camera) override;
	virtual void Scale(float xScale, float yScale, bool scalePosition = true) override;
	virtual void LoadContent(ID3D10Device * graphicsdevice) override;

	void SetSpinningMovement(bool value) { mSpinningMovement = value; }

	void OnOwnerDead();
};

#endif

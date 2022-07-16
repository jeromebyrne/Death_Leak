#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "solidmovingsprite.h"
#define MAX_PROJECTLES_ALLOWED 20 // the maximum number of projectiles we want alive

class SolidLineStrip;

class Projectile : public SolidMovingSprite
{
public:

	enum ProjectileType
	{
		kUnknownProjectileType,
		kBladeProjectile,
		kBombProjectile,
		kBloodFXProjectile,
		kLiquid
	};

	enum ProjectileOwnerType
	{
		kPlayerProjectile,
		kNPCProjectile,
		kUnknownProjectile
	};

	friend class BombProjectile;

	Projectile(ProjectileOwnerType ownerType, 
				const string & textureFileName, 
				const string & impactTextureFilename,
				Vector2 position,
				DepthLayer depthLayer,
				Vector2 dimensions,
				Vector2 collisionDimensions,
				Vector2 direction,
				float damage = 5.0f,
				float speed = 10.0f,
				float maxTimeInActive = 2.0f);

	virtual ~Projectile(void);

	virtual bool OnCollision(SolidMovingSprite* object) override;
	virtual void Update(float delta) override;
	virtual void Scale(float xScale, float yScale, bool scalePosition = true) override;
	virtual void LoadContent(ID3D10Device * graphicsdevice) override;
	virtual void DebugDraw(ID3D10Device * graphicsdevice) override;

	void SetSpinningMovement(bool value) { mSpinningMovement = value; mRotateToDirection = !mSpinningMovement; }

	ProjectileType GetProjectileType() const { return mType; }
	void SetProjectileType(ProjectileType t) { mType = t; }
	ProjectileOwnerType GetOwnerType() const { return mOwnerType; }
	void SetOwnerType(ProjectileOwnerType type) { mOwnerType = type; }

	Vector2 GetCollisionRayStart() const;
	Vector2 GetCollisionRayEnd() const;

	Vector2 GetLastFrameCollisionRayStart();
	// Vector2 GetLastFrameCollisionRayEnd();

	float GetSpeed() const { return mSpeed; }

	bool IsActive() const { return m_isActive; }

	void SetDamage(float damageAmount)
	{
		m_damage = damageAmount;
	}

	void SetProjectileResistance(float value) { mProjectileResistance = value; }

	void SetIsDeflectable(bool value) { mIsDeflectable = value; }

	bool isDeflectable() const;

	void SetDoAlphaFadeOut(bool value) { mDoAlphaFadeOut = value; }

	void SetDoScaleFadeOut(bool value) { mDoScaleFadeOut = value; }

	void SetShouldRotateToDirection(bool value) { mRotateToDirection = value; }

	void SetWasDeflected(bool value) { mWasDeflected = value; }

	bool WasDeflected() const { return mWasDeflected; }

protected:

	void HandleSolidLineStripCollision(SolidLineStrip * solidLineStrip);

	void ReboundOffSolidLine(SolidLineStrip * solidLine, unsigned lineIndex);

	void DoBloodProjectilePositionJitter();

	void DoRotateToDirection();

	static int NUM_PROJECTILES_ACTIVE; // the number of projectiles currently alive

	bool m_isActive; // are we active (ie: do update and onCollision)
	bool m_wasActiveLastFrame; // we were active in the last frame
	float m_timeBecameInactive; // the time at which this projectile became inactive 
	float m_maxTimeInActive; // the maximum time to be inactive (when we exceed the max we just dissappear)
	float m_damage; // the amount of damage this projectile inflicts
	
	string m_impactTextureFilename;
	ID3D10ShaderResourceView * m_impactTexture; // change to this texture on impact

	bool mCollidedWithProjectile;
	bool mSpinningMovement; 

	ProjectileType mType;
	ProjectileOwnerType mOwnerType;

	float mReboundRotateRate;

	bool mTrailParticlesAttached;

	float mSpeed = 1.0f;

	float mProjectileResistance = 1.0f;

	bool mIsDeflectable = true; // most projectile are deflectable

	bool mDoAlphaFadeOut = true;
	bool mDoScaleFadeOut = false;
	bool mRotateToDirection = true;

	float mTimeActive = 0.0f;

	bool mWasDeflected = false;
};

#endif

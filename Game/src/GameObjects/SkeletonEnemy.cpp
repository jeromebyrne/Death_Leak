#include "precompiled.h"
#include "SkeletonEnemy.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "material.h"
#include "Debris.h"
#include "AIStateFollow.h"
#include "AIStateRepel.h"
#include "Game.h"
#include "Projectile.h"

SkeletonEnemy::SkeletonEnemy(void) :
	NPC()
{
	mProjectileFilePath = "Media/fireball.png";
	mProjectileImpactFilePath = "Media/fireball.png";
	mExplodesGruesomely = true;
	mHealth = 500.0f;
	mMaxHealth = 500.0f;
	mEmitsBlood = false;
	mFireProjectileRandOffsetMax = -100.0f;

	mRunAnimFramerateMultiplier = 4.0f;
	m_projectileOffset.X = 10.0f;
	m_projectileOffset.Y = 50.0f;
	mCheckNPCOverlapCollisions = false;
}

SkeletonEnemy::~SkeletonEnemy(void)
{
}

void SkeletonEnemy::Initialise()
{
	NPC::Initialise();

	SetState(AIState::kRangeAttack);
}

void SkeletonEnemy::Update(float delta)
{
	NPC::Update(delta);

	if (Game::GetIsLevelEditMode())
	{
		return;
	}
}

Projectile * SkeletonEnemy::FireWeapon(Vector2 direction)
{
	Vector2 pos = m_position;
	pos.X = (direction.X > 0) ? pos.X + m_projectileOffset.X : pos.X -= m_projectileOffset.X;
	pos.Y += m_projectileOffset.Y;

	if (direction.X > 0)
	{
		pos.X += m_projectileOffset.X;
	}
	else
	{
		pos.X -= m_projectileOffset.X;
	}

	// TODO: ideally want these properties configurable per character
	Projectile * p = new Projectile(Projectile::kNPCProjectile,
									mProjectileFilePath.c_str(),
									mProjectileImpactFilePath.c_str(),
									pos,
									GameObject::kGhostVomitProjectile,
									Vector2(95.625, 62.625),
									Vector2(25.0f, 8.0f),
									direction,
									2.0f,
									5.0f,
									2.0f);

	p->SetIsNativeDimensions(false);
	p->SetSpinningMovement(false);
	p->SetProjectileResistance(1.0f);
	p->SetIsDeflectable(false);
	p->SetProjectileType(Projectile::kLiquid);
	p->SetApplyGravity(false);

	if (!WasInWaterLastFrame())
	{
		PlayRandomWeaponFireSound();
	}

	return p;
}

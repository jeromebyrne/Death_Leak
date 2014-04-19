#include "precompiled.h"
#include "BombProjectile.h"
#include "Explosion.h"
#include "ParticleEmitterManager.h"
#include "NPC.h"
#include "Orb.h"
#include "AudioManager.h"
#include "waterblock.h"

BombProjectile::BombProjectile(const char* textureFileName , const char * impactTextureFilename, GameObject * owner, Vector3 position, Vector2 dimensions, Vector2 collisionDimensions,
				Vector2 direction,float damage, float speed, int maxTimeInActive):
Projectile(textureFileName , impactTextureFilename, owner, position, dimensions, collisionDimensions,
			 direction, damage, speed, maxTimeInActive)
{
	mSpinningMovement = true;
	mBouncable = true;
	mBounceDampening = 0.35f;

	AudioManager::Instance()->PlaySoundEffect("fuse_burning.wav");
}

BombProjectile::~BombProjectile(void)
{
	Explosion * explosion = new Explosion(m_damage, 700, m_position.X, m_position.Y, m_position.Z - 0.1);

	GameObjectManager::Instance()->AddDrawableObject_RunTime(explosion);
}

void BombProjectile::OnCollision(SolidMovingSprite* object)
{
	if(m_owner &&
		object != m_owner && 
		!dynamic_cast<Orb*>(object))
	{
		NPC * objAsNPC = dynamic_cast<NPC *>(object);
		if (objAsNPC && dynamic_cast<NPC *>(m_owner) ||
			dynamic_cast<WaterBlock*>(object))
		{
			// npc projectiles don't damage other npc's
			return;
		}

		Projectile * objAsProj = dynamic_cast<Projectile *>(object);
		if (objAsProj)
		{
			Player * player = GameObjectManager::Instance()->GetPlayer();
			if (objAsProj->m_owner == player && 
				this->m_owner != player)
			{
				// set the player as the owner of the projectile and fire it back at the enemy
				Vector3 enemyPosition = m_owner->Position();

				Vector3 direction = enemyPosition - m_position;
				direction.Normalise();
				m_velocity = Vector3(direction.X * 17, direction.Y * 17, 1);
				objAsProj->SetVelocityXYZ(-m_velocity.X * 0.5, -5, 0);

				ParticleEmitterManager::Instance()->CreateRadialSpray(10, m_position, Vector3(3000,3000, 1), "Media/spark.png", 2, 5, 0.4f, 0.6f, 30, 40, 2.0, false, 0.8,0.9,0,true, 4.0);
				AudioManager::Instance()->PlaySoundEffect("metalclink.wav");

				objAsProj->mCollidedWithProjectile = true;
				m_owner = player;
			}
			return;
		}

		if (!dynamic_cast<Character *> (object))
		{
			if (m_isActive)
			{
				m_isActive = false;
				m_timeBecameInactive = Timing::Instance()->GetTotalTimeSeconds();
			}

			SolidMovingSprite::OnCollision(object);
		}
	}
}

void BombProjectile::Update(float delta)
{
	float targetDelta =  Timing::Instance()->GetTargetDelta();
	float percentDelta = delta / targetDelta;

	if (percentDelta > 1.4f)
	{
		percentDelta = 1.4f; 
	}

	// apply gravity to the velocity
	if (!mIsInWater)
	{
		m_velocity.Y -= 0.3f * percentDelta;
	}
	else
	{
		m_velocity.Y -= 0.06f * percentDelta;
	}


	// nice simple update
	if (mIsInWater)
	{
		m_velocity.X *= 0.92f; // slow down significantly
		//m_velocity.Y *= 0.9f; // slow down significantly
	}
	m_position += m_velocity * percentDelta;

	// we dont need complicated movement so we'll ignore the MovingSprite class
	Sprite::Update(delta);

	int groundLevel = Environment::Instance()->GroundLevel();
	
	float bottom = Bottom();
	if (bottom <= groundLevel || GetIsCollidingOnTopOfObject())
	{
		if (m_isActive)
		{
			m_isActive = false;
			m_timeBecameInactive = Timing::Instance()->GetTotalTimeSeconds();
		}
		m_isOnGround = true;
		if(bottom < groundLevel) // if below ground level then set at ground level
		{
			m_position.Y = (groundLevel + m_dimensions.Y/2);
			StopYAccelerating();
			m_velocity.Y = 0;
		}

		if (!mIsInWater)
		{
			SetRotationAngle(GetRotationAngle() + ((m_velocity.X * -0.04) *percentDelta));
		}
		else
		{
			SetRotationAngle(GetRotationAngle() + ((m_velocity.X * -0.009) *percentDelta));
		}
	}
	else
	{
		m_isOnGround = false;

		if (!mIsInWater)
		{
			SetRotationAngle(GetRotationAngle() + ((m_velocity.X * -0.01) * percentDelta));
		}
		else
		{
			SetRotationAngle(GetRotationAngle() + ((m_velocity.X * -0.005) * percentDelta));
		}
	}

	if (!m_isActive)
	{
		float currentTime = Timing::Instance()->GetTotalTimeSeconds();
		float timeToDie = m_timeBecameInactive + m_maxTimeInActive;

		if(currentTime > timeToDie)
		{
			// time to kill ourselves
			GameObjectManager::Instance()->RemoveGameObject_RunTime(this);
		}
	}
}
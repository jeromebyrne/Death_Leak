#include "precompiled.h"
#include "BombProjectile.h"
#include "Explosion.h"
#include "ParticleEmitterManager.h"
#include "NPC.h"
#include "Orb.h"
#include "AudioManager.h"
#include "waterblock.h"
#include "SolidLineStrip.h"
#include "Material.h"

static const float kParticleDelay = 0.1f;

BombProjectile::BombProjectile(ProjectileOwnerType ownerType, 
								const char* textureFileName, 
								const char * impactTextureFilename,
								Vector3 position, 
								Vector2 dimensions,
								Vector2 collisionDimensions,
								Vector2 direction,
								float damage,
								float speed,
								int maxTimeInActive):
Projectile(ownerType,textureFileName, 
			impactTextureFilename,
			position,
			dimensions,
			collisionDimensions,
			 direction, 
			 damage,
			 speed,
			 maxTimeInActive),
			 mTimeUntilNextParticleSpray(0.0f)
{
	mSpinningMovement = true;
	mBouncable = true;
	mBounceDampening = 0.35f;

	mType = kBombProjectile;

	AudioManager::Instance()->PlaySoundEffect("fuse_burning.wav");
}

BombProjectile::~BombProjectile(void)
{
	Explosion * explosion = new Explosion(m_damage, 700, m_position.X, m_position.Y, m_position.Z - 0.1);

	GameObjectManager::Instance()->AddGameObject(explosion);

	if (Camera2D::GetInstance()->IsObjectInView(this))
	{
		Camera2D::GetInstance()->DoBigShake();
	}
}

void BombProjectile::OnCollision(SolidMovingSprite* object)
{
	if (object->IsOrb())
	{
		return;
	}

	if (object->IsWaterBlock())
	{
		return;
	}

	if (object->IsCharacter())
	{
		return;
	}

	if (object->IsSolidLineStrip())
	{
		SolidLineStrip * lineStrip = static_cast<SolidLineStrip*>(object);
		HandleSolidLineStripCollision(lineStrip);
		return;
	}

	if (m_isActive)
	{
		m_isActive = false;
		m_timeBecameInactive = Timing::Instance()->GetTotalTimeSeconds();
	}

	SolidMovingSprite::OnCollision(object);
}

void BombProjectile::Update(float delta)
{
	float targetDelta =  Timing::Instance()->GetTargetDelta();
	float percentDelta = delta / targetDelta;

	if (percentDelta > 1.4f)
	{
		percentDelta = 1.4f; 
	}

	// apply gravity to the 
	if (!mIsOnSolidLine)
	{
		if (!mIsInWater)
		{
			m_velocity.Y -= 0.3f * percentDelta;
		}
		else
		{
			m_velocity.Y -= 0.06f * percentDelta;
		}
	}

	// nice simple update
	if (mIsInWater)
	{
		m_velocity.X *= 0.92f; // slow down significantly
		//m_velocity.Y *= 0.9f; // slow down significantly
	}

	m_direction = m_velocity;
	m_direction.Normalise();

	m_position += m_velocity * percentDelta;

	// we dont need complicated movement so we'll ignore the MovingSprite class
	Sprite::Update(delta);

	if (IsOnSolidSurface())
	{
		// StopYAccelerating();

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
			GameObjectManager::Instance()->RemoveGameObject(this);
		}
	}

	if (mTimeUntilNextParticleSpray > 0.0f)
	{
		mTimeUntilNextParticleSpray -= delta;

		if (mTimeUntilNextParticleSpray < 0.0f)
		{
			mTimeUntilNextParticleSpray = 0.0f;
		}
	}
}

void BombProjectile::HandleSolidLineStripCollision(SolidLineStrip * solidLineStrip)
{
	Vector3 collisionPosition;

	if (solidLineStrip->GetBombProjectileCollisionData(this, collisionPosition))
	{
		if (m_velocity.Y < 0.0f && !WasInWaterLastFrame())
		{
			// bounce
			m_velocity.X *= 0.9f;
			m_velocity.Y *= -0.5f;
		}
		else if (WasInWaterLastFrame())
		{
			m_velocity.X *= 0.3f;
			m_velocity.Y *= -0.2f;
		}


		float diffY = collisionPosition.Y - CollisionBottom();

		SetY(solidLineStrip->Position().Y - collisionPosition.Y);

		if (m_isActive)
		{
			m_timeBecameInactive = Timing::Instance()->GetTotalTimeSeconds();
		}
		
		if (mSpinningMovement)
		{
			SetRotationAngle(0);
		}

		// damage the other object
		if (!mIsInWater)
		{
			// OnDamage(m_damage, collisionPosition);
		}

		Material * objectMaterial = solidLineStrip->GetMaterial();
		if (objectMaterial != nullptr)
		{
			// where should the particles spray from
			Vector3 particlePos = solidLineStrip->Position() - collisionPosition;
			particlePos.Z = m_position.Z - 0.01f;

			// show particles
			bool loop = false;
			float minLive = 0.35f;
			float maxLive = 0.75f;

			if (m_isActive)
			{
				// play sound for non-characters, characters handle their sounds in OnDamage
				string soundFile = objectMaterial->GetRandomDamageSoundFilename();
				AudioManager::Instance()->PlaySoundEffect(soundFile);
			}
			
			if (m_velocity.X > 5.0f && mTimeUntilNextParticleSpray == 0.0f)
			{
				string particleTexFile = objectMaterial->GetRandomParticleTexture();
				ParticleEmitterManager::Instance()->CreateDirectedSpray(5,
																		particlePos,
																		Vector3(-m_direction.X, -m_direction.Y, 0),
																		0.4,
																		Vector3(3200, 1200, 0),
																		particleTexFile,
																		1.0f,
																		4.0f,
																		minLive,
																		maxLive,
																		10,
																		30,
																		0.7,
																		loop,
																		0.7f,
																		1.0f,
																		10.0f,
																		true,
																		3.5f,
																		1.5f,
																		3.0f,
																		0.15f,
																		0.6f);

				mTimeUntilNextParticleSpray = kParticleDelay;
			}
			
		}

		m_isActive = false;
	}
}

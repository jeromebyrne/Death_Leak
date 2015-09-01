#include "precompiled.h"
#include "Orb.h"
#include "GameObjectManager.h"
#include "particleemittermanager.h"
#include "material.h"
#include "materialmanager.h"
#include "audiomanager.h"

static const float kTargetRange = 250;
static const float kCollisionRange = 40;

unsigned long Orb::mLastTimePlayedSFX = 0;

static const float kMinLiveTimeUntilCollisionsActive = 1.0f;
static const float kMinTimeBetweenSFX = 0.5f;

Orb::Orb(SolidMovingSprite * target, 
		 Vector3 position, 
		 Vector3 dimensions, 
		 Vector3 collisionDimensions, 
		 const char * textureFile,
		 bool nativeDimensions,
		 float speedMultiplier) :
	MovingSprite(position.X, position.Y, position.Z, dimensions.X, dimensions.Y, dimensions.Z),
	m_physicalTarget(target),
	m_closestDistanceToTarget(2000000000, 2000000000, 2000000000),
	mValue(1),
	mCreationTime(0.0f)
{
	mIsOrb = true;

	// TODO: notify the target that it has orbs following it so that it can cancel before it dies
	// m_collisionBoxDimensions = collisionDimensions;
	m_textureFilename = textureFile;

	m_applyGravity = true;

	SetMaxVelocityXYZ(25 * speedMultiplier, 25 * speedMultiplier, 0);
	
	m_resistance.X = 0.99;
	m_resistance.Y = 2.5;

	srand(ID() * 500);
	
	float directionX = ((rand()%99) + 1) * 0.01;
	float directionY = ((rand()%99) + 1) * 0.01;

	float rand_speed = ((rand()%25) + 1) * speedMultiplier;

	bool plus_sign = (bool)(rand()%2);

	if (!plus_sign)
	{
		directionX = -directionX;
	}

	SetVelocityXYZ(directionX * rand_speed, directionY * rand_speed, 0);

	m_drawAtNativeDimensions = nativeDimensions;
	
	// set default material
	m_material = MaterialManager::Instance()->GetMaterial("default_orb");

	mCreationTime = Timing::Instance()->GetTotalTimeSeconds();
}

Orb::Orb(void)
{
}

Orb::~Orb(void)
{
}

void Orb::OnCollideWithTarget()
{
	float currentTime = Timing::Instance()->GetTotalTimeSeconds();

	if (currentTime > mCreationTime + kMinLiveTimeUntilCollisionsActive)
	{
		GameObjectManager::Instance()->RemoveGameObject(this);
		
		if (m_material)
		{
			// show particles when we make contact
			string particleName = m_material->GetRandomParticleTexture();
			
			ParticleEmitterManager::Instance()->CreateRadialSpray(5,
																  Vector3(m_physicalTarget->X(), m_physicalTarget->Y(), m_physicalTarget->Z() - 0.1f),
																  Vector3(3200, 1200, 0),
																  particleName,
																  0.2,
																  2,
																  0.4f,
																  0.7f,
																  75,
																  150,
																  0.5,
																  false,
																  0.8,
																  1.0,
																  0.8f,
																  true, 
																  0.1,
																  0.15f,
																  0.8f,
																  5,
																  5);
		}

		// play sound effect
		if (currentTime > mLastTimePlayedSFX + kMinTimeBetweenSFX)
		{
			if (m_material)
			{
				string soundFile = m_material->GetRandomDamageSoundFilename();
				AudioManager::Instance()->PlaySoundEffect(soundFile);
			}

			mLastTimePlayedSFX = currentTime;
		}
	}
}

void Orb::Update(float delta)
{
	MovingSprite::Update(delta);

	if (m_physicalTarget && Timing::Instance()->GetTotalTimeSeconds() > mCreationTime + kMinLiveTimeUntilCollisionsActive)
	{
		// accelerate towards the target
		Vector3 direction = m_physicalTarget->Position() - m_position;

		// if within range then move towards the player
		float distance = direction.LengthSquared();
		if (distance < kTargetRange * kTargetRange)
		{
			if (distance < kCollisionRange * kCollisionRange)
			{
				OnCollideWithTarget();
			}
			else
			{
				direction.Normalise();

				m_direction = direction;

				AccelerateX(m_direction.X, 1.0f);

				AccelerateY(m_direction.Y, 1.0f);

				if ((m_direction.X < 0 && m_velocity.X > 0) ||
					(m_direction.X > 0 && m_velocity.X < 0))
				{
					// the velocity of the orb is still moving in the opposite x direction
					// let's give it a helping hand to catch up by accelerating harshly
					AccelerateX(m_direction.X, 3.0f);
				}

				if ((m_direction.Y < 0 && m_velocity.Y > 0) ||
					(m_direction.Y > 0 && m_velocity.Y < 0))
				{
					// the velocity of the orb is still moving in the opposite y direction
					// let's give it a helping hand to catch up by accelerating harshly
					AccelerateY(m_direction.Y, 3.0f);
				}
			}
		}
	}

	float targetDelta =  Timing::Instance()->GetTargetDelta();
	float percentDelta = delta / targetDelta;

	SetRotationAngle(m_rotationAngle - (m_velocity.X * 0.02) * percentDelta);
}
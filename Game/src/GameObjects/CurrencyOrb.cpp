#include "precompiled.h"
#include "CurrencyOrb.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "material.h"
#include "Game.h"
#include "MaterialManager.h"

static float kTrackingRangeTrigger = 200.0f;
static float kAccelerateRate = 1.8f;
static float kHarshAccelerateRate = 3.5f;
static float kCollisionRange = 50.0f;
static const float kMinTimeBetweenSFX = 0.5f;

unsigned long CurrencyOrb::mLastTimePlayedSFX = 0;

CurrencyOrb::CurrencyOrb(void) :
	SolidMovingSprite(),
	mCurrentState(kIdle),
	mIsLargeType(false),
	mParticleTrailObjectId(-1)
{
	mIsCurrencyOrb = true;
}

CurrencyOrb::~CurrencyOrb(void)
{
}

void CurrencyOrb::OnDamage(GameObject * damageDealer, float damageAmount, Vector3 pointOfContact, bool shouldExplode)
{
}

void CurrencyOrb::Initialise()
{
	mSineWaveProps.RandomiseInitialStep = true;
	mSineWaveProps.Amplitude = 2.0f;
	mSineWaveProps.OffsetY = 20.0f;

	SolidMovingSprite::Initialise();

	m_passive = false;
	m_applyGravity = false;
	m_resistance.X = 1.0f;
	m_resistance.Y = 3.0f;
	m_maxVelocity.X = 20.0f;

	m_material = MaterialManager::Instance()->GetMaterial("flame_orb");

	if (m_drawAtNativeDimensions)
	{
		mIsLargeType = true;
	}
}

bool CurrencyOrb::OnCollision(SolidMovingSprite * object)
{
	Player * player = GameObjectManager::Instance()->GetPlayer();
	if (object != player)
	{
		return false;
	}

	Vector3 direction = player->Position() - m_position;

	// if within range then move towards the player
	float distance = direction.LengthSquared();

	if (distance < (kCollisionRange * kCollisionRange))
	{
		if (m_material)
		{
			// show particles when we make contact
			string particleName = m_material->GetRandomParticleTexture();

			ParticleEmitterManager::Instance()->CreateRadialSpray(10,
																Vector3(m_position.X + player->VelocityX() * 5, m_position.Y + player->VelocityY() * 2, player->Z() - 0.02f),
																Vector3(3200, 1200, 0),
																particleName,
																1.0,
																2.4,
																0.3f,
																0.65f,
																40,
																80,
																0.5,
																false,
																0.8,
																1.0,
																0.8f,
																true,
																2.0f,
																0.15f,
																0.8f,
																3,
																3);

			ParticleEmitterManager::Instance()->CreateRadialSpray(1,
																Vector3(m_position.X + player->VelocityX() * 5, m_position.Y + player->VelocityY() * 2, player->Z() - 0.01f),
																Vector3(3200, 1200, 0),
																"Media\\blast_circle.png",
																1.0,
																2.4,
																0.35f,
																0.35f,
																20,
																40,
																0.5,
																false,
																0.8,
																1.0,
																0.8f,
																true,
																5.5f,
																0.15f,
																0.8f,
																0,
																0);
		}

		float currentTime = Timing::Instance()->GetTotalTimeSeconds();

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

		player->SetShowBurstTint(true);
		player->SetburstTintStartTime(Timing::Instance()->GetTotalTimeSeconds());

		if (mParticleTrailObjectId != -1)
		{
			shared_ptr<GameObject> & particleObj = GameObjectManager::Instance()->GetObjectByID(mParticleTrailObjectId);
			if (particleObj)
			{
				particleObj->Detach();

				GameObjectManager::Instance()->RemoveGameObject(particleObj.get());
			}
		}

		GameObjectManager::Instance()->RemoveGameObject(this);
	}

	return true;
}

void CurrencyOrb::Update(float delta)
{
	SolidMovingSprite::Update(delta);

	if (Game::GetIsLevelEditMode())
	{
		return;
	}

	switch (mCurrentState)
	{
		case kIdle:
		{
			DoIdleHover(delta);

			Player * player = GameObjectManager::Instance()->GetPlayer();
			if (player)
			{
				Vector3 direction = player->Position() - m_position;

				// if within range then move towards the player
				float distance = direction.LengthSquared();
				if (distance < kTrackingRangeTrigger * kTrackingRangeTrigger)
				{
					// player tracking triggered
					mCurrentState = kTracking;

					AddTrailParticles();
				}
			}

			break;
		}
		case kTracking:
		{
			DoTrackPlayer(delta);
			break;
		}
	}
}

void CurrencyOrb::DoIdleHover(float delta)
{
	mSineWaveProps.DoSineWave = true;
}

void CurrencyOrb::DoTrackPlayer(float delta)
{
	mSineWaveProps.DoSineWave = false;

	Player * player = GameObjectManager::Instance()->GetPlayer();
	if (!player)
	{
		return;
	}

	// accelerate towards the target
	Vector3 direction = Vector3(player->CollisionCentreX(), player->CollisionCentreY(), player->Z()) - m_position;

	direction.Normalise();

	m_direction = direction;

	bool prioritiseX = false;
	if (std::abs(m_direction.X) > std::abs(m_direction.Y))
	{
		AccelerateX(m_direction.X, kAccelerateRate);
		prioritiseX = true;
	}
	else
	{
		AccelerateY(m_direction.Y, kAccelerateRate);
	}

	if (prioritiseX && ((m_direction.X < 0 && m_velocity.X > 0) ||
		(m_direction.X > 0 && m_velocity.X < 0)))
	{
		// the velocity of the orb is still moving in the opposite x direction
		// let's give it a helping hand to catch up by accelerating harshly
		AccelerateX(m_direction.X, kHarshAccelerateRate);
	}

	if (!prioritiseX && ((m_direction.Y < 0 && m_velocity.Y > 0) ||
		(m_direction.Y > 0 && m_velocity.Y < 0)))
	{
		// the velocity of the orb is still moving in the opposite y direction
		// let's give it a helping hand to catch up by accelerating harshly
		AccelerateY(m_direction.Y, kHarshAccelerateRate);
	}

	Vector2 dir = Vector2(m_velocity.X, m_velocity.Y);
	dir.Normalise();

	if (dir.X > 0)
	{
		SetRotationAngle(-acos(dir.Dot(Vector2(0, -1))));
	}
	else
	{
		SetRotationAngle(acos(dir.Dot(Vector2(0, -1))));
	}
}

void CurrencyOrb::DoCollisionSmallType()
{

}

void CurrencyOrb::DoCollisionLargeType()
{

}

void CurrencyOrb::AddTrailParticles()
{
	if (m_material)
	{
		// show particles when we make contact
		string particleName = m_material->GetRandomParticleTexture();

		ParticleSpray * p = ParticleEmitterManager::Instance()->CreateRadialSpray(40,
																					m_position,
																					Vector3(3200, 1200, 0),
																					particleName,
																					1.5,
																					3.4,
																					0.1f,
																					0.35f,
																					50,
																					100,
																					0.5,
																					true,
																					0.8,
																					1.0,
																					-1.0f,
																					true,
																					0.1f,
																					0.15f,
																					0.8f,
																					3,
																					3);

		if (p)
		{
			p->AttachTo(GameObjectManager::Instance()->GetObjectByID(ID()), Vector3(0, 0, 0.1f), true);
			mParticleTrailObjectId = p->ID();
		}
	}
}
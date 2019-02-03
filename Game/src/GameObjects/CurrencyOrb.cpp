#include "precompiled.h"
#include "CurrencyOrb.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "material.h"
#include "Game.h"
#include "MaterialManager.h"
#include "SaveManager.h"

static float kTrackingRangeTrigger = 150.0f;
static float kAccelerateRate = 2.1f;
static float kHarshAccelerateRate = 3.8f;
static float kCollisionRange = 80.0f;
static const float kMinTimeBetweenSFX = 0.2f;

unsigned long CurrencyOrb::mLastTimePlayedSFX = 0;

CurrencyOrb::CurrencyOrb(void) :
	SolidMovingSprite(),
	mCurrentState(kIdle),
	mIsLargeType(false),
	mParticleTrailObjectId(-1),
	mSpawnPeriodTime(0.1f),
	mTimeTracking(0.0f),
	mIsLoadTimeObject(false)
{
	mIsCurrencyOrb = true;
}

CurrencyOrb::~CurrencyOrb(void)
{
}

void CurrencyOrb::OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode)
{
}

void CurrencyOrb::Initialise()
{
	mSineWaveProps.DoSineWave = true;
	mSineWaveProps.RandomiseInitialStep = true;
	mSineWaveProps.Amplitude = 2.0f;
	mSineWaveProps.OffsetX = 0.0f;
	mSineWaveProps.OffsetY = 20.0f;
	mSineWaveProps.InitialXPosition = m_position.X;
	mSineWaveProps.InitialYPosition = m_position.Y;

	mSinWave.Initialise(rand() % 999999,
						mSineWaveProps.OffsetY,
						mSineWaveProps.Amplitude,
						mSineWaveProps.InitialYPosition,
						mSineWaveProps.InitialXPosition,
						mSineWaveProps.OffsetX);

	m_alpha = 0.75f;

	SolidMovingSprite::Initialise();

	m_passive = false;
	m_applyGravity = false;
	m_resistance.X = 1.0f;
	m_resistance.Y = 3.0f;
	m_maxVelocity.X = 20.0f;

	if (m_drawAtNativeDimensions) // This is a TERRIBLE way to determine this
	{
		mIsLargeType = true;
	}

	m_material = mIsLargeType ? MaterialManager::Instance()->GetMaterial("flame_orb_large") : MaterialManager::Instance()->GetMaterial("flame_orb");

	SetDepthLayer(GameObject::kOrb);
}

bool CurrencyOrb::OnCollision(SolidMovingSprite * object)
{
#if _DEBUG
	if (Game::GetInstance()->GetIsLevelEditMode())
	{
		return false;
	}
#endif

	if (mCurrentState == kSpawnPeriod)
	{
		return false;
	}

	Player * player = GameObjectManager::Instance()->GetPlayer();
	if (object != player)
	{
		return false;
	}

	Vector2 direction = player->Position() - m_position;

	// if within range then move towards the player
	float distance = direction.LengthSquared();

	if (mIsLargeType || distance < (kCollisionRange * kCollisionRange))
	{
		mIsLargeType ? DoCollisionLargeType(player) : DoCollisionSmallType(player);

		// player->SetShowBurstTint(true);
		// player->SetburstTintStartTime(Timing::Instance()->GetTotalTimeSeconds());

		if (mParticleTrailObjectId != -1)
		{
			shared_ptr<GameObject> & particleObj = GameObjectManager::Instance()->GetObjectByID(mParticleTrailObjectId);
			if (particleObj)
			{
				particleObj->Detach();

				static_cast<ParticleSpray*>(particleObj.get())->SetIsLooping(false);
			}
		}

		GameObjectManager::Instance()->RemoveGameObject(this);

		SaveManager::GetInstance()->SetNumCurrencyOrbsCollected(SaveManager::GetInstance()->GetNumCurrencyOrbsCollected() + 1);

		if (m_material)
		{
			AudioManager::Instance()->PlaySoundEffect(m_material->GetRandomFootstepSoundFilename());
		}
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

			if (!mIsLargeType)
			{

				Player * player = GameObjectManager::Instance()->GetPlayer();
				if (player)
				{
					Vector2 direction = player->Position() - m_position;

					// if within range then move towards the player
					float distance = direction.LengthSquared();
					if (distance < kTrackingRangeTrigger * kTrackingRangeTrigger)
					{
						// player tracking triggered
						mCurrentState = kTracking;

						AddTrailParticles();

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
					}
				}
			}
			break;
		}
		case kTracking:
		{
			mTimeTracking += delta;
			DoTrackPlayer(delta);
			break;
		}
		case kSpawnPeriod:
		{
			mSpawnPeriodTime -= delta;
			if (mSpawnPeriodTime < 0.0f)
			{
				mSpawnPeriodTime = 0.0f;

				mCurrentState = kTracking;

				AddTrailParticles();
			}
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
	Vector2 direction = Vector2(player->CollisionCentreX(), player->CollisionCentreY()) - m_position;

	direction.Normalise();

	m_direction = direction;

	float multiplier = mTimeTracking > 2.0f ? 3.0f : 1.0f;

	bool prioritiseX = false;
	if (std::abs(m_direction.X) > std::abs(m_direction.Y))
	{
		AccelerateX(m_direction.X, kAccelerateRate * multiplier);
		prioritiseX = true;
	}
	else
	{
		AccelerateY(m_direction.Y, kAccelerateRate * multiplier);
	}

	if (prioritiseX && ((m_direction.X < 0 && m_velocity.X > 0) ||
		(m_direction.X > 0 && m_velocity.X < 0)))
	{
		// the velocity of the orb is still moving in the opposite x direction
		// let's give it a helping hand to catch up by accelerating harshly
		AccelerateX(m_direction.X, kHarshAccelerateRate * multiplier);
	}

	if (!prioritiseX && ((m_direction.Y < 0 && m_velocity.Y > 0) ||
		(m_direction.Y > 0 && m_velocity.Y < 0)))
	{
		// the velocity of the orb is still moving in the opposite y direction
		// let's give it a helping hand to catch up by accelerating harshly
		AccelerateY(m_direction.Y, kHarshAccelerateRate * multiplier);
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

void CurrencyOrb::DoCollisionSmallType(Player * player)
{
	if (mIsLoadTimeObject)
	{
		GameObjectManager::Instance()->SetCurrencyOrbCollected(ID());
	}
}

void CurrencyOrb::DoCollisionLargeType(Player * player)
{
	if (mIsLoadTimeObject)
	{
		GameObjectManager::Instance()->SetCurrencyOrbCollected(ID());
	}

	if (m_material)
	{
		// show particles when we make contact
		string particleName = m_material->GetRandomParticleTexture();

		ParticleEmitterManager::Instance()->CreateRadialSpray(20,
																Vector2(m_position.X + player->VelocityX() * 5, m_position.Y + player->VelocityY() * 2),
																GetDepthLayer(),
																Vector2(3200.0f, 1200.0f),
																particleName,
																3.0f,
																7.4f,
																0.4f,
																0.6f,
																50.0f,
																100.0f,
																0.5f,
																false,
																0.8f,
																1.0f,
																0.8f,
																true,
																3.0f,
																0.15f,
																0.8f,
																5.0f,
																5.0f);

		Vector2 position(m_position.X, m_position.Y);
		SpawnOrbs(position, 10);
	}
}

void CurrencyOrb::AddTrailParticles()
{
	if (m_material)
	{
		// show particles when we make contact
		string particleName = m_material->GetRandomParticleTexture();

		ParticleSpray * p = nullptr;

		if (!mIsLargeType)
		{
			p = ParticleEmitterManager::Instance()->CreateRadialSpray(15,
																		m_position,
																		GetDepthLayer(),
																		Vector2(3200.0f, 1200.0f),
																		particleName,
																		1.5f,
																		3.4f,
																		0.1f,
																		0.35f,
																		40.0f,
																		80.0f,
																		0.5f,
																		true,
																		0.8f,
																		1.0f,
																		-1.0f,
																		true,
																		0.1f,
																		0.15f,
																		0.8f,
																		3.0f,
																		3.0f);
		}
		else
		{
			p = ParticleEmitterManager::Instance()->CreateRadialSpray(40,
																		m_position,
																		GetDepthLayer(),
																		Vector2(3200.0f, 1200.0f),
																		particleName,
																		1.8f,
																		3.6f,
																		0.1f,
																		0.35f,
																		200.0f,
																		300.0f,
																		0.5f,
																		true,
																		0.8f,
																		1.0f,
																		-1.0f,
																		true,
																		0.1f,
																		0.15f,
																		0.8f,
																		4.0f,
																		5.0f);
		}

		if (p)
		{
			p->AttachTo(GameObjectManager::Instance()->GetObjectByID(ID()), Vector2(0.0f, 0.0f), GetDepthLayer(), true);
			mParticleTrailObjectId = p->ID();
		}
	}
}

void CurrencyOrb::SpawnOrbs(Vector2 & position, unsigned int numOrbs)
{
	std::string animFile = "XmlFiles\\orb_anim.xml";

	for (unsigned int i = 0; i < numOrbs; ++i)
	{
		CurrencyOrb * newOrb = new CurrencyOrb();
		newOrb->m_dimensions.X = 47.697300;
		newOrb->m_dimensions.Y = 72.831207;
		newOrb->m_alpha = 0.8f;
		newOrb->m_drawAtNativeDimensions = false;
		newOrb->m_collisionBoxDimensions.X = 20.0f;
		newOrb->m_collisionBoxDimensions.Y = 20.0f;
		newOrb->mCollisionBoxOffset.Y = 10;
		newOrb->m_isAnimated = true;
		newOrb->m_animationFile = animFile;
		newOrb->SetXY(position.X, position.Y);
		newOrb->SetDepthLayer(GameObject::kOrb);
		newOrb->m_maxVelocity.X = 20.0f;
		newOrb->m_maxVelocity.Y = 99999;
		newOrb->SetVelocityXY((((rand() % 100) * 0.18f)) * ((rand() % 2) == 1 ? -1.0f : 1.0f),
			(((rand() % 100) * 0.3f) + 30.0f) * ((rand() % 2) == 1 ? -1.0f : 1.0f));

		GameObjectManager::Instance()->AddGameObject(newOrb);

		newOrb->FlipVertical();
		newOrb->mSpawnPeriodTime = ((rand() % 100) * 0.001f) + 0.01f;
		newOrb->mCurrentState = kSpawnPeriod;
	}
}
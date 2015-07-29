#include "precompiled.h"
#include "CurrencyOrb.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "material.h"

static float kTrackingRangeTrigger = 500.0f;
static float kAccelerateRate = 1.0f;
static float kHarshAccelerateRate = 1.1f;

CurrencyOrb::CurrencyOrb(void) :
	SolidMovingSprite(),
	mCurrentState(kIdle)
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
	SolidMovingSprite::Initialise();

	m_passive = false;
	m_applyGravity = false;
	m_resistance.X = 1.0f;
	m_resistance.Y = 3.0f;
}

bool CurrencyOrb::OnCollision(SolidMovingSprite * object)
{
	// return SolidMovingSprite::OnCollision(object);
	return false;
}

void CurrencyOrb::Update(float delta)
{
	SolidMovingSprite::Update(delta);

	switch (mCurrentState)
	{
		case kIdle:
		{
			DoIdleHover(delta);
			break;
		}
		case kTracking:
		{
			DoTrackPlayer(delta);
			break;
		}
	}

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
		}
	}
}

void CurrencyOrb::DoIdleHover(float delta)
{

}

void CurrencyOrb::DoTrackPlayer(float delta)
{
	Player * player = GameObjectManager::Instance()->GetPlayer();
	if (!player)
	{
		return;
	}

	// accelerate towards the target
	Vector3 direction = player->Position() - m_position;

	direction.Normalise();

	m_direction = direction;

	if (std::abs(m_direction.X) > std::abs(m_direction.Y))
	{
		AccelerateX(m_direction.X, kAccelerateRate);

		// AccelerateY(m_direction.Y, kAccelerateRate);
	}
	else
	{
		AccelerateY(m_direction.Y, kAccelerateRate);
	}

	if ((m_direction.X < 0 && m_velocity.X > 0) ||
		(m_direction.X > 0 && m_velocity.X < 0))
	{
		// the velocity of the orb is still moving in the opposite x direction
		// let's give it a helping hand to catch up by accelerating harshly
		AccelerateX(m_direction.X, kHarshAccelerateRate);
	}

	if ((m_direction.Y < 0 && m_velocity.Y > 0) ||
		(m_direction.Y > 0 && m_velocity.Y < 0))
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
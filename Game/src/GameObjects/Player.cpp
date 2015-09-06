#include "precompiled.h"
#include "Player.h"
#include "AnimationPart.h"
#include "AudioManager.h"
#include "orb.h"
#include "particleemittermanager.h"
#include "BombProjectile.h"
#include "Game.h"
#include "SaveManager.h"
#include "PlayerLevelManager.h"

static const char * kBombTextureFile = "Media/bomb.png";

Player::Player(float x, float y, float z, float width, float height, float breadth) :
Character(x, y, z, width, height, breadth),
	mProjectileFireDelay(0.2f),
	mTimeUntilProjectileReady(0.0f),
	mFireBurstNum(5),
	mCurrentBurstNum(0),
	mFireBurstDelay(0.5f),
	mTimeUntilFireBurstAvailable(0.0f)
{
	mHealth = 100.0f;
	mAlwaysUpdate = true;
}

Player::~Player(void)
{
}

void Player::Initialise()
{
	// update the base classes
	Character::Initialise();

	mRunAnimFramerateMultiplier = 1.0f;
}
void Player::XmlRead(TiXmlElement * element)
{
	// update the base classes
	Character::XmlRead(element);
}

void Player::XmlWrite(TiXmlElement * element)
{
	Character::XmlWrite(element);
}

void Player::OnDamage(GameObject * damageDealer, float damageAmount, Vector3 pointOfContact, bool shouldExplode)
{
	Character::OnDamage(damageDealer, damageAmount, pointOfContact, shouldExplode);

	if (mHealth <= 0.0f)
	{
		m_alpha = 0.2f; 
	}
}

Projectile * Player::FireBomb(Vector2 direction)
{
	Vector3 pos = m_position;
	pos.X = (direction.X > 0) ? pos.X + m_projectileOffset.X : pos.X -= m_projectileOffset.X;
	pos.Y += m_projectileOffset.Y;
	pos.Z -= 0.1;

	if (direction.X > 0)
	{
		pos.X += m_projectileOffset.X;
	}
	else
	{
		pos.X -= m_projectileOffset.X;
	}

	float speed = mSprintActive ? 20 : 15;

	Projectile * p = new BombProjectile(Projectile::kPlayerProjectile,
										kBombTextureFile,
										mProjectileImpactFilePath.c_str(),
										pos,
										Vector2(30,8),
										Vector2(60,60),
										direction,
										50,
										speed,
										2.0f);

	if (m_isAnimated && m_animation)
	{
		AnimationPart * armPart = m_animation->GetPart("arm");

		if (armPart)
		{
			armPart->Restart();
			armPart->Animate(); // get's updated in UpdateAnimations()
		}
	}

	PlayRandomWeaponFireSound();

	return p;
}

void Player::Update(float delta)
{
	// update base classes
	Character::Update(delta);

	/*if (mSprintActive)
	{
		Camera2D::GetInstance()->DoSmallShake();
	}*/

	if (mCurrentBurstNum >= mFireBurstNum)
	{
		mCurrentBurstNum = 0;
		mTimeUntilFireBurstAvailable = mFireBurstDelay;
	}
	else if (mTimeUntilFireBurstAvailable > 0.0f)
	{
		mTimeUntilFireBurstAvailable -= delta;

		if (mTimeUntilFireBurstAvailable <= 0.0f)
		{
			mTimeUntilFireBurstAvailable = 0.0f;
		}
	}
	else if (mTimeUntilProjectileReady > 0.0f)
	{
		float projectileReloadDelta = delta;
		if (Timing::Instance()->GetTimeModifier() < 1.0f)
		{
			projectileReloadDelta *= 5.0f;
		}

		mTimeUntilProjectileReady -= projectileReloadDelta;

		if (mTimeUntilProjectileReady <= 0.0f)
		{
			mTimeUntilProjectileReady = 0.0f;
		}
	}

	CheckForAndDoLevelUp();
}

Projectile * Player::FireWeapon(Vector2 direction)
{
	if ( mCurrentBurstNum >= mFireBurstNum || mTimeUntilProjectileReady > 0.0f)
	{
		return nullptr;
	}

	++mCurrentBurstNum;
	mTimeUntilProjectileReady = mProjectileFireDelay;

	Vector3 pos = m_position;
	pos.X = (direction.X > 0) ? pos.X + m_projectileOffset.X : pos.X -= m_projectileOffset.X;
	pos.Y += m_projectileOffset.Y;
	pos.Z -= 0.1;

	if (direction.X > 0)
	{
		pos.X += m_projectileOffset.X;
	}
	else
	{
		pos.X -= m_projectileOffset.X;
	}

	float speed = mSprintActive ? 23 : 15;
	bool isInDeepWater = WasInWaterLastFrame() && GetWaterIsDeep();

	Projectile * p = new Projectile(Projectile::kPlayerProjectile,
									mProjectileFilePath.c_str(),
									mProjectileImpactFilePath.c_str(),
									pos,
									Vector2(30,8),
									Vector2(60,20),
									direction,
									5,
									isInDeepWater ? speed * 0.6f : speed,
									0.25f);

	if (m_isAnimated && m_animation)
	{
		AnimationPart * armPart = m_animation->GetPart("arm");

		if (armPart)
		{
			armPart->Restart();
			armPart->Animate(); // get's updated in UpdateAnimations()
		}
	}

	if (direction.X > 0)
	{
		p->UnFlipVertical();
	}
	else
	{
		//p->FlipVertical();
		p->FlipHorizontal();
	}

	if (!WasInWaterLastFrame())
	{
		PlayRandomWeaponFireSound();
	}

	return p;
}

void Player::DebugDraw(ID3D10Device *  device)
{
	if (!Game::GetInstance()->GetIsLevelEditMode())
	{
		mShowDebugText = true;
	}

	Character::DebugDraw(device);
}

void Player::ResetProjectileFireDelay()
{
	mTimeUntilProjectileReady = 0.0f;
	mTimeUntilFireBurstAvailable = 0.0f;
	mCurrentBurstNum = 0;
}

void Player::CheckForAndDoLevelUp()
{
	unsigned int playerLevel = SaveManager::GetInstance()->GetPlayerLevel();
	unsigned int orbsCollected = SaveManager::GetInstance()->GetNumCurrencyOrbsCollected();

	if (PlayerLevelManager::GetInstance()->ShouldLevelUp(playerLevel, orbsCollected))
	{
		SaveManager::GetInstance()->SetPlayerLevel(playerLevel + 1);

		// Do effects
		Timing::Instance()->SetTimeModifierForNumSeconds(0.1f, 3.5f);
		ParticleEmitterManager::Instance()->CreateDirectedSpray(1,
																m_position,
																Vector3(0, 1, 0),
																0.1,
																Vector3(3200, 1200, 0),
																"Media\\blast_circle.png",
																1.0f,
																1.0f,
																0.3f,
																0.3f,
																256.0f,
																256.0f,
																0.0f,
																false,
																1.0f,
																1.0f,
																0.0f,
																true,
																8.0f,
																0.0f,
																0.0f,
																0.05f,
																0.1f,
																true);
	}
}
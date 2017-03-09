#include "precompiled.h"
#include "Player.h"
#include "AnimationPart.h"
#include "AudioManager.h"
#include "Debris.h"
#include "particleemittermanager.h"
#include "BombProjectile.h"
#include "Game.h"
#include "SaveManager.h"
#include "PlayerLevelManager.h"
#include "TextObject.h"
#include "AudioManager.h"
#include "DrawUtilities.h"

static const char * kBombTextureFile = "Media/bomb.png";
static const float kAimLineOpacityDecrementDelay = 1.0f;
static const float kAimLineOpacityDecreaseRate = 1.8f;

Player::Player(float x, float y, float z, float width, float height, float breadth) :
Character(x, y, z, width, height, breadth),
	mProjectileFireDelay(0.1f),
	mTimeUntilProjectileReady(0.0f),
	mFireBurstNum(5),
	mCurrentBurstNum(0),
	mFireBurstDelay(0.5f),
	mTimeUntilFireBurstAvailable(0.0f),
	mBurstFireEnabled(false),
	mAimLineSprite(nullptr),
	mTimeUntilAimLineStartDisappearing(0.0f)
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

	mRunAnimFramerateMultiplier = 3.0f;

	m_maxJumpSpeed = 20;

	mMaxJumpsAllowed = 2;
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

	float speed = mSprintActive ? 10 : 7.5;

	Projectile * p = new BombProjectile(Projectile::kPlayerProjectile,
										kBombTextureFile,
										mProjectileImpactFilePath.c_str(),
										pos,
										Vector2(30,8),
										Vector2(30,30),
										direction,
										500,
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

	if (mBurstFireEnabled && mCurrentBurstNum >= mFireBurstNum)
	{
		mCurrentBurstNum = 0;
		mTimeUntilFireBurstAvailable = mFireBurstDelay;
	}
	else if (mBurstFireEnabled && mTimeUntilFireBurstAvailable > 0.0f)
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

	if (mAimLineSprite)
	{
		mAimLineSprite->SetY(m_position.Y + m_projectileOffset.Y);

		if (mIsStrafing)
		{
			// always show aim line when strafing
			mTimeUntilAimLineStartDisappearing = kAimLineOpacityDecrementDelay;
		}

		if (mTimeUntilAimLineStartDisappearing > 0.0f)
		{
			mTimeUntilAimLineStartDisappearing -= delta;
			mAimLineSprite->SetAlpha(1.0f);
		}
		else
		{
			if (mAimLineSprite->Alpha() > 0.0f)
			{
				mAimLineSprite->SetAlpha(mAimLineSprite->Alpha() - (kAimLineOpacityDecreaseRate * delta));
			}
		}
	}
}

Projectile * Player::FireWeapon(Vector2 direction)
{
	if (GetIsCollidingAtObjectSide() ||
		GetIsRolling())
	{
		return nullptr;
	}

	mTimeUntilAimLineStartDisappearing = kAimLineOpacityDecrementDelay;
	if ((mBurstFireEnabled && mCurrentBurstNum >= mFireBurstNum) || 
		mTimeUntilProjectileReady > 0.0f)
	{
		return nullptr;
	}

	if (mBurstFireEnabled)
	{
		++mCurrentBurstNum;
	}

	mTimeUntilProjectileReady = mProjectileFireDelay;

	Vector3 pos = m_position;
	pos.X = (direction.X > 0) ? pos.X + m_projectileOffset.X : pos.X -= m_projectileOffset.X;
	pos.Y += mIsCrouching ? m_projectileOffset.Y - 25 : m_projectileOffset.Y;
	pos.Z = m_position.Z + 0.1;

	if (mIsMidAirMovingUp)
	{
		pos.Y += 65;
	}
	else if (mIsMidAirMovingDown)
	{
		pos.Y += 40;
	}

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

	if (GetCurrentSolidLineStrip() != nullptr)
	{
		DrawUtilities::DrawLine(Vector2(m_position.X, m_position.Y), 
								Vector2(m_position.X + 100 * mCurrentSolidLineDirection.X, m_position.Y + 100 * mCurrentSolidLineDirection.Y));
	}
	else
	{
		DrawUtilities::DrawLine(Vector2(m_position.X, m_position.Y), Vector2(m_position.X + 100 * m_direction.X, m_position.Y));
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
		AudioManager::Instance()->PlaySoundEffect("gong.wav", false, false, false);
		AudioManager::Instance()->PlaySoundEffect("music\\japanese1.wav", false, false, false);
		Camera2D::GetInstance()->DoBigShake();
		Timing::Instance()->SetTimeModifierForNumSeconds(0.04f, 4.0f);
		ParticleEmitterManager::Instance()->CreateDirectedSpray(1,
																m_position,
																Vector3(0, 1, 0),
																0.1,
																Vector3(3200, 1200, 0),
																"Media\\blast_circle.png",
																1.0f,
																1.0f,
																0.1f,
																0.1f,
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

		ParticleEmitterManager::Instance()->CreateDirectedSpray(1,
																m_position,
																Vector3(0, 1, 0),
																0.1,
																Vector3(3200, 1200, 0),
																"Media\\explosion_lines.png",
																1.0f,
																1.0f,
																0.1f,
																0.1f,
																256.0f,
																256.0f,
																0.0f,
																false,
																1.0f,
																1.0f,
																0.0f,
																true,
																12.0f,
																0.0f,
																0.0f,
																0.05f,
																0.1f,
																true);

		ParticleEmitterManager::Instance()->CreateDirectedSpray(1,
																m_position,
																Vector3(0, 1, 0),
																0.1,
																Vector3(3200, 1200, 0),
																"Media\\explosion_lines.png",
																1.0f,
																1.0f,
																0.15f,
																0.15f,
																256.0f,
																256.0f,
																0.0f,
																false,
																1.0f,
																1.0f,
																0.0f,
																true,
																6.0f,
																0.0f,
																0.0f,
																0.05f,
																0.1f,
																true);

		// *** Level Up! text
		{
			TextObject * levelUpText = new TextObject(m_position.X, m_position.Y, 3.0f);

			levelUpText->SetFont("Jing Jing");
			levelUpText->SetFontColor(1.0f, 0.0f, 0.0f);
			levelUpText->SetFontSize(50.0f);
			levelUpText->SetStringKey("level_up_notification");
			levelUpText->SetNoClip(true);

			levelUpText->SetDimensionsXYZ(200, 250, 1);

			GameObjectManager::Instance()->AddGameObject(levelUpText);

			levelUpText->AttachTo(GameObjectManager::Instance()->GetObjectByID(ID()), Vector3(0, 125, -30), false);
		}
		// ************************

		// *** Game Mechanic unlocked text
		{
			TextObject * mechanicDescriptionText = new TextObject(m_position.X, m_position.Y, 3.0f);

			mechanicDescriptionText->SetFont("Jing Jing");
			mechanicDescriptionText->SetFontColor(0.1f, 0.1f, 0.1f);
			mechanicDescriptionText->SetFontSize(35.0f);
			mechanicDescriptionText->SetStringKey("test_string");
			mechanicDescriptionText->SetNoClip(true);

			mechanicDescriptionText->SetDimensionsXYZ(300, 100, 1);

			GameObjectManager::Instance()->AddGameObject(mechanicDescriptionText);

			mechanicDescriptionText->AttachTo(GameObjectManager::Instance()->GetObjectByID(ID()), Vector3(0, 80, -30), false);
		}
		// ************************************
	}
}

void Player::AddAimLineSprite()
{
	GAME_ASSERT(!mAimLineSprite);

	if (mAimLineSprite)
	{
		return;
	}

	mAimLineSprite = new Sprite();
	
	mAimLineSprite->SetDimensionsXYZ(100, 100, 0);

	mAimLineSprite->SetTextureFilename("Media\\aim_line.png");

	mAimLineSprite->EffectName = this->EffectName;

	mAimLineSprite->SetZ(Z() + 0.1f);

	mAimLineSprite->AttachTo(GameObjectManager::Instance()->GetObjectByID(ID()), Vector3(m_projectileOffset.X , m_projectileOffset.Y , 0), false);

	GameObjectManager::Instance()->AddGameObject(mAimLineSprite);

	mAimLineSprite->SetAlpha(0.0f);
}

void Player::SetAimLineDirection(Vector2 & dir)
{
	if (!mAimLineSprite)
	{
		return;
	}

	if (dir.Y < 0)
	{
		if (dir.X > 0)
		{
			mAimLineSprite->SetRotationAngle(-acos(dir.Dot(Vector2(1, 0))));
			mAimLineSprite->UnFlipVertical();
			mAimLineSprite->SetAttachmentOffsetX(m_projectileOffset.X);
		}
		else
		{
			mAimLineSprite->SetRotationAngle(acos(dir.Dot(Vector2(1, 0))));
			mAimLineSprite->FlipVertical();
			mAimLineSprite->SetAttachmentOffsetX(-m_projectileOffset.X);
		}
	}
	else
	{
		if (dir.X > 0)
		{
			mAimLineSprite->SetRotationAngle(acos(dir.Dot(Vector2(1, 0))));
			mAimLineSprite->UnFlipVertical();
			mAimLineSprite->SetAttachmentOffsetX(m_projectileOffset.X);
		}
		else
		{
			mAimLineSprite->SetRotationAngle(-acos(dir.Dot(Vector2(1, 0))));
			mAimLineSprite->FlipVertical();
			mAimLineSprite->SetAttachmentOffsetX(-m_projectileOffset.X);
		}
	}
}

void Player::SetCurrentSolidLineDirection(const Vector2 & direction)
{
	mCurrentSolidLineDirection = direction;
}



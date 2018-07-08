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
#include "FeatureUnlockManager.h"

static const char * kBombTextureFile = "Media/bomb.png";
static const float kAimLineOpacityDecrementDelay = 0.05f;
static const float kAimLineOpacityDecreaseRate = 10.0f;

static const float kFocusUseRate = 150.0f;
static const float kFocusRechargeRate = 10.0f;
static const float kFocusCooldownTime = 5.0f;
static const float kMeleeFocusAmount = 75.0f;
static const float kDownwardDashFocusAmount = 75.0f;
static const float kRollFocusAmount = 20.0f;

Player::Player(float x, float y, float width, float height) :
Character(x, y, GameObject::kPlayer, width, height),
	mProjectileFireDelay(0.15f),
	mTimeUntilProjectileReady(0.0f),
	mFireBurstNum(0),
	mCurrentBurstNum(0),
	mFireBurstDelay(0.45f),
	mTimeUntilFireBurstAvailable(0.0f),
	mBurstFireEnabled(true),
	mAimLineSprite(nullptr),
	mTimeUntilAimLineStartDisappearing(0.0f)
{
	mHealth = 100.0f;
	mAlwaysUpdate = true;
	mIsPlayer = true;
}

Player::~Player(void)
{
}

void Player::Initialise()
{
	// update the base classes
	Character::Initialise();

	// Just hardcoding these variables now
	// because I want to get this shit locked down and finished
	mRunAnimFramerateMultiplier = 3.0f;
	m_maxJumpSpeed = 23;
	mMaxJumpsAllowed = 1;
	m_maxVelocity.X = 1.0000f;
	mSprintVelocityX = 17.5f;

	UpdateResistance();

	// TODO: just testing (or am I really, just keep crouch jump all the time?)
	FeatureUnlockManager::GetInstance()->SetFeatureUnlocked(FeatureUnlockManager::kCrouchJump, true);

	m_projectileOffset.X = 5.0f;
	m_projectileOffset.Y = 25.0f;
}

void Player::UpdateResistance()
{
	if (mSprintActive)
	{
		m_resistance.X = 0.94f; // HAS to be lower than 1.0 (TODO: really need to change this)
	}
	else
	{
		m_resistance.X = 0.895f; // HAS to be lower than 1.0 (TODO: really need to change this)
	}

	m_resistance.Y = 0.88f;
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

void Player::OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode)
{
	Character::OnDamage(damageDealer, damageAmount, pointOfContact, shouldExplode);

	if (mHealth <= 0.0f)
	{
		m_alpha = 0.2f; 
	}
}

Projectile * Player::FireBomb(Vector2 direction)
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

	float speed = mSprintActive ? 15 : 11;

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

	UpdateResistance();

	if (mBurstFireEnabled && mFireBurstNum > 0 && mCurrentBurstNum >= mFireBurstNum)
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

	UpdateFocus(delta);
}

Projectile * Player::FireWeapon(Vector2 direction)
{
	if (GetIsCollidingAtObjectSide() ||
		GetIsRolling())
	{
		return nullptr;
	}

	mTimeUntilAimLineStartDisappearing = kAimLineOpacityDecrementDelay;
	if ((mBurstFireEnabled && (mCurrentBurstNum >= mFireBurstNum && mFireBurstNum > 0)) ||
		mTimeUntilProjectileReady > 0.0f)
	{
		return nullptr;
	}

	if (mBurstFireEnabled)
	{
		++mCurrentBurstNum;
	}

	mTimeUntilProjectileReady = mProjectileFireDelay;

	Vector2 pos = m_position;
	pos.X = (direction.X > 0.0f) ? pos.X + m_projectileOffset.X : pos.X -= m_projectileOffset.X;
	pos.Y += mIsCrouching ? m_projectileOffset.Y - 25.0f : m_projectileOffset.Y;

	if (mIsMidAirMovingUp)
	{
		pos.Y += 65.0f;
	}
	else if (mIsMidAirMovingDown)
	{
		pos.Y += 40.0f;
	}

	if (direction.X > 0.0f)
	{
		pos.X += m_projectileOffset.X;
	}
	else
	{
		pos.X -= m_projectileOffset.X;
	}

	float speed = mSprintActive ? 35.0f : 30.0f;
	bool isInWater = WasInWaterLastFrame();

	Projectile * p = new Projectile(Projectile::kPlayerProjectile,
									mProjectileFilePath.c_str(),
									mProjectileImpactFilePath.c_str(),
									pos,
									GameObject::kPlayerProjectile,
									Vector2(91.0f,16.0f),
									Vector2(91.0f,16.0f),
									direction,
									0.5f,
									isInWater ? speed * 0.6f : speed,
									0.25f);
	p->SetIsNativeDimensions(false);

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
	if (Game::GetInstance()->IsLevelEditTerrainMode())
	{
		// If we are terrain editing then just highlight the terrain (SolidLineStrip)
		return;
	}

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
																GetDepthLayer(),
																Vector2(0.0f, 0.0f),
																0.1f,
																Vector2(3200.0f, 1200.0f),
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
																GetDepthLayer(),
																Vector2(0.0f, 1.0f),
																0.1f,
																Vector2(3200.0f, 1200.0f),
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
			TextObject * levelUpText = new TextObject(m_position.X, m_position.Y, GetDepthLayer(), 3.0f);

			levelUpText->SetFont("Jing Jing");
			levelUpText->SetFontColor(1.0f, 0.0f, 0.0f);
			levelUpText->SetFontSize(50.0f);
			levelUpText->SetStringKey("level_up_notification");
			levelUpText->SetNoClip(true);

			levelUpText->SetDimensionsXY(200.0f, 250.0f);

			GameObjectManager::Instance()->AddGameObject(levelUpText);

			levelUpText->AttachTo(GameObjectManager::Instance()->GetObjectByID(ID()), Vector2(0.0f, 125.0f), GameObject::kPlayer, false);
		}
		// ************************

		// *** Game Mechanic unlocked text
		{
			TextObject * mechanicDescriptionText = new TextObject(m_position.X, m_position.Y, GetDepthLayer(), 3.0f);

			mechanicDescriptionText->SetFont("Jing Jing");
			mechanicDescriptionText->SetFontColor(0.1f, 0.1f, 0.1f);
			mechanicDescriptionText->SetFontSize(35.0f);
			mechanicDescriptionText->SetStringKey("test_string");
			mechanicDescriptionText->SetNoClip(true);

			mechanicDescriptionText->SetDimensionsXY(300.0f, 100.0f);

			GameObjectManager::Instance()->AddGameObject(mechanicDescriptionText);

			mechanicDescriptionText->AttachTo(GameObjectManager::Instance()->GetObjectByID(ID()), Vector2(0.0f, 80.0f),GameObject::kPlayer,  false);
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
	
	mAimLineSprite->SetIsNativeDimensions(false);
	mAimLineSprite->SetDimensionsXY(400.0f, 400.0f);

	mAimLineSprite->SetTextureFilename("Media\\aim_line.png");

	mAimLineSprite->EffectName = this->EffectName;

	mAimLineSprite->SetDepthLayer(GameObject::kPlayer);

	mAimLineSprite->AttachTo(GameObjectManager::Instance()->GetObjectByID(ID()), Vector2(m_projectileOffset.X , m_projectileOffset.Y), GameObject::kPlayer, false);

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

bool Player::CanJump() const
{
	if (!Character::CanJump())
	{
		return false;
	}

	if (GetIsInWater())
	{
		return true;
	}

	if (mCurrentJumpsBeforeLand > 1 &&
		!FeatureUnlockManager::GetInstance()->IsFeatureUnlocked(FeatureUnlockManager::kDoubleJump))
	{
		return false;
	}

	return true;
}

void Player::TryFocus()
{
	if (mIsFocusing)
	{
		return;
	}

	if (mCurrentFocusCooldown > 0.0f)
	{
		return;
	}

	if (mCurrentFocusAmount <= 0.0f)
	{
		return;
	}

	Timing::Instance()->SetTimeModifier(0.15f);
	mIsFocusing = true;
}

void Player::StopFocus()
{
	Timing::Instance()->SetTimeModifier(1.0f);
	mIsFocusing = false;
}

void Player::UpdateFocus(float delta)
{
	if (mIsFocusing)
	{
		ConsumeFocus(kFocusUseRate * delta);
	}
	else
	{
		if (mCurrentFocusCooldown > 0.0f)
		{
			mCurrentFocusCooldown -= delta;
		}
		else if (mCurrentFocusAmount < mMaxFocusAmount)
		{
			mCurrentFocusAmount += kFocusRechargeRate * delta;
		}
	}
}

bool Player::HasEnoughFocus(float amountToUse)
{
	if (mCurrentFocusAmount >= amountToUse)
	{
		return true;
	}

	return false;
}

bool Player::DoMeleeAttack()
{
	if (!HasEnoughFocus(kMeleeFocusAmount))
	{
		return false;
	}

	if (Character::DoMeleeAttack())
	{
		ConsumeFocus(kMeleeFocusAmount);
		return true;
	}

	return false;
}

void Player::ConsumeFocus(float focusAmount)
{
	mCurrentFocusAmount -= focusAmount;

	if (mCurrentFocusAmount <= 0.0f)
	{
		mCurrentFocusAmount = 0.0f;
		StopFocus();
		// start cooldown
		mCurrentFocusCooldown = kFocusCooldownTime;
	}
}

bool Player::DoDownwardDash()
{
	if (!HasEnoughFocus(kDownwardDashFocusAmount))
	{
		return false;
	}

	if (Character::DoDownwardDash())
	{
		ConsumeFocus(kDownwardDashFocusAmount);
		return true;
	}

	return false;
}

bool Player::Roll() 
{
	if (!HasEnoughFocus(kRollFocusAmount))
	{
		return false;
	}

	if (Character::Roll())
	{
		ConsumeFocus(kRollFocusAmount);
		return true;
	}

	return false;
}


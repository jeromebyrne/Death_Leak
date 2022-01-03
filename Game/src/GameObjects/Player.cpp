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
#include "UIManager.h"
#include "Timing.h"

static const char * kBombTextureFile = "Media/bomb.png";
static const float kAimLineOpacityDecrementDelay = 0.05f;
static const float kAimLineOpacityDecreaseRate = 10.0f;
static const float kSprintZoomPercent = 0.75f;
static const float kSprintZoomCamChangeRateIn = 0.3f;
static const float kSprintZoomCamChangeRateOut = 0.10f;

static const float kStomachPullTimeRate = 0.05f;

static const float kFocusUseRate = 80.0f;
static const float kSprintFocusUseRate = 30.0f;
static const float kFocusRechargeRate = 10.0f;
static const float kFocusCooldownTime = 5.0f;
static const float kMeleeFocusAmount = 4.0f;
static const float kDownwardDashFocusAmount = 50.0f;
static const float kRollFocusAmount = 2.0f;
static const float kWaterFocusUseRate = 0.5f;
static const float kDrownHealthLossRate = 2.0f;
static const float kStomachSwordPullTime = 3.5f;
static const float kResistanceY = 0.55f;
static const float kInitialSwordPullBreathingVolume = 0.35f;
static const float kSwordPullIdleShowPromptDelay = 8.0f;
static const float kProjectileDamage = 1.25f;
static const float kTotalTimeInFinalLevel = 60.0f;

Player::Player(float x, float y, float width, float height) :
Character(x, y, GameObject::kPlayer, width, height),
	mProjectileFireDelay(0.225f),
	mTimeUntilProjectileReady(0.0f),
	mFireBurstNum(4),
	mCurrentBurstNum(0),
	mFireBurstDelay(0.50f),
	mTimeUntilFireBurstAvailable(0.0f),
	mBurstFireEnabled(true),
	mAimLineSprite(nullptr),
	mTimeUntilAimLineStartDisappearing(0.0f)
{
	mAlwaysUpdate = true;
	mIsPlayer = true;
	mDoMeleeSpriteResize = true;
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
	mRunAnimFramerateMultiplier = 2.65f; 
	m_maxJumpSpeed = 21.0f;
	mMaxJumpsAllowed = 1;
	mSprintVelocityX = 17.5f;
	mAccelXRate = 1.00f;
	m_maxVelocity.X = 11.0f;
	mDefaultVelocityX = m_maxVelocity.X;

	UpdateResistance();

	m_projectileOffset.X = 5.0f;
	m_projectileOffset.Y = 25.0f;

	SetGravityApplyAmount(0.75f);

	int maxHealth = SaveManager::GetInstance()->GetPlayerMaxHealth();
	if (maxHealth == 0)
	{
		maxHealth = 50;
	}

	mMaxHealth = maxHealth;

	if (mHealth > mMaxHealth)
	{
		mHealth = mMaxHealth;
	}

	// mDeltaTimeMultiplierInSloMo = 3.5f;

	mCameraZoomOnLoad = Camera2D::GetInstance()->GetZoomLevel();

	string currentLevel = GameObjectManager::Instance()->GetCurrentLevelFile();
	mIsInFinalLevel = (currentLevel == "XmlFiles\\levels\\sea_2.xml");
}

void Player::UpdateResistance()
{
	if (GetIsRolling())
	{
		m_resistance.X = 0.93f;
	}
	else if (mSprintActive)
	{
		m_resistance.X = 0.94f; // HAS to be lower than 1.0 (TODO: really need to change this)
	}
	else
	{
		// m_resistance.X = 0.895f; // HAS to be lower than 1.0 (TODO: really need to change this)
		if (m_acceleration.X == 0.0f)
		{
			m_resistance.X = 0.80f;
		}
		else
		{
			m_resistance.X = 0.9999f;
		}
	}

	m_resistance.Y = kResistanceY;
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

	if (damageAmount > 0.5f)
	{
		Game::GetInstance()->Vibrate(0.0f, 0.4f, 0.15f);
	}

	if (IsDead())
	{
		m_alpha = 0.0f;

		Game::GetInstance()->Vibrate(1.0f, 1.0f, 0.3f);

		TriggerDiedUI();
	}
}

void Player::TriggerDiedUI()
{
	if (mHasTriggeredDiedUI == false)
	{
		mHasTriggeredDiedUI = true;
		UIManager::Instance()->PushUI("you_died");
	}
}

Projectile * Player::FireBomb(Vector2 direction)
{
	if (WasInWaterLastFrame())
	{
		return nullptr;
	}

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
										Vector2(45,45),
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

void Player::EndStory()
{
	SaveManager::GetInstance()->WipeSaveFile();
	UIManager::Instance()->PopUI("final_scene_hud");
	UIManager::Instance()->EndStory();
}

void Player::Update(float delta)
{
	// update base classes
	Character::Update(delta);

	if (!SaveManager::GetInstance()->HasPulledSwordFromStomach())
	{
		UpdateIsPullingSwordFromStomach(delta);
		return;
	}

	if (mIsInFinalLevel)
	{
		SetCrouching(true);
		SetVelocityX(0.0f);
		SetMaxVelocityX(0.0f);

		mCurrentTimeInFinalLevel += delta;

		if (mCurrentTimeInFinalLevel > kTotalTimeInFinalLevel)
		{
			EndStory();	
		}
		return;
	}

	UpdateResistance();

	float timeMod = Timing::Instance()->GetTimeModifier();

	if ((mBurstFireEnabled && timeMod == 1.0f) && mFireBurstNum > 0 && mCurrentBurstNum >= mFireBurstNum)
	{
		mCurrentBurstNum = 0;
		mTimeUntilFireBurstAvailable = mFireBurstDelay;
	}
	else if ((mBurstFireEnabled && timeMod == 1.0f) && mTimeUntilFireBurstAvailable > 0.0f)
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
		if (timeMod < 1.0f)
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

	if (GetIsSprintActive() && CanSprint())
	{
		ConsumeFocus(kSprintFocusUseRate * delta);

		mIsDoingSprintZoom = true;
	}
	else
	{
		SetSprintActive(false);
	}

	if (IsDead())
	{
		Timing::Instance()->SetTimeModifier(0.15f);
	}
}

Projectile * Player::FireWeapon(Vector2 direction, float speedMultiplier)
{
	if (GetIsCollidingAtObjectSide() ||
		GetIsRolling() ||
		mWasInWaterLastFrame || 
		mIsInWater)
	{
		return nullptr;
	}

	float timeMod = Timing::Instance()->GetTimeModifier();

	mTimeUntilAimLineStartDisappearing = kAimLineOpacityDecrementDelay;
	if (((mBurstFireEnabled && timeMod == 1.0f) && (mCurrentBurstNum >= mFireBurstNum && mFireBurstNum > 0)) ||
		mTimeUntilProjectileReady > 0.0f)
	{
		return nullptr;
	}

	if ((mBurstFireEnabled && timeMod == 1.0f))
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

	float speed = (mSprintActive ? 25.0f : 20.0f) * speedMultiplier;
	bool isInWater = WasInWaterLastFrame();

	Projectile * p = new Projectile(Projectile::kPlayerProjectile,
									mProjectileFilePath.c_str(),
									mProjectileImpactFilePath.c_str(),
									pos,
									GameObject::kPlayerProjectile,
									Vector2(84.50,25.0f),
									Vector2(84.50,25.0f),
									direction,
									kProjectileDamage,
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
	if (!FeatureUnlockManager::GetInstance()->IsFeatureUnlocked(FeatureUnlockManager::kSlowMotion))
	{
		return;
	}

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

	if (WasInWaterLastFrame())
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
	else if (!WasInWaterLastFrame())
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
	else
	{
		ConsumeFocus(kWaterFocusUseRate * delta);

		if (mCurrentFocusAmount <= 0.0f)
		{
			mHealth -= kDrownHealthLossRate * delta;

			if (mHealth < 25.0f && mHealth > 0.0f)
			{
				Camera2D::GetInstance()->DoSmallShake();
				Game::GetInstance()->Vibrate(0.25, 0.25f, 0.1f);
			}
			else if (mHealth <= 0.0f)
			{
				TriggerDiedUI();
			}
		}
	}
}

bool Player::HasEnoughFocus(float amountToUse)
{
	if (mCurrentFocusAmount > 0)
	{
		// fuck it, just let the player do anything if there's any focus
		return true;
	}

	if (mCurrentFocusAmount >= amountToUse)
	{
		return true;
	}

	return false;
}

bool Player::DoMeleeAttack()
{
	if (WasInWaterLastFrame())
	{
		return false;
	}

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
	if (!FeatureUnlockManager::GetInstance()->IsFeatureUnlocked(FeatureUnlockManager::kRoll))
	{
		return false;
	}

	if (!HasEnoughFocus(kRollFocusAmount))
	{
		return false;
	}

	if (Character::Roll())
	{
		ConsumeFocus(kRollFocusAmount);
		Game::GetInstance()->Vibrate(0.0f, 0.04f, 0.15f);
		return true;
	}

	return false;
}

void Player::SetMaxHealth(float value)
{
	mMaxHealth = value;

	SaveManager::GetInstance()->SetPlayerMaxHealth((int)mMaxHealth);
}

void Player::SetHealth(float value)
{
	mHealth = value;
}

bool Player::CanSprint()
{
	if (GetIsRolling() ||
		IsDoingMelee() ||
		!IsOnSolidSurface())
	{
		return false;
	}

	return HasEnoughFocus(0.1f);
}

void Player::TrySprint()
{
	if (WasInWaterLastFrame())
	{
		return;
	}

	if (!CanSprint())
	{
		return;
	}

	SetSprintActive(true);
}

void Player::StopSprint()
{
	SetSprintActive(false);
}

void Player::UpdateAnimations()
{
	bool hasStarted = SaveManager::GetInstance()->HasPulledSwordFromStomach();

	if (!hasStarted)
	{
		AnimationPart * bodyPart = m_animation->GetPart("body");
		GAME_ASSERT(bodyPart);

		if (bodyPart == nullptr)
		{
			return;
		}

		string current_body_sequence_name = bodyPart->CurrentSequence()->Name();

		if (current_body_sequence_name != "IntroCutscene1" && current_body_sequence_name != "IntroCutscene2")
		{
			bodyPart->SetSequence("IntroCutscene1");
		}

		if (bodyPart->CurrentSequence()->Name() == "IntroCutscene1")
		{
			bodyPart->AnimateLooped();
		}
		else
		{
			bodyPart->Animate();
		}
		
		m_texture = bodyPart->CurrentFrame();

		m_mainBodyTexture = m_texture;

		// arm anim
		{
			AnimationPart * armPart = m_animation->GetPart("arm");

			if (armPart != nullptr)
			{
				if (!armPart->IsFinished())
				{
					armPart->Animate();
				}
				if (armPart->CurrentSequence()->Name() != current_body_sequence_name)
				{
					// arm part must have the same sequences as the body for this to work
					armPart->SetSequence(current_body_sequence_name);
					armPart->Finish();
				}
			}
		}
	}
	else
	{
		Character::UpdateAnimations();
	}
}

void Player::UpdateIsPullingSwordFromStomach(float delta)
{
	AnimationPart * bodyPart = m_animation->GetPart("body");
	GAME_ASSERT(bodyPart);

	if (bodyPart == nullptr)
	{
		return;
	}

	string current_body_sequence_name = bodyPart->CurrentSequence()->Name();

	if (current_body_sequence_name == "IntroCutscene1")
	{
		mTotalTimePullingSword += delta;

		if (mBreathingIntroSFX == nullptr)
		{
			mBreathingIntroSFX = AudioManager::Instance()->PlaySoundEffect("character\\breathing_intro.wav", true, true, true);
			mBreathingIntroSFX->setVolume(kInitialSwordPullBreathingVolume);
		}

		// Phase 1
		const InputManager & i = Game::GetInstance()->GetInputManager();
		if (i.IsPressingInteractButton())
		{
			mCurrentTimePullingSword += delta;

			if (mBreathingIntroSFX != nullptr)
			{
				float percent = mCurrentTimePullingSword / kStomachSwordPullTime;

				mBreathingIntroSFX->setVolume(kInitialSwordPullBreathingVolume + ((1.0f - kInitialSwordPullBreathingVolume) * percent));
			}

			if (mCurrentTimePullingSword > kStomachSwordPullTime)
			{
				mCurrentTimePullingSword = 0.0f;
				bodyPart->SetSequence("IntroCutscene2");

				auto cam = Camera2D::GetInstance();
				cam->SetZoomLevel(mCameraZoomOnLoad);

				Game::GetInstance()->Vibrate(1.0f, 0.25f, 0.75f);

				ParticleEmitterManager::Instance()->CreateDirectedSpray(20,
					Vector2(m_position.X - 35.0f, m_position.Y - 15.0f),
					GetDepthLayer(),
					Vector2(-0.75f, -0.25f),
					0.2f,
					Vector2(4000, 4000),
					"Media\\bloodparticle2.png",
					1.0f,
					4.0f,
					0.25f,
					0.75f,
					20.0f,
					50.0f,
					1.5f,
					false,
					0.5f,
					1.0f,
					0.0f,
					true,
					3.0f,
					0.1f,
					0.8f,
					5.0f,
					1.0f);

				ParticleEmitterManager::Instance()->CreateDirectedSpray(50,
					Vector2(m_position.X , m_position.Y - 20.0f),
					GetDepthLayer(),
					Vector2(-0.5f, 0.5f),
					0.45f,
					Vector2(4000, 4000),
					"Media\\bloodparticle3.png",
					8.0f,
					18.0f,
					0.45f,
					0.75f,
					50.0f,
					75.0f,
					1.5f,
					false,
					0.5f,
					1.0f,
					0.0f,
					true,
					1.75f,
					0.1f,
					0.8f,
					5.0f,
					1.0f);

				AudioManager::Instance()->PlaySoundEffect("blood\\blood2.wav");
				AudioManager::Instance()->PlaySoundEffect("character\\sword_pull_grunt.wav");

				FireBloodSpatter(Vector2(-1.6f, 1.4f), Vector2(m_position.X, m_position.Y + 5.0f));

				if (mBreathingIntroSFX != nullptr)
				{
					mBreathingIntroSFX->stop();
					mBreathingIntroSFX->drop();
					mBreathingIntroSFX = nullptr;
				}

				return;
			}

			float time = mCurrentTimePullingSword / kStomachSwordPullTime;

			Camera2D::GetInstance()->DoShake(time * 5.0f, 0.1f);

			Game::GetInstance()->Vibrate(0.0f, time * 0.4f, 0.1f);

			// camera zoom
			{
				auto cam = Camera2D::GetInstance();
				float currentZoom = cam->GetZoomLevel();
				cam->SetZoomLevel(currentZoom - (delta * kStomachPullTimeRate));

				currentZoom = cam->GetZoomLevel();
				if (currentZoom < mCameraZoomOnLoad * kSprintZoomPercent)
				{
					currentZoom = mCameraZoomOnLoad * kSprintZoomPercent;
				}
				else if (currentZoom > mCameraZoomOnLoad)
				{
					currentZoom = mCameraZoomOnLoad;
					mIsDoingSprintZoom = false;
				}

				cam->SetZoomLevel(currentZoom);
			}
		}
		else
		{
			mCurrentTimePullingSword = 0.0f;

			auto cam = Camera2D::GetInstance();
			float currentZoom = cam->GetZoomLevel();
			cam->SetZoomLevel(currentZoom + (delta * kStomachPullTimeRate * 2.0f));

			if (mBreathingIntroSFX)
			{
				if (mBreathingIntroSFX->getVolume() > kInitialSwordPullBreathingVolume)
				{
					mBreathingIntroSFX->setVolume(mBreathingIntroSFX->getVolume() - 0.01f);
				}
				else
				{
					mBreathingIntroSFX->setVolume(kInitialSwordPullBreathingVolume);
				}
			}
		}
	}
	else if (current_body_sequence_name == "IntroCutscene2")
	{
		// Timing::Instance()->SetTimeModifier(0.15f);
		mTotalTimePullingSword = 0.0f;
		if (bodyPart->IsFinished())
		{
			SaveManager::GetInstance()->SetHasPulledSwordFromStomach(true);
		}
	}
	else
	{
		// Timing::Instance()->SetTimeModifier(1.0f);
		mTotalTimePullingSword = 0.0f; 
		SaveManager::GetInstance()->SetHasPulledSwordFromStomach(true);
	}
}

bool Player::CanBeControlled()
{
	if (!SaveManager::GetInstance()->HasPulledSwordFromStomach() ||
		mIsInFinalLevel)
	{
		return false;
	}
	return true;
}

void Player::Draw(ID3D10Device* device, Camera2D* camera)
{
	Character::Draw(device, camera);

	if (mTotalTimePullingSword > kSwordPullIdleShowPromptDelay)
	{
		const InputManager& i = Game::GetInstance()->GetInputManager();
		if (i.IsPressingInteractButton() == false)
		{
			DrawUtilities::DrawTexture(Vector3(m_position.X - 35.0f, m_position.Y + 50.0f, GetDepthLayer() + 0.1f),
				Vector2(50.0f, 50.0f),
				"Media\\UI\\gamepad_icons\\x.png");
		}
	}
}


#include "precompiled.h"
#include "Character.h"
#include "AudioManager.h"
#include "Material.h"
#include "particleEmitterManager.h"
#include "Debris.h"
#include "Projectile.h"
#include "platform.h"
#include "BombProjectile.h"
#include "WaterBlock.h"
#include "DrawUtilities.h"
#include "CurrencyOrb.h"
#include "SolidLineStrip.h"
#include "Game.h"
#include "FeatureUnlockManager.h"
#include "ParticleSpray.h"

float Character::mLastTimePlayedDeathSFX = 0.0f;
static const float kMinTimeBetweenDeathSFX = 0.1f;
static const float kJumpDelay = 0.05f;
static const int kDamageKickback = 20;
static const float kTimeAllowedToJumpAfterLeaveSolidGround = 0.3f;
static const float kSmallDropDistance = 300.0f;
static const float kLargeDropDistance = 700.0f;
static const float kWallJumpTime = 0.3f;
static const float kRollVelocityX = 12.0f;
static const float kRollVelocityY = 0.0f;
static const float kLandRollInputWindow = 0.25f;
static const float kLandJumpInputWindow = 0.2f;
static const float kWallJumpXResistance = 0.99f;
static const float kWallJumpVelocityXBoost = 15.0f;
static const float kWaterJumpPercentModifier = 0.4f;
static Vector2 kMeleeSpriteMultiplier = Vector2(1.1585647f, 1.1201479f);
static const float kTeleportTime = 0.8f;

float Character::mTeleportSfxDelay = 0.5f;

Character::Character(float x, float y, DepthLayer depthLayer, float width, float height) :
	SolidMovingSprite(x, y, depthLayer, width, height),
	m_isJumping(false),
	m_maxJumpSpeed(10.0f),
	m_waterWadeSFXTime(1.0f),
	mAccelXRate(0.0f),
	mHealth(100.0f),
	mMaxHealth(100.0f),
	mSprintVelocityX(8.0f),
	mSprintActive(false),
	mHasExploded(false),
	m_mainBodyTexture(nullptr),
	m_projectileOffset(0.0f, 0.0f),
	mLastTimePlayedDamageSound(0.0f),
	mDamageSoundDelayMilli(0.15f),
	mRunAnimFramerateMultiplier(3.0f),
	mMatchAnimFrameRateWithMovement(true),
	m_lastTimePlayedWaterWadeSFX(0.0f),
	mIsMidAirMovingDown(false),
	mIsMidAirMovingUp(false),
	mMidAirMovingUpStartTime(0.0f),
	mMidAirMovingDownStartTime(0.0f),
	mExplodesGruesomely(false),
	mMaxJumpsAllowed(1),
	mCurrentJumpsBeforeLand(0),
	mTimeNotOnSolidSurface(0.0f),
	mCurrentSolidLineDroppingDownThroughId(0),
	mIsStrafing(false),
	mStrafeDirectionX(1.0f),
	mTimeOnSolidSurface(0.0f),
	mIsCrouching(false),
	mIsFullyCrouched(false),
	mHighestPointWhileInAir(0.0f),
	mJustFellFromShortDistance(false),
	mJustfellFromLargeDistance(false),
	mLastRunFramePlayed(99999),
	mWallJumpCountdownTime(5.0f),
	mIsWallJumping(false),
	mCurrentWallJumpXDirection(1.0f),
	mIsRolling(false),
	mDoReboundJump(false),
	mDoSwimBurstAnim(false),
	mIsDoingMelee(false),
	mIsDownwardDashing(false),
	mWasDownwardDashing(false),
	mCanIncreaseJumpVelocity(false),
	mStunParticles(nullptr),
	mRegularCollisionBox(0.0f, 0.0f),
	mCollisionBoxOffsetOriginal(0.0f, 0.0f)
{
	mProjectileFilePath = "Media/knife_2.png";
	mProjectileImpactFilePath = "Media/knife_impact_2.png";
	mIsCharacter = true;

	mExplodesGruesomely = true;
}

Character::~Character(void)
{
}

void Character::Scale(float xScale, float yScale, bool scalePosition)
{
	SolidMovingSprite::Scale(xScale, yScale, scalePosition);

	m_maxJumpSpeed = m_maxJumpSpeed * yScale;

	mAccelXRate = mAccelXRate * xScale;

	m_projectileOffset.X = m_projectileOffset.X * xScale;
	m_projectileOffset.Y = m_projectileOffset.Y * xScale;
}

void Character::SetIsWallJumping(bool value)
{
	mIsWallJumping = value;

	if (mIsWallJumping)
	{
		mWallJumpCountdownTime = kWallJumpTime;
	}
}

void Character::DoLargeImpactLanding()
{
	mJustFellFromShortDistance = false;
	mJustfellFromLargeDistance = true;

	StopXAccelerating();

	if (IsPlayer())
	{
		Camera2D::GetInstance()->DoMediumShake(); 
	}
}

void Character::Update(float delta)
{
	if (IsDead())
	{
		return;
	}

	if (mTeleportSfxDelay > 0.0f)
	{
		mTeleportSfxDelay -= delta;
	}

	if (mTeleportCurrentTime > 0.0f)
	{
		mTeleportCurrentTime -= delta;

		if (mTeleportCurrentTime > 0.0f)
		{
			m_alpha = 0.0f;
			return;
		}
		else
		{
			// Just finished teleporting
			FinishTeleport(mTeleportPosition.X, mTeleportPosition.Y, mShowTeleportParticles);
			m_alpha = 1.0f;
		}	
	}

	if (mDoReboundJump)
	{
		Jump(95.0f);
		mDoReboundJump = false;
	}

	// update the base classes
	// TODO: animations are being updated here, should update animations afterwards?
	SolidMovingSprite::Update(delta);

	UpdateCollisionBox();

	if (mCurrentStunTime > 0.0f)
	{
		SetCrouching(true);
		mCurrentStunTime -= delta;

		if (mCurrentStunTime <= 0.0f)
		{
			EnableStunParticles(false);
			if (!IsPlayer())
			{
				if (CanRoll())
				{
					Roll();
				}
				else
				{
					SetCrouching(false);
				}
			}
		}
	}

	if (IsOnSolidSurface())
	{
		if (mWasDownwardDashing)
		{
			DoLargeImpactLanding();
		}
		if (mIsDownwardDashing)
		{
			mWasDownwardDashing = true;
		}
		else
		{
			mWasDownwardDashing = false;
		}
		mIsDownwardDashing = false;
	}

	if (mIsRolling)
	{
		AccelerateX(m_direction.X);

		if (IsPlayer())
		{
			SetMaxVelocityLimitEnabled(false);
		}
	}
	else
	{
		if (!mIsWallJumping)
		{
			SetMaxVelocityLimitEnabled(true);
		}
	}

	if (mIsWallJumping)
	{
		mWallJumpCountdownTime -= delta;

		SetMaxVelocityLimitEnabled(false);
		SetCurrentXResistance(kWallJumpXResistance);

		if (m_direction.X > 0)
		{
			UnFlipHorizontal();
		}
		else
		{
			FlipHorizontal();
		}

		if (mWallJumpCountdownTime <= 0.0f ||
			IsOnSolidSurface())
		{
			mIsWallJumping = false;
		}
	}
	else
	{
		// back to default air resistance with x velocity cap
		if (!mIsRolling)
		{
			SetMaxVelocityLimitEnabled(true);
		}
		SetCurrentXResistance(m_resistance.X);
	}

	if (GetIsCollidingAtObjectSide())
	{
		// sliding on the side of an object so reduce gravity resistance
		mCurrentYResistance = m_resistance.Y * 2.0f; 

		m_velocity.Y *= 0.7f;
	}
	else
	{
		mCurrentYResistance = m_resistance.Y;
	}

	bool isInWater = WasInWaterLastFrame();
	if ((m_acceleration.Y > 0.0f || isInWater) &&
		!m_onTopOfOtherSolidObject && 
		!GetIsCollidingAtObjectSide()) // we are accelerating vertically and not on top of another object
	{
		if ((!isInWater && m_velocity.Y > -0.5f) || (isInWater && m_velocity.Y > 0.0f))
		{
			if (!mIsMidAirMovingUp)
			{
				mMidAirMovingUpStartTime = Timing::Instance()->GetTotalTimeSeconds();
			}
			mIsMidAirMovingUp = true;
			mIsMidAirMovingDown = false;
		}
		else if (!(isInWater && m_velocity.Y <= -0.5f) ||
			(m_velocity.Y <= 0.0f && isInWater))
		{
			if (!mIsMidAirMovingDown)
			{
				mMidAirMovingDownStartTime = Timing::Instance()->GetTotalTimeSeconds();
			}
			mIsMidAirMovingDown = true;
			mIsMidAirMovingUp = false;
		}
	}
	else
	{
		mIsMidAirMovingUp = false;
		mIsMidAirMovingDown = false;
	}

	if (mIsMidAirMovingUp ||
		mIsMidAirMovingDown)
	{
		if (mHighestPointWhileInAir < m_position.Y)
		{
			mHighestPointWhileInAir = m_position.Y;
		}

		// can't be ducking if in mid air
		SetCrouching(false);
	}

	if (GetIsCollidingAtObjectSide())
	{
		mJustfellFromLargeDistance = false;
	}

	if (IsOnSolidSurface())
	{
		if (mHighestPointWhileInAir != m_position.Y)
		{
			float dropDistance = mHighestPointWhileInAir - m_position.Y;

			DoLandOnSolidSurfaceEffects(dropDistance);

			if (IsPlayer())
			{
				// if we pressed roll just before landing then we should roll
				auto inputManager = Game::GetInstance()->GetInputManager();

				float totalTime = Timing::Instance()->GetTotalTimeSeconds();
				float rollDiff = totalTime - inputManager.GetLastTimePressedRoll();
				if (rollDiff >= 0.0f && rollDiff < kLandRollInputWindow)
				{
					Roll();
					if (dropDistance >= kLargeDropDistance)
					{
						Camera2D::GetInstance()->DoMediumShake();
					}

					// we avoided the crouch delay by rolling
					mJustFellFromShortDistance = false;
					mJustfellFromLargeDistance = false;
				}
				else if (!mIsRolling && !isInWater)
				{
					if (dropDistance > kSmallDropDistance && dropDistance < kLargeDropDistance)
					{
						// soft landing
						mJustFellFromShortDistance = true;
						mJustfellFromLargeDistance = false;

						StopXAccelerating();
					}
					else if (dropDistance >= kLargeDropDistance)
					{
						DoLargeImpactLanding();
					}
				}
			}

			mHighestPointWhileInAir = m_position.Y;
		}
		// reset this because we're on solid ground
		mTimeOnSolidSurface += delta;
		if (mCurrentJumpsBeforeLand > 0)
		{
			float timeSinceJump = Timing::Instance()->GetTotalTimeSeconds() - mMidAirMovingUpStartTime;
			if (timeSinceJump > 0.15f )
			{
				mCurrentJumpsBeforeLand = 0;
			}
		}

		mTimeNotOnSolidSurface = 0.0f;
	}
	else
	{
		mTimeNotOnSolidSurface += delta;
		mTimeOnSolidSurface = 0.0f;
	}
}

void Character::ResetJumpHeightVariables()
{
	mHighestPointWhileInAir = -99999.0f;
	mTimeNotOnSolidSurface = 0.0f;
}

void Character::DoLandOnSolidSurfaceEffects(float dropDistance)
{
	if (dropDistance < kSmallDropDistance)
	{
		return;
	}

	if (IsOnSolidLine())
	{
		auto solidLine = GetCurrentSolidLineStrip();

		if (solidLine)
		{
			Material * objectMaterial = solidLine->GetMaterial();
			string particleFile = "";
			if (objectMaterial != nullptr)
			{
				string soundfile = objectMaterial->GetRandomFootstepSoundFilename();
				AudioManager::Instance()->PlaySoundEffect(soundfile);

				particleFile = objectMaterial->GetRandomParticleTexture();

				if (!particleFile.empty())
				{
					if (dropDistance < kLargeDropDistance)
					{
						ParticleEmitterManager::Instance()->CreateDirectedSpray(10,
							Vector2(m_position.X + (m_direction.X * 10.f), CollisionBottom() + 35.0f),
							GetDepthLayer(),
							Vector2(0.0f, 1.0f),
							0.8f,
							Vector2(1200.0f, 720.0f),
							particleFile,
							1.0f,
							1.5f,
							0.8f,
							1.2f,
							32.0f,
							32.0f,
							0.8f,
							false,
							0.8f,
							1.0f,
							1.0f,
							true,
							5.0f,
							5.0f,
							0.2f,
							0.15f,
							0.9f);

						if (IsPlayer() && objectMaterial->ShouldVibrate())
						{
							Game::GetInstance()->Vibrate(0.0f, 0.2f, 0.2f);
						}
					}
					else
					{
						ParticleEmitterManager::Instance()->CreateDirectedSpray(30,
							Vector2(m_position.X + (m_direction.X * 10.f), CollisionBottom()),
							GetDepthLayer(),
							Vector2(0.0f, 1.0f),
							0.4f,
							Vector2(1200.0f, 720.0f),
							particleFile,
							3.0f,
							5.0f,
							0.4f,
							1.0f,
							5.0f,
							10.0f,
							0.8f,
							false,
							0.8f,
							1.0f,
							1.0f,
							true,
							15.0f,
							5.0f,
							0.2f,
							0.15f,
							0.9f);

						if (IsPlayer() && objectMaterial->ShouldVibrate())
						{
							Game::GetInstance()->Vibrate(0.35f, 0.5f, 0.5f);
						}
					}
				}
			}
		}
	}
}

void Character::SetSprintActive(bool value)
{
	mSprintActive = value;

	if (value)
	{
		m_maxVelocity.X = mSprintVelocityX;
	}
	else
	{
		m_maxVelocity.X = mDefaultVelocityX;
	}
}

void Character::Initialise()
{
	// update the base classes
	SolidMovingSprite::Initialise();

	mDefaultVelocityX = m_maxVelocity.X;

	if (m_isAnimated && m_animation->GetPart("arm"))
	{
		m_animation->SetPartSequence("arm", "Still");
	}

	mRegularCollisionBox = m_collisionBoxDimensions;
	mCollisionBoxOffsetOriginal = mCollisionBoxOffset;

	mRegularSpriteSize = m_dimensions;
	mMeleeSpriteSize = mRegularSpriteSize * kMeleeSpriteMultiplier;

	mHighestPointWhileInAir = m_position.Y;
}

void Character::XmlRead(TiXmlElement * element)
{
	// update base classes
	SolidMovingSprite::XmlRead(element);

	m_maxJumpSpeed = XmlUtilities::ReadAttributeAsFloat(element, "maxjumpspeed", "value");

	mAccelXRate = XmlUtilities::ReadAttributeAsFloat(element, "xaccelrate", "value");

	m_projectileOffset.X = XmlUtilities::ReadAttributeAsFloat(element, "projectileoffsetx", "value");
	
	m_projectileOffset.Y = XmlUtilities::ReadAttributeAsFloat(element, "projectileoffsety", "value");
}

void Character::XmlWrite(TiXmlElement * element)
{
	SolidMovingSprite::XmlWrite(element);

	TiXmlElement * jumpSpeedElem = new TiXmlElement("maxjumpspeed");
	jumpSpeedElem->SetAttribute("value", Utilities::ConvertDoubleToString(m_maxJumpSpeed).c_str());
	element->LinkEndChild(jumpSpeedElem);

	TiXmlElement * xaccelrateElem = new TiXmlElement("xaccelrate");
	xaccelrateElem->SetAttribute("value", Utilities::ConvertDoubleToString(mAccelXRate).c_str());
	element->LinkEndChild(xaccelrateElem);

	TiXmlElement * projectileoffsetxElem = new TiXmlElement("projectileoffsetx");
	projectileoffsetxElem->SetAttribute("value", Utilities::ConvertDoubleToString(m_projectileOffset.X).c_str());
	element->LinkEndChild(projectileoffsetxElem);

	TiXmlElement * projectileoffsetyElem = new TiXmlElement("projectileoffsety");
	projectileoffsetyElem->SetAttribute("value", Utilities::ConvertDoubleToString(m_projectileOffset.Y).c_str());
	element->LinkEndChild(projectileoffsetyElem);
}

void Character::LoadContent(ID3D10Device * graphicsdevice)
{
	SolidMovingSprite::LoadContent(graphicsdevice);

	m_mainBodyTexture = m_texture;
}

bool Character::OnCollision(SolidMovingSprite * object)
{
	if (IsTeleporting())
	{
		return false;
	}

	// see if we are doing melee and damage the other object
	if (mIsDoingMelee && 
		object->CanBeStruckByMelee() &&
		!object->IsSolidLineStrip())
	{
		DoMeleeCollisions(object);
	}

	if (!object->IsCharacter() &&
		!object->IsCurrencyOrb() &&
		!object->IsDebris() &&
		!object->IsPlatform() &&
		!object->IsWaterBlock() &&
		!object->IsFoliage())
	{
		SolidMovingSprite::OnCollision(object);
	}
	else if (object->IsPlatform())
	{
		if (Bottom() > object->Y()) // is the bottom of the character above the platform centre point?
		{
			return SolidMovingSprite::OnCollision(object);
		}
	}

	return true;
}

void Character::DoMeleeCollisions(SolidMovingSprite * object)
{
	if (!mIsDoingMelee)
	{
		return;
	}

	if (mCurrentMeleePhase == kMeleePhase3)
	{
		if (m_direction.X > 0)
		{
			// left edge of the object must be greater than the center of the character
			if (object->CollisionLeft() > CollisionLeft())
			{
				object->OnDamage(this, mMeleeDamage, Vector2(0.0f, 0.0f), true);
				object->TriggerMeleeCooldown();

				if (object->IsCharacter())
				{
					Game::GetInstance()->Vibrate(0.6f, 1.0f, 0.3f);
				}
			}
		}
		else if (m_direction.X < 0)
		{
			// right edge of the object must be less than the center of the character
			if (object->CollisionRight() < CollisionRight())
			{
				object->OnDamage(this, mMeleeDamage, Vector2(0.0f, 0.0f), true);
				object->TriggerMeleeCooldown();

				if (object->IsCharacter())
				{
					Game::GetInstance()->Vibrate(0.6f, 1.0f, 0.3f);
				}
			}
		}
	}

	if (WillDeflectProjectile(object->DirectionX(), object->CollisionLeft(), object->CollisionRight()))
	{
		// deflect any projectiles
		if (object->IsProjectile())
		{
			GAME_ASSERT(dynamic_cast<Projectile *>(object));
			Projectile * objAsProj = static_cast<Projectile *>(object);

			if (objAsProj->isDeflectable() && objAsProj->IsActive())
			{
				int yOffset = rand() % 100;
				int randOffsetSign = rand() % 2;
				if (randOffsetSign == 0)
				{
					yOffset *= -1;
				}

				Vector2 newTargetPosition(objAsProj->X() + (500 * -objAsProj->DirectionX()), objAsProj->Y() + yOffset);
				Vector2 newDirection = newTargetPosition - Vector2(objAsProj->X(), objAsProj->Y());
				newDirection.Normalise();

				float speedMultiplier = 2.5f;

				objAsProj->SetVelocityXY((objAsProj->GetSpeed() * speedMultiplier) * newDirection.X, (objAsProj->GetSpeed() * speedMultiplier) * newDirection.Y);

				if (objAsProj->GetOwnerType() == Projectile::kNPCProjectile)
				{
					objAsProj->SetOwnerType(Projectile::kPlayerProjectile);
					objAsProj->SetDamage(9999); // a large amount to instantly kill 
				}
				else if (objAsProj->GetOwnerType() == Projectile::kPlayerProjectile)
				{
					objAsProj->SetOwnerType(Projectile::kNPCProjectile);
				}

				AudioManager::Instance()->PlaySoundEffect(rand() % 2 == 1 ? "projectile_deflect.wav" : "projectile_deflect_2.wav");

				object->TriggerMeleeCooldown();

				if (IsPlayer())
				{
					Game::GetInstance()->DoDamagePauseEffectLonger();

					Camera2D::GetInstance()->DoMediumShake();

					Game::GetInstance()->Vibrate(0.4f, 0.5f, 0.15f);
				}
			}
		}
		else if (object->IsDebris())
		{
			object->SetVelocityX(object->GetVelocity().X * 2.0f);
			object->SetDirectionX(m_direction.X);
		}
	}
}

// This is used for the animation viewer
void Character::UpdateAnimTexture(const string & bodyPart)
{
	Sprite::UpdateAnimTexture(bodyPart);
	m_mainBodyTexture = m_texture;
}

void Character::UpdateAnimations()
{
	AnimationPart * bodyPart = m_animation->GetPart("body");
	GAME_ASSERT(bodyPart);

	bool isInWater = WasInWaterLastFrame();

	if (bodyPart)
	{
		string current_body_sequence_name = bodyPart->CurrentSequence()->Name();

		if (mJustfellFromLargeDistance)
		{
			// if we fell from a large distance the nothing else matters.
			// We pause all character movement and wait until the recover animation is finished
			if (current_body_sequence_name != "ImpactLandHard")
			{
				bodyPart->SetSequence("ImpactLandHard");
			}

			bodyPart->Animate();

			m_texture = bodyPart->CurrentFrame(); // set the current texture

			if (bodyPart->IsFinished())
			{
				mJustfellFromLargeDistance = false;
			}

			m_mainBodyTexture = m_texture;

			return;
		}

		if (mJustFellFromShortDistance)
		{
			if (current_body_sequence_name != "ImpactLandSoft")
			{
				bodyPart->SetSequence("ImpactLandSoft");
			}

			bodyPart->Animate();

			m_texture = bodyPart->CurrentFrame(); // set the current texture

			m_mainBodyTexture = m_texture;

			if (bodyPart->IsFinished())
			{
				mJustFellFromShortDistance = false;
			}
			else
			{
				return;
			}
		}

		if (mIsDoingMelee)
		{
			switch (mCurrentMeleePhase)
			{
				case kMeleePhase1:
				{
					if (current_body_sequence_name != "Melee")
					{
						bodyPart->SetSequence("Melee");
						AudioManager::Instance()->PlaySoundEffect(rand() % 2 == 1 ? "character/sword_1.wav" : "character/sword_3.wav");
					}

					bodyPart->Animate();
					m_texture = bodyPart->CurrentFrame(); // set the current texture

					if (bodyPart->IsFinished())
					{
						mCurrentMeleePhase = kMeleePhase2;
					}
					break;
				}
				case kMeleePhase2:
				{
					if (current_body_sequence_name != "Melee2")
					{
						bodyPart->SetSequence("Melee2");
						//AudioManager::Instance()->PlaySoundEffect("character/sword_2.wav");
					}

					bodyPart->Animate();
					m_texture = bodyPart->CurrentFrame(); // set the current texture

					if (bodyPart->IsFinished())
					{
						mCurrentMeleePhase = kMeleePhase3;
					}
					break;
				}
				case kMeleePhase3:
				{
					if (current_body_sequence_name != "Melee3")
					{
						bodyPart->SetSequence("Melee3");
						AudioManager::Instance()->PlaySoundEffect(rand() % 2 == 1 ? "character/sword_2.wav" : "character/sword_4.wav");
					}

					bodyPart->Animate();
					m_texture = bodyPart->CurrentFrame(); // set the current texture

					if (bodyPart->IsFinished())
					{
						mCurrentMeleePhase = kMeleeFinish;
					}
					break;
				}
				case kMeleeFinish:
				{
					if (current_body_sequence_name != "MeleeFinish")
					{
						bodyPart->SetSequence("MeleeFinish");
					}

					bodyPart->Animate();
					m_texture = bodyPart->CurrentFrame(); // set the current texture

					if (bodyPart->IsFinished())
					{
						mIsDoingMelee = false;
					}

					break;
				}
				default:
				{
					GAME_ASSERT(false);
					break;
				}
			}

			// mJustFellFromDistance = false;
			mIsFullyCrouched = false;
			mWasCrouching = false;
		}
		else if (mIsRolling)
		{
			if (current_body_sequence_name != "Roll")
			{
				bodyPart->SetSequence("Roll");
			}

			bodyPart->Animate();
			m_texture = bodyPart->CurrentFrame(); // set the current texture

			if (bodyPart->IsFinished())
			{
				mIsRolling = false;

				// if we pressed jump while rolling then jump straight away
				auto inputManager = Game::GetInstance()->GetInputManager();
				float totalTime = Timing::Instance()->GetTotalTimeSeconds();
				float jumpDiff = totalTime - inputManager.GetLastTimePressedJump();
				if (jumpDiff >= 0.0f && jumpDiff < kLandJumpInputWindow)
				{
					Jump(70.0f);
				}
			}

			DoAnimationEffectIfApplicable(bodyPart);

			// mJustFellFromDistance = false;
			mIsFullyCrouched = false;
			mWasCrouching = false;
		}
		else if (mIsCrouching)
		{
			if (current_body_sequence_name != "Crouch")
			{
				bodyPart->SetSequence("Crouch");
			}

			bodyPart->Animate();
			m_texture = bodyPart->CurrentFrame(); // set the current texture

			if (bodyPart->IsFinished())
			{
				mIsFullyCrouched = true;
			}

			// mJustFellFromDistance = false;
		}
		else if (mIsMidAirMovingDown) // we are accelerating vertically and not on top of another object
		{
			if (Timing::Instance()->GetTotalTimeSeconds() > mMidAirMovingDownStartTime + kJumpDelay ||
				GetTimeNotOnSolidSurface() > 0.25f ||
				isInWater)
			{
				if (isInWater)
				{
					if (std::abs(VelocityX()) > 0.0f)
					{
						if (current_body_sequence_name != "SwimForward")
						{
							bodyPart->SetSequence("SwimForward");
						}

						bodyPart->AnimateLooped();
					}
					else
					{
						if (current_body_sequence_name != "SwimIdle")
						{
							bodyPart->SetSequence("SwimIdle");
						}

						bodyPart->AnimateLooped();
					}
				}
				else
				{
					if (current_body_sequence_name != "JumpingDown")
					{
						bodyPart->SetSequence("JumpingDown");
					}
					bodyPart->Animate();
				}
			}

			m_texture = bodyPart->CurrentFrame(); // set the current texture
			mIsFullyCrouched = false;
			mWasCrouching = false;
			// mJustFellFromDistance = false;
		}
		else if (mIsMidAirMovingUp)
		{
			if (isInWater)
			{
				if (current_body_sequence_name != "SwimForward")
				{
					bodyPart->SetSequence("SwimForward");
				}

				bodyPart->AnimateLooped();
			}
			else
			{
				if (current_body_sequence_name != "JumpingUp")
				{
					bodyPart->SetSequence("JumpingUp");
				}
				bodyPart->Animate();
			}

			m_texture = bodyPart->CurrentFrame(); // set the current texture

			mIsFullyCrouched = false;
			mWasCrouching = false;
			// mJustFellFromDistance = false;
		}
		else if (GetIsCollidingAtObjectSide() &&
			!IsOnSolidSurface()) // we have jumped at the side of a wall
		{
			if (current_body_sequence_name != "SlidingDown")
			{
				bodyPart->SetSequence("SlidingDown");
			}

			bodyPart->AnimateLooped();

			m_texture = bodyPart->CurrentFrame(); // set the current texture

			mIsFullyCrouched = false;
			mWasCrouching = false;
			// mJustFellFromDistance = false;
		}
		else if ((m_velocity.X > 0.05f || m_velocity.X < -0.05f) && !GetIsCollidingAtObjectSide()) // we are moving left or right and not colliding with the side of an object
		{
			if (mIsStrafing)
			{
				if (mStrafeDirectionX != m_direction.X)
				{
					if (current_body_sequence_name != "StrafeBack")
					{
						bodyPart->SetSequence("StrafeBack");
					}
				}
				else
				{
					if (current_body_sequence_name != "StrafeForward")
					{
						bodyPart->SetSequence("StrafeForward");
					}
				}
			}
			else
			{
				if (current_body_sequence_name != "Running")
				{
					bodyPart->SetSequence("Running");
				}
			}

			bodyPart->AnimateLooped();

			if (mMatchAnimFrameRateWithMovement /*&& !mIsStrafing*/)
			{
				if (WasInWaterLastFrame())
				{
					bodyPart->CurrentSequence()->SetFrameRate(std::abs((m_velocity.X * 2.0f) * mRunAnimFramerateMultiplier), true);
				}
				else
				{
					float animFramerate = std::abs(mSprintActive ? (m_velocity.X * 2.0f) * mRunAnimFramerateMultiplier : (m_velocity.X * 1.8f) * mRunAnimFramerateMultiplier);

					if (Timing::Instance()->GetTimeModifier() < 1.0f)
					{
						animFramerate * mDeltaTimeMultiplierInSloMo;
					}

					bodyPart->CurrentSequence()->SetFrameRate(animFramerate);
				}
			}

			DoAnimationEffectIfApplicable(bodyPart);
			
			m_texture = bodyPart->CurrentFrame(); // set the current texture

			mIsFullyCrouched = false;
			mWasCrouching = false;
			// mJustFellFromDistance = false;

			mLastRunFramePlayed = bodyPart->FrameNumber();
		}
		else if (mWasCrouching)
		{
			if (current_body_sequence_name != "CrouchUp")
			{
				bodyPart->SetSequence("CrouchUp");
			}

			bodyPart->Animate();

			m_texture = bodyPart->CurrentFrame(); // set the current texture

			if (bodyPart->IsFinished())
			{
				mWasCrouching = false;
			}

			mIsFullyCrouched = false;
			//mJustFellFromDistance = false;
		}
		else
		{
			if (current_body_sequence_name != "Still")
			{
				bodyPart->SetSequence("Still");
			}

			bodyPart->AnimateLooped();

			m_texture = bodyPart->CurrentFrame(); // set the current texture

			mIsFullyCrouched = false;
			mWasCrouching = false;
		}

		// update the arm
		AnimationPart * armPart = m_animation->GetPart("arm");

		if(armPart != nullptr)
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

	m_mainBodyTexture = m_texture;
}

void Character::DoAnimationEffectIfApplicable(AnimationPart * bodyPart)
{
	if (mLastRunFramePlayed != bodyPart->FrameNumber() && bodyPart->HasSFXforCurrentFrame())
	{
		// footstep SFX
		if (IsOnSolidLine())
		{
			auto solidLine = GetCurrentSolidLineStrip();

			if (solidLine)
			{
				auto material = solidLine->GetMaterial();

				if (material)
				{
					std::string filename = material->GetRandomFootstepSoundFilename();

					if (!WasInWaterLastFrame())
					{
						AudioManager::Instance()->PlaySoundEffect(filename);
					}
					
					if (!material->IsWater())
					{
						// do particles
						std::string particleFile = material->GetRandomParticleTexture();

						if (!particleFile.empty())
						{
							ParticleEmitterManager::Instance()->CreateDirectedSpray(10,
								Vector2(m_position.X + (m_direction.X * 12.f), CollisionBottom() + 17.0f),
								GetDepthLayer(),
								m_direction.X > 0 ? Vector2(-0.5f, 0.5f) : Vector2(0.5f, 0.5f),
								0.1,
								Vector2(1200.0f, 720.0f),
								particleFile,
								2.0f,
								4.0f,
								0.3f,
								0.7f,
								40,
								50,
								0.5,
								false,
								0.8,
								1.0,
								1,
								true,
								2.5f,
								1.0f,
								0.0f,
								0.15f,
								0.7f);
						}

						if (IsPlayer() && material->ShouldVibrate())
						{
							// Game::GetInstance()->Vibrate(0.03f, 0.03f, 0.06f);
						}
					}
				}
			}
		}
	}
}

bool Character::CanJump() const
{
	if (GetIsInWater())
	{
		// can always "Jump" when in water
		return true;
	}

	if (GetIsCollidingAtObjectSide())
	{
		return false;
	}

	if (mIsRolling)
	{
		return false;
	}

	if (GetCurrentSolidLineDroppingDownThroughId() != -1)
	{
		// we're currently dropping down
		return false;
	}

	if (mCurrentJumpsBeforeLand >= mMaxJumpsAllowed)
	{
		return false;
	}

	// only do these checks if jumping from the ground
	if (mCurrentJumpsBeforeLand == 0)
	{
		// allow a small amount of time to jump just after being on a solid object
		if ((m_velocity.Y < 0.0f && !IsOnSolidSurface() &&
			GetTimeNotOnSolidSurface() > kTimeAllowedToJumpAfterLeaveSolidGround))
		{
			return false;
		}
	}

	return true;
}

bool Character::Jump(float percent)
{
	// set this to false by default, gets set true if we reach the end of this function
	mCanIncreaseJumpVelocity = false;

	if (!CanJump())
	{
		return false;
	}

	if (GetIsInWater() || WasInWaterLastFrame())
	{
		return false; // WaterJump();
	}

	// clamp
	percent = percent > 100.0f ? 100.0f : percent;
	percent = percent <= 0.0f ? 1.0f : percent;

	// play jump sound
	AudioManager::Instance()->PlaySoundEffect("jump.wav");

	if (!mIsMidAirMovingUp)
	{
		mIsMidAirMovingUp = true;
		// force the jump anim to trigger straight away by saying it happened slightly in the past
		mMidAirMovingUpStartTime = Timing::Instance()->GetTotalTimeSeconds() - kJumpDelay;
	}

	// if we are crouching fully then we get a nice boost to our jump
	/*
	if (mIsFullyCrouched &&
		FeatureUnlockManager::GetInstance()->IsFeatureUnlocked(FeatureUnlockManager::kCrouchJump))
	{
		percent *= 5.75f;
	}
	*/

	if (mCurrentJumpsBeforeLand == 0)
	{
		m_velocity.Y = 1.0f;
		m_direction.Y = 1.0f;
		m_acceleration.Y = (m_maxJumpSpeed / 100.0f) * percent;

		// can only increase jump on first jump from land
		mCanIncreaseJumpVelocity = true;
	}
	else
	{
		// DOUBLE JUMP
		m_velocity.Y += 12.0f;

		if (m_velocity.Y < 14.0f)
		{
			m_velocity.Y = 14.0f;
		}

		// we just double jumped so reset jump animation
		AnimationPart * bodyPart = m_animation->GetPart("body");
		GAME_ASSERT(bodyPart);

		bodyPart->Restart();
	}

	SetY(m_position.Y + 10.0f); // bump us up so that solid lines don't keep us grounded

	++mCurrentJumpsBeforeLand;

	DoJumpParticles();

	return true;
}

void Character::DoJumpParticles()
{
	if (!IsOnSolidLine())
	{
		return;
	}

	auto solidLine = GetCurrentSolidLineStrip();

	if (!solidLine)
	{
		return;
	}

	Material * objectMaterial = solidLine->GetMaterial();
	if (!objectMaterial)
	{
		return;
	}

	string particleFile;

	string soundfile = objectMaterial->GetRandomFootstepSoundFilename();
	AudioManager::Instance()->PlaySoundEffect(soundfile);

	particleFile = objectMaterial->GetRandomParticleTexture();

	if (particleFile.empty())
	{
		return;
	}

	ParticleEmitterManager::Instance()->CreateDirectedSpray(10,
		Vector2(m_position.X + (m_direction.X * 10.f), CollisionBottom() + 35.0f),
		GetDepthLayer(),
		Vector2(0.0f, 1.0f),
		0.8f,
		Vector2(1200.0f, 720.0f),
		particleFile,
		1.0f,
		1.5f,
		0.8f,
		1.2f,
		32.0f,
		32.0f,
		0.8f,
		false,
		0.8f,
		1.0f,
		1.0f,
		true,
		5.0f,
		5.0f,
		0.2f,
		0.15f,
		0.9f);
}

bool Character::WaterJump()
{
	float jumpPercent = 40.0f;

	// NOTE: assumes the character is in water
	if (m_velocity.Y < 0.0f)
	{
		// slow gravity
		m_velocity.Y *= 0.2f;
	}

	mCanIncreaseJumpVelocity = true;
	mDoSwimBurstAnim = true;

	m_velocity.Y = 1.0f;
	m_direction.Y = 1.0f;
	m_acceleration.Y = (m_maxJumpSpeed / 100.0f) * jumpPercent;

	// can only increase jump on first jump from land
	mCanIncreaseJumpVelocity = true;

	if (IsOnSolidSurface())
	{
		SetY(m_position.Y + 10.0f); // bump us up so that solid lines don't keep us grounded
	}

	return true;
}

void Character::IncreaseJump(float percent)
{
	if (IsOnSolidSurface())
	{
		return;
	}

	if (GetCurrentSolidLineDroppingDownThroughId() != -1)
	{
		// we're currently dropping down
		return;
	}

	if (WasInWaterLastFrame())
	{
		percent *= kWaterJumpPercentModifier;
	}

	float velY = (m_maxJumpSpeed / 100.0f) * percent;

	float diff = velY - m_velocity.Y;

	if (diff > 0.0f)
	{
		m_velocity.Y += diff;
	}
}

void Character::WallJump(int directionX, float percent)
{
	if(percent > 100.0f)
	{
		percent = 100.0f;
	}
	else if(percent <= 0)
	{
		percent = 1.0f;
	}

	SetDirectionX(directionX);

	if (directionX > 0.0f)
	{
		UnFlipHorizontal();
	}
	else
	{
		FlipHorizontal();
	}
	
	SetMaxVelocityLimitEnabled(false);

	SetVelocityY(20.0f);
	SetVelocityX(kWallJumpVelocityXBoost * directionX);

	SetIsWallJumping(true);

	mCurrentWallJumpXDirection = directionX;

	AudioManager::Instance()->PlaySoundEffect("jump.wav");
}

bool Character::Roll()
{
	if (!CanRoll())
	{
		return false;
	}

	if (mIsRolling ||
		!IsOnSolidSurface() /* ||
		IsStrafing() */)
	{
		return false;
	}

	mIsRolling = true;

	SetMaxVelocityLimitEnabled(false);

	if (std::abs(m_velocity.X) < kRollVelocityX)
	{
		SetVelocityX(m_direction.X * kRollVelocityX);
	}
	else
	{
		if (GetIsSprintActive())
		{
			SetVelocityX(m_velocity.X * 0.4f);
		}
	}

	SetVelocityY(kRollVelocityY);

	return true;
}

void Character::AccelerateX(float directionX, float percent)
{
	if (IsStunned())
	{
		return;
	}

	if (GetIsCollidingAtObjectSide()  && m_direction.X != directionX)
	{
		// don't allow accelerating into the side of solid objects
		return;
	}

	// float deepWaterModifier = WasInWaterLastFrame() ? (IsOnSolidSurface() ? 0.5f : 0.2f) : 1.0f;

	float accelVal = mAccelXRate * percent;

	if (GetIsSprintActive())
	{
		MovingSprite::AccelerateX(directionX, (accelVal * 2.0f) /* * deepWaterModifier */);
	}
	else
	{
		MovingSprite::AccelerateX(directionX, accelVal/* * deepWaterModifier */);
	}

	if (mIsStrafing)
	{
		if (mStrafeDirectionX > 0.0f)
		{
			UnFlipHorizontal();
		}
		else
		{
			FlipHorizontal();
		}
	}
	else
	{
		if (directionX < 0.0f)
		{
			// flip the sprite horizontally
			FlipHorizontal();
		}
		else if (directionX > 0.0f)
		{
			// unflip
			UnFlipHorizontal();
		}
	}
}

void Character::OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode)
{
	if (IsTeleporting())
	{
		return;
	}

	if (mCanBeDamaged)
	{
		mHealth -= damageAmount;

		Vector2 damageDealerDirection = Vector2(0.0f, 0.0f);

		if (damageDealer && damageDealer->IsProjectile() && GameObjectManager::Instance()->GetPlayer() != this)
		{
			Projectile * asProjectile = static_cast<Projectile *>(damageDealer);

			if (asProjectile->GetProjectileType() == Projectile::kBloodFXProjectile)
			{
				// not affected by blood
				return;
			}

			// do a kickback in the x direction
			int dir = (asProjectile->DirectionX() < 0.0f) ? -1 : 1;

			m_position.X += dir * kDamageKickback;

			damageDealerDirection.X = asProjectile->DirectionX();
			damageDealerDirection.Y = asProjectile->DirectionY();
		}

		FireBloodSpatter(Vector2(damageDealerDirection.X, damageDealerDirection.Y), m_position);

		// play sound effect
		float current_time = Timing::Instance()->GetTotalTimeSeconds();
		if (current_time > mLastTimePlayedDamageSound + mDamageSoundDelayMilli)
		{
			if (m_material)
			{
				string soundFile = m_material->GetRandomDamageSoundFilename();
				AudioManager::Instance()->PlaySoundEffect(soundFile);
			}
			mLastTimePlayedDamageSound = current_time;
		}

		if (mHealth <= 0)
		{
			mHealth = 0;
			// DEAD, do dead stuff

			// explode
			if (!mHasExploded)
			{
				if (shouldExplode && mExplodesGruesomely)
				{
					Vector2 pos = m_position;
					pos.Y = Bottom();

					bool loop = false;
					unsigned long loopTime = 10;

					if (m_material != nullptr)
					{
						string particletexFile = /* "Media\\bloodparticle2.png"; */ m_material->GetRandomParticleTexture();

						ParticleEmitterManager::Instance()->CreateDirectedSpray(40,
																				Vector2(pos.X, pos.Y + 30.0f),
																				/* GetDepthLayer() */ GameObject::kPlayer,
																				Vector2(0.2f, 0.8f),
																				0.3f,
																				Vector2(3200.0f, 2000.0f),
																				particletexFile,
																				7.0f,
																				12.0f,
																				0.8f,
																				1.5f,
																				10.0f,
																				40.0f,
																				1.4f,
																				loop,
																				0.75f,
																				1.0f,
																				loopTime,
																				true,
																				8.0f,
																				5.0f,
																				10.0f,
																				0.15f,
																				0.8f);
					}

					bool slowTime = (rand() % 8) == 1;

					static float lastTimeWentSlowMo = 0;
					auto timing = Timing::Instance();
					if (slowTime &&
						lastTimeWentSlowMo + 8.0f < timing->GetTotalTimeSeconds() &&
						Camera2D::GetInstance()->IsObjectInView(this))
					{
						timing->SetTimeModifierForNumSeconds(0.2f, 2.5f);
						lastTimeWentSlowMo = timing->GetTotalTimeSeconds();
					}

					if (m_material)
					{
						// play sound effect
						if (current_time > mLastTimePlayedDeathSFX + kMinTimeBetweenDeathSFX)
						{
							AudioManager::Instance()->PlaySoundEffect(m_material->GetRandomDestroyedSound());

							mLastTimePlayedDeathSFX = current_time;
						}
					}

					// spawn some orbs
					if (GameObjectManager::Instance()->GetPlayer() != this)
					{
						CurrencyOrb::SpawnOrbs(m_position, 3);
					}
				}
				else if (shouldExplode && !mExplodesGruesomely)
				{
					Vector2 pos = m_position;
					pos.Y = CollisionBottom();

					float spawnSpreadX = (m_collisionBoxDimensions.X / 100.0f) * 7.0f;
					float spawnSpreadY = (m_collisionBoxDimensions.Y / 100.0f) * 10.0f;

					ParticleEmitterManager::Instance()->CreateRadialSpray(20,
																			pos,
																			GetDepthLayer(),
																			Vector2(3200.0f, 2000.0f),
																			"Media\\smoke4.png",
																			1.8f,
																			3.5f,
																			0.5f,
																			1.0f,
																			75.0f,
																			150.0f,
																			1.0f,
																			false,
																			0.9f,
																			1.0f,
																			-1.0f,
																			true,
																			3.0f,
																			0.9f,
																			0.8f,
																			spawnSpreadX * 0.7f,
																			spawnSpreadY * 1.5f);

					AudioManager::Instance()->PlaySoundEffect("explosion/smoke_explosion.wav");
				}

				m_alpha = 0.0f;
				mHasExploded = true;

				if (mStunParticles != nullptr)
				{
					mStunParticles->Detach();
					GameObjectManager::Instance()->RemoveGameObject(mStunParticles);
				}

				// mark ourselves for deletion (only if we are not a player)
				if (GameObjectManager::Instance()->GetPlayer() != this)
				{
					GameObjectManager::Instance()->RemoveGameObject(this);
				}
			}
		}

		SolidMovingSprite::OnDamage(damageDealer, damageAmount, pointOfContact);

		// blood explosion by default
		Vector2 point = m_position + pointOfContact;

		if (!mHasExploded || (mHasExploded && !shouldExplode))
		{
			Vector2 pos = Vector2(m_position.X + pointOfContact.X, m_position.Y + pointOfContact.Y);

			if (m_material != nullptr)
			{
				string particletexFile = m_material->GetRandomParticleTexture();

				ParticleEmitterManager::Instance()->CreateDirectedSpray(20,
					pos,
					GetDepthLayer(),
					damageDealerDirection,
					0.2f,
					Vector2(3200.0f, 2000.0f),
					particletexFile,
					4.0f,
					10.0f,
					0.3f,
					0.5f,
					3.0f,
					6.0f,
					2.5f,
					false,
					0.8f,
					1.0f,
					-1.0f,
					true,
					22.0f,
					2.0f,
					2.0f,
					0.05f,
					0.85f);
			}
		}
	}
}

void Character::IncreaseHealth(float value)
{
	mHealth += value;

	if (mHealth > mMaxHealth)
	{
		mHealth = mMaxHealth;
	}
}

void Character::DebugDraw(ID3D10Device *  device)
{
	SolidMovingSprite::DebugDraw(device);
}

void Character::PlayRandomWeaponFireSound()
{
	int randnum = rand() % 3;

	switch(randnum)
	{
		case 0:
			{
				AudioManager::Instance()->PlaySoundEffect("knifeThrow.wav");
				break;
			}
		case 1: 
			{
				AudioManager::Instance()->PlaySoundEffect("knifeThrow2.wav");
				break;
			}
		case 2:
			{
				AudioManager::Instance()->PlaySoundEffect("knifeThrow3.wav");
				break;
			}
		default:
			{
				AudioManager::Instance()->PlaySoundEffect("knifeThrow.wav");
				break;
			}
	}
}

void Character::Draw(ID3D10Device * device, Camera2D * camera)
{
	if (IsOnSolidSurface())
	{
		// TODO: this is a temporary function, just testing the shadow - REMOVE
		// TODO: Really remove this as it's very slow
		DrawUtilities::DrawTexture(Vector3(m_position.X, m_position.Y - 150, GetDepthLayer() + 0.1f), Vector2(200, 128), "Media\\characters\\player\\shadow.png");
	}
	
	// draw the arm first because it should be behind the body
	if (m_isAnimated)
	{
		AnimationPart * arm = m_animation->GetPart("arm");

		if (arm && !arm->IsFinished())
		{
			m_texture = arm->CurrentFrame();
			
			// draw the arm
			SolidMovingSprite::Draw(device, camera);
		}
	}

	m_texture = m_mainBodyTexture;

	SolidMovingSprite::Draw(device, camera);
}

bool Character::DoMeleeAttack()
{
	if (!IsOnSolidSurface())
	{
		return false;
	}

	if (mIsDoingMelee ||
		mIsRolling)
	{
		return false;
	}

	mIsDoingMelee = true;
	mCurrentMeleePhase = kMeleePhase1;
	return true;
}

void Character::DropDown()
{
	auto solidLine = GetCurrentSolidLineStrip();
	if (solidLine)
	{
		SetCurrentSolidLineDroppingDownThroughId(solidLine->ID());
		//if (std::abs(m_velocity.Y < 5.0f))
		{
			m_velocity.Y = -10.0f;
		}
	}
}

void Character::Teleport(float posX, float posY, bool showParticles)
{
	if (!CanTeleport())
	{
		return;
	}

	mTeleportCurrentTime = kTeleportTime;

	// particles in old position
	if (showParticles)
	{
		Vector2 pos = m_position;
		pos.Y = CollisionBottom() + 40;

		float spawnSpreadX = (m_collisionBoxDimensions.X / 100.0f) * 7.0f;
		float spawnSpreadY = (m_collisionBoxDimensions.Y / 100.0f) * 6.0f;

		// show particles in old position
		ParticleEmitterManager::Instance()->CreateRadialSpray(10,
			pos,
			GetDepthLayer(),
			Vector2(3200.0f, 2000.0f),
			"Media\\smoke5.png",
			2.8f,
			5.5f,
			0.5f,
			1.0f,
			75.0f,
			150.0f,
			1.0f,
			false,
			0.5f,
			0.7f,
			-1.0f,
			true,
			5.0f,
			0.2f,
			0.8f,
			spawnSpreadX * 0.7f,
			spawnSpreadY * 1.5f);

		// disabling this until only 1 sound is played for all NPCs teleporting
		if (mTeleportSfxDelay <= 0.0f)
		{
			AudioManager::Instance()->PlaySoundEffect("explosion/smoke_explosion.wav");
		}
	}

	StopXAccelerating();
	m_velocity.Y = 0.0f;
	m_velocity.X = 0.0f;

	mShowTeleportParticles = showParticles;
	mTeleportPosition = Vector2(posX, posY);
}

void Character::FinishTeleport(float posX, float posY, bool showParticles)
{
	m_position.X = posX;
	m_position.Y = posY;
	SetIsOnSolidLine(false, nullptr);

	// particles in new position
	if (showParticles)
	{
		Vector2 pos = m_position;
		pos.Y = CollisionBottom();

		float spawnSpreadX = (m_collisionBoxDimensions.X / 100.0f) * 7.0f;
		float spawnSpreadY = (m_collisionBoxDimensions.Y / 100.0f) * 10.0f;

		ParticleEmitterManager::Instance()->CreateRadialSpray(20,
			pos,
			GetDepthLayer(),
			Vector2(3200.0f, 2000.0f),
			"Media\\smoke4.png",
			1.8f,
			3.5f,
			0.5f,
			1.0f,
			75.0f,
			150.0f,
			1.0f,
			false,
			0.5f,
			0.7f,
			-1.0f,
			true,
			3.0f,
			0.2f,
			0.8f,
			spawnSpreadX * 0.7f,
			spawnSpreadY * 1.5f);
	}
}

void Character::SetCrouching(bool value)
{ 
	if (value == true)
	{
		bool brk = true;
	}

	if (mIsCrouching == true && value == false)
	{
		// come back up
		mWasCrouching = true;
	}

	mIsCrouching = value;
}

bool Character::WillDeflectProjectile(float projectileDirectionX, float projectileCollisionLeft, float projectileCollisionRight)
{
	if (projectileDirectionX == 0.0f)
	{
		return false;
	}

	if ((projectileDirectionX > 0.0f && m_direction.X > 0.0f) ||
		(projectileDirectionX < 0.0f && m_direction.X < 0.0f))
	{
		return false;
	}

	if (mIsDoingMelee &&
		(mCurrentMeleePhase == kMeleePhase1 || mCurrentMeleePhase == kMeleePhase3))
	{
		if (m_direction.X > 0)
		{
			if (projectileDirectionX > 0.0f)
			{
				// character is facing the same direction as the projectile, don't deflect
				return false;
			}
			// left edge of the object must be greater than the center of the character
			if (projectileCollisionLeft > CollisionLeft())
			{
				return true;
			}
		}
		else if (m_direction.X < 0)
		{
			if (projectileDirectionX < 0.0f)
			{
				// character is facing the same direction as the projectile, don't deflect
				return false;
			}
			// right edge of the object must be less than the center of the character
			if (projectileCollisionRight < CollisionRight())
			{
				return true;
			}
		}
	}

	return false;
}

bool Character::DoDownwardDash()
{
	if (IsOnSolidSurface())
	{
		return false;
	}

	if (GetTimeNotOnSolidSurface() < 0.25f)
	{
		return false;
	}

	if (!FeatureUnlockManager::GetInstance()->IsFeatureUnlocked(FeatureUnlockManager::kDownwardDash))
	{
		return false;
	}

	StopYAccelerating();

	m_velocity.Y = -50.0f;

	mCanIncreaseJumpVelocity = false;

	mIsDownwardDashing = true;

	return true;
}

void Character::Stun(float stunTime)
{
	if (mStunParticles == nullptr)
	{
		AddStunParticles();
	}

	EnableStunParticles(true);

	StopXAccelerating();
	StopYAccelerating();
	SetVelocityXY(0.0f, 0.0f);
	mCurrentStunTime = stunTime;
	SetCrouching(true);
}

void Character::AddStunParticles()
{
	if (mStunParticles != nullptr)
	{
		return;
	}

	mStunParticles = ParticleEmitterManager::Instance()->CreateRadialSpray(5,
																			m_position,
																			GetDepthLayer(),
																			Vector2(3200.0f, 2000.0f),
																			"Media\\stun_star.png",
																			1.5f,
																			2.0f,
																			0.5f,
																			1.0f,
																			20.0f,
																			20.0f,
																			0.0f,
																			true,
																			1.0f,
																			1.0f,
																			-1.0f,
																			false,
																			1.5f,
																			0.1f,
																			0.7f,
																			100.0f,
																			50.0f);
	if (mStunParticles)
	{
		mStunParticles->AttachTo(GameObjectManager::Instance()->GetObjectByID(ID()), Vector2(IsHFlipped() ? 50.0f : -50.0f, -5.0f), GetDepthLayer(), true);
	}
}

void Character::EnableStunParticles(bool enabled)
{
	if (mStunParticles == nullptr)
	{
		return;
	}

	mStunParticles->SetUpdateable(enabled);
	mStunParticles->SetAlpha(enabled ? 1.0f : 0.0f);
}

void Character::UpdateCollisionBox()
{
	if (!IsPlayer())
	{
		// Just do this in Player class?
		return;
	}

	m_collisionBoxDimensions = mRegularCollisionBox;
	mCollisionBoxOffset = mCollisionBoxOffsetOriginal;

	if (mIsDoingMelee)
	{
		if (mDoMeleeSpriteResize)
		{
			if (m_dimensions.X != mMeleeSpriteSize.X)
			{
				m_dimensions = mMeleeSpriteSize;
				m_collisionBoxDimensions.Y = mRegularCollisionBox.Y * kMeleeSpriteMultiplier.Y;
				RecalculateVertices();
				m_applyChange = true;
			}
		}

		if (mCurrentMeleePhase == kMeleePhase3)
		{
			// this is so the collision box will overlap more and cause damage
			m_collisionBoxDimensions.X *= mMeleeCollisionBoundsX;
		}
	}
	else
	{
		if (m_dimensions.X != mRegularSpriteSize.X)
		{
			m_dimensions = mRegularSpriteSize;
			RecalculateVertices();
			m_applyChange = true;
		}
	}

#if _DEBUG
	SetupDebugDraw();
#endif
}

void Character::FireBloodSpatter(Vector2 direction, const Vector2 & origin)
{
	if (!mEmitsBlood)
	{
		return;
	}

	float speed = 3.5f;
	float maxProjectileSize = 256.0f;
	float randScale = 0.5f + ((rand() % 50) * 0.01f);
	float projectileSize = maxProjectileSize * randScale;

	string randImpactTex = "Media\\blood_impact.png";
	if (rand() % 2 == 1)
	{
		randImpactTex = "Media\\blood_impact_2.png";
	}

	Projectile * p = new Projectile(Projectile::kUnknownProjectile,
											"Media\\blood_projectile.png",
											randImpactTex,
											origin,
											GetDepthLayer(), // TODO: maybe should be the blood spray layer?
											Vector2(projectileSize, projectileSize),
											Vector2(64.0f, 64.0f),
											direction,
											0.0f,
											speed,
											3.5f);

	p->SetProjectileType(Projectile::kBloodFXProjectile);
	p->SetDoAlphaFadeOut(false);
	p->SetDoScaleFadeOut(true);
	p->SetSpinningMovement(false);
	p->SetShouldRotateToDirection(false);
	p->SetIsNativeDimensions(false);
	// p->FlipVertical();

	GameObjectManager::Instance()->AddGameObject(p);
}

bool Character::IsDead()
{
	return mHealth <= 0.0f;
}





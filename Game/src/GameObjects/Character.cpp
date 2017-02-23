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

float Character::mLastTimePlayedDeathSFX = 0.0f;
static const float kMinTimeBetweenDeathSFX = 0.1f;
static const float kJumpDelay = 0.05f;
static const int kDamageKickback = 20;
static const float kTimeAllowedToJumpAfterLeaveSolidGround = 0.3f;
static const float kSmallDropDistance = 200.0f;
static const float kLargeDropDistance = 600.0f;
static const float kWallJumpTime = 0.3f;
static const float kRollVelocityX = 14.0f;
static const float kRollVelocityY = 0.0f;
static const float kLandRollInputWindow = 0.3f;
static const float kLandJumpInputWindow = 0.3f;
static const float kWallJumpXResistance = 0.99f;
static const float kWallJumpVelocityXBoost = 15.0f;

Character::Character(float x, float y, float z, float width, float height, float breadth): 
	SolidMovingSprite(x,y,z,width, height, breadth),
	m_isJumping(false), 
	m_maxJumpSpeed(10), 
	m_waterWadeSFXTime(1.0f),
	mAccelXRate(0),
	mHealth(100),
	mMaxHealth(100),
	mSprintVelocityX(8),
	mSprintActive(false),
	mHasExploded(false),
	m_mainBodyTexture(0),
	m_projectileOffset(0,0),
	mLastTimePlayedDamageSound(0.0f),
	mDamageSoundDelayMilli(0.15f),
	mRunAnimFramerateMultiplier(1.0f),
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
	mJustFellFromDistance(false),
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
	mCanIncreaseJumpVelocity(false)
{
	mProjectileFilePath = "Media/knife.png";
	mProjectileImpactFilePath = "Media/knife_impact.png";
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
	mJustFellFromDistance = false;
	mJustfellFromLargeDistance = true;

	StopXAccelerating();

	Camera2D::GetInstance()->DoMediumShake();

	if (IsOnSolidLine())
	{
		auto solidLine = GetCurrentSolidLineStrip();

		if (solidLine)
		{
			Material * objectMaterial = solidLine->GetMaterial();
			string particleFile = "";
			if (objectMaterial != 0)
			{
				string soundfile = objectMaterial->GetRandomFootstepSoundFilename();
				AudioManager::Instance()->PlaySoundEffect(soundfile);

				particleFile = objectMaterial->GetRandomParticleTexture();

				if (!particleFile.empty())
				{
					ParticleEmitterManager::Instance()->CreateDirectedSpray(20,
						Vector3(m_position.X + (m_direction.X * 60.f), CollisionBottom(), m_position.Z - 0.1),
						Vector3(0, 1, 0),
						0.4,
						Vector3(1200, 720, 0),
						particleFile,
						3.0f,
						6.0f,
						0.4f,
						1.0f,
						5,
						10,
						0.8,
						false,
						0.8,
						1.0,
						1,
						true,
						15,
						5.0f,
						0.2f,
						0.15f,
						0.9f);
				}
			}
		}
	}
}

void Character::Update(float delta)
{
	if (mDoReboundJump)
	{
		Jump(95.0f);
		mDoReboundJump = false;
	}

	// update the base classes
	SolidMovingSprite::Update(delta);

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

		SetMaxVelocityLimitEnabled(false);
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

	bool inDeepWater = WasInWaterLastFrame() && GetWaterIsDeep();
	if ((m_acceleration.Y > 0.0f || inDeepWater) && !m_onTopOfOtherSolidObject && !GetIsCollidingAtObjectSide()) // we are accelerating vertically and not on top of another object
	{
		if ((!inDeepWater && m_velocity.Y > -0.5) || (inDeepWater && m_velocity.Y > 0.0f))
		{
			if (!mIsMidAirMovingUp)
			{
				mMidAirMovingUpStartTime = Timing::Instance()->GetTotalTimeSeconds();
			}
			mIsMidAirMovingUp = true;
			mIsMidAirMovingDown = false;
		}
		else if (!(inDeepWater && m_velocity.Y <= -0.5) ||
			(m_velocity.Y <= 0.0f && inDeepWater))
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

	bool isInDeepWater = WasInWaterLastFrame() && GetWaterIsDeep();
	if (IsOnSolidSurface())
	{
		if (mHighestPointWhileInAir != m_position.Y)
		{
			float dropDistance = mHighestPointWhileInAir - m_position.Y;

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
				mJustFellFromDistance = false;
				mJustfellFromLargeDistance = false;
			}
			else if (!mIsRolling && !isInDeepWater)
			{
				if (dropDistance > kSmallDropDistance && dropDistance < kLargeDropDistance)
				{
					mJustFellFromDistance = true;
					mJustfellFromLargeDistance = false;

					float jumpDiff = totalTime - inputManager.GetLastTimePressedJump();
					if (!mIsDownwardDashing && !mWasDownwardDashing && jumpDiff >= 0.0f && jumpDiff < kLandJumpInputWindow)
					{
						mDoReboundJump = true;
						Jump(95.0f);
					}
				}
				else if (dropDistance >= kLargeDropDistance)
				{
					DoLargeImpactLanding();
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
		!object->IsWaterBlock())
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
				object->OnDamage(this, mMeleeDamage, Vector3(0, 0, 0), false);
				object->TriggerMeleeCooldown();
			}
		}
		else if (m_direction.X < 0)
		{
			// right edge of the object must be less than the center of the character
			if (object->CollisionRight() < CollisionRight())
			{
				object->OnDamage(this, mMeleeDamage, Vector3(0, 0, 0), false);
				object->TriggerMeleeCooldown();
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

			if (objAsProj->IsActive())
			{
				int yOffset = rand() % 200;
				int randOffsetSign = rand() % 2;
				if (randOffsetSign == 0)
				{
					yOffset *= -1;
				}

				Vector2 newTargetPosition(objAsProj->X() + (500 * -objAsProj->DirectionX()), objAsProj->Y() + yOffset);
				Vector2 newDirection = newTargetPosition - Vector2(objAsProj->X(), objAsProj->Y());
				newDirection.Normalise();

				objAsProj->SetVelocityXYZ(objAsProj->GetSpeed() * newDirection.X, objAsProj->GetSpeed() * newDirection.Y, 0);

				if (objAsProj->GetOwnerType() == Projectile::kNPCProjectile)
				{
					objAsProj->SetOwnerType(Projectile::kPlayerProjectile);
				}
				else if (objAsProj->GetOwnerType() == Projectile::kPlayerProjectile)
				{
					objAsProj->SetOwnerType(Projectile::kNPCProjectile);
				}

				ParticleEmitterManager::Instance()->CreateDirectedSpray(1,
					object->Position(),
					Vector3(0, 0, 0),
					0.4,
					Vector3(3200, 1200, 0),
					"Media\\blast_circle.png",
					0.01,
					0.01,
					0.40f,
					0.40f,
					20,
					20,
					0,
					false,
					0.7,
					1.0,
					10000,
					true,
					2,
					0.0f,
					0.0f,
					0.0f,
					0.3f);

				AudioManager::Instance()->PlaySoundEffect("metalclink.wav");

				Game::GetInstance()->DoDamagePauseEffect();

				object->TriggerMeleeCooldown();
			}
		}
		else if (object->IsDebris())
		{
			object->SetVelocityX(object->GetVelocity().X * 2.0f);
			object->SetDirectionX(m_direction.X);
		}
	}
}

void Character::UpdateAnimations()
{
	AnimationPart * bodyPart = m_animation->GetPart("body");
	GAME_ASSERT(bodyPart);

	bool isInDeepWater = WasInWaterLastFrame() && GetWaterIsDeep();

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

		if (mIsDoingMelee)
		{
			switch (mCurrentMeleePhase)
			{
				case kMeleePhase1:
				{
					if (current_body_sequence_name != "Melee")
					{
						bodyPart->SetSequence("Melee");
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

			mJustFellFromDistance = false;
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

			mJustFellFromDistance = false;
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

			mJustFellFromDistance = false;
		}
		else if (mIsMidAirMovingDown) // we are accelerating vertically and not on top of another object
		{
			if (Timing::Instance()->GetTotalTimeSeconds() > mMidAirMovingDownStartTime + kJumpDelay ||
				GetTimeNotOnSolidSurface() > 0.25f ||
				isInDeepWater)
			{
				if (isInDeepWater)
				{
					if (current_body_sequence_name == "SwimBurst" ||
						current_body_sequence_name == "SwimBurstKick")
					{
						bodyPart->SetSequence("SwimBurstSettle");
						bodyPart->Animate();
					}
					else if (!bodyPart->IsFinished() && current_body_sequence_name == "SwimBurstSettle")
					{
						bodyPart->Animate();
					}
					else if (current_body_sequence_name != "SwimIdle")
					{
						bodyPart->SetSequence("SwimIdle");
						bodyPart->AnimateLooped();
					}
					else
					{
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
			mJustFellFromDistance = false;
		}
		else if (mIsMidAirMovingUp)
		{
			if (isInDeepWater)
			{
				if (current_body_sequence_name != "SwimBurst" &&
					current_body_sequence_name != "SwimBurstKick")
				{
					bodyPart->SetSequence("SwimBurst");
				}

				if (mDoSwimBurstAnim)
				{
					if (bodyPart->IsFinished())
					{
						bodyPart->SetSequence("SwimBurstKick");
						bodyPart->Restart();
					}
					
					mDoSwimBurstAnim = false;
				}
				bodyPart->Animate();
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
			mJustFellFromDistance = false;
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
			mJustFellFromDistance = false;
		}
		else if ((m_velocity.X > 1.0f || m_velocity.X < -1.0f) && !GetIsCollidingAtObjectSide()) // we are moving left or right and not colliding with the side of an object
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

			if (mMatchAnimFrameRateWithMovement && !mIsStrafing)
			{
				if (WasInWaterLastFrame())
				{
					bodyPart->CurrentSequence()->SetFrameRate(std::abs((m_velocity.X * 2.0f) * mRunAnimFramerateMultiplier), true);
				}
				else
				{
					float animFramerate = std::abs(mSprintActive ? (m_velocity.X * 1.6f) * mRunAnimFramerateMultiplier : (m_velocity.X * 1.4f) * mRunAnimFramerateMultiplier);

					bodyPart->CurrentSequence()->SetFrameRate(animFramerate);
				}
			}

			DoAnimationEffectIfApplicable(bodyPart);
			
			m_texture = bodyPart->CurrentFrame(); // set the current texture

			mIsFullyCrouched = false;
			mWasCrouching = false;
			mJustFellFromDistance = false;

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
			mJustFellFromDistance = false;
		}
		else
		{
			if (mJustFellFromDistance)
			{
				if (current_body_sequence_name != "ImpactLandSoft")
				{
					bodyPart->SetSequence("ImpactLandSoft");
				}

				bodyPart->Animate();

				m_texture = bodyPart->CurrentFrame(); // set the current texture

				if (bodyPart->IsFinished())
				{
					mJustFellFromDistance = false;
				}
			}
			else
			{
				if (current_body_sequence_name != "Still")
				{
					bodyPart->SetSequence("Still");
				}

				bodyPart->AnimateLooped();

				m_texture = bodyPart->CurrentFrame(); // set the current texture
			}

			mIsFullyCrouched = false;
			mWasCrouching = false;
		}

		// update the arm
		AnimationPart * armPart = m_animation->GetPart("arm");

		if(armPart != 0)
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

					bool isInDeepWater = WasInWaterLastFrame() && GetWaterIsDeep();
					if (!isInDeepWater)
					{
						AudioManager::Instance()->PlaySoundEffect(filename);
					}
					
					// do particles
					std::string particleFile = material->GetRandomParticleTexture();

					if (!particleFile.empty())
					{
						ParticleEmitterManager::Instance()->CreateDirectedSpray(10,
							Vector3(m_position.X + (m_direction.X * 5.f), CollisionBottom(), m_position.Z + 0.1),
							Vector3(0, 1, 0),
							0.1,
							Vector3(1200, 720, 0),
							particleFile,
							2.0f,
							4.0f,
							0.3f,
							0.7f,
							10,
							20,
							0.5,
							false,
							0.8,
							1.0,
							1,
							true,
							7,
							2.0f,
							0.0f,
							0.15f,
							0.7f);
					}
				}
			}
		}
	}
}

bool Character::Jump(float percent)
{
	// set this to false by default, gets set true if we reach the end of this function
	mCanIncreaseJumpVelocity = false;

	if (GetIsCollidingAtObjectSide())
	{
		return false;
	}

	if (mIsRolling)
	{
		return false;
	}

	if (mCurrentJumpsBeforeLand >= mMaxJumpsAllowed &&
		!(WasInWaterLastFrame() && GetWaterIsDeep()))
	{
		return false;
	}
	
	// only do these checks if jumping from the ground
	if (mCurrentJumpsBeforeLand == 0)
	{
		// allow a small amount of time to jump just after being on a solid object
		if ((m_velocity.Y < 0.0f && !IsOnSolidSurface() &&
			GetTimeNotOnSolidSurface() > kTimeAllowedToJumpAfterLeaveSolidGround) &&
			!(WasInWaterLastFrame() && GetWaterIsDeep()))
		{
			return false;
		}
	}

	if (WasInWaterLastFrame() && GetWaterIsDeep())
	{
		percent *= 0.2f;
		mDoSwimBurstAnim = true;
	}

	if(percent > 100.0f)
	{
		percent = 100.0f;
	}
	else if(percent <= 0.0f)
	{
		percent = 1.0f;
	}

	if (/*m_acceleration.Y == 0 && */ !WasInWaterLastFrame())
	{
		// play jump sound
		AudioManager::Instance()->PlaySoundEffect("jump.wav");
	}

	if (!mIsMidAirMovingUp)
	{
		mIsMidAirMovingUp = true;
		// force the jump anim to trigger straight away by saying it happened slightly in the past
		mMidAirMovingUpStartTime = Timing::Instance()->GetTotalTimeSeconds() - kJumpDelay;
	}

	// if we are crouching fully then we get a nice boost to our jump
	if (mIsFullyCrouched)
	{
		percent *= 4.0f;

		if (IsOnSolidLine())
		{
			auto solidLine = GetCurrentSolidLineStrip();

			if (solidLine)
			{
				Material * objectMaterial = solidLine->GetMaterial();
				string particleFile;
				if (objectMaterial != 0)
				{
					string soundfile = objectMaterial->GetRandomFootstepSoundFilename();
					AudioManager::Instance()->PlaySoundEffect(soundfile);

					particleFile = objectMaterial->GetRandomParticleTexture();

					bool isInDeepWater = WasInWaterLastFrame() && GetWaterIsDeep();
					if (!particleFile.empty())
					{
						ParticleEmitterManager::Instance()->CreateDirectedSpray(40,
							Vector3(m_position.X + (m_direction.X * 60.f), CollisionBottom(), m_position.Z - 0.1),
							Vector3(0, 1, 0),
							0.1,
							Vector3(1200, 720, 0),
							particleFile,
							3.0f,
							10.0f,
							0.4f,
							1.0f,
							5,
							10,
							0.8,
							false,
							0.8,
							1.0,
							1,
							true,
							10,
							3.0f,
							0.0f,
							0.15f,
							0.7f);
					}
				}
			}
		}
	}
	
	if (mCurrentJumpsBeforeLand == 0)
	{
		m_velocity.Y = 0;
		m_direction.Y = 1;
		m_acceleration.Y = (m_maxJumpSpeed / 100) * percent;

		// can only increase jump on first jump from land
		mCanIncreaseJumpVelocity = true;
	}
	else
	{
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

	++mCurrentJumpsBeforeLand;

	return true;
}

void Character::IncreaseJump(float percent)
{
	if (IsOnSolidSurface())
	{
		return;
	}

	float velY = (m_maxJumpSpeed / 100) * percent;

	float diff = velY - m_velocity.Y;

	if (diff > 0)
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
	if (mIsRolling ||
		!IsOnSolidSurface() ||
		IsStrafing())
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
		SetVelocityX(m_velocity.X * 1.5f);
	}

	SetVelocityY(kRollVelocityY);

	return true;
}

void Character::AccelerateX(float directionX)
{
	if (GetIsCollidingAtObjectSide()  && m_direction.X != directionX)
	{
		// don't allow accelerating into the side of solid objects
		return;
	}

	bool isInDeepWater = WasInWaterLastFrame() && GetWaterIsDeep();
	float deepWaterModifier = isInDeepWater ? (IsOnSolidSurface() ? 0.5f : 0.2f) : 1.0f;

	float strafeModifier = mIsStrafing ? 0.3f : 1.0f;

	if (GetIsSprintActive())
	{
		MovingSprite::AccelerateX(directionX, (mAccelXRate * 2) * deepWaterModifier * strafeModifier);
	}
	else
	{
		MovingSprite::AccelerateX(directionX, mAccelXRate * deepWaterModifier * strafeModifier);
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
		if (directionX < 0)
		{
			// flip the sprite horizontally
			FlipHorizontal();
		}
		else if (directionX > 0)
		{
			// unflip
			UnFlipHorizontal();
		}
	}
}

void Character::OnDamage(GameObject * damageDealer, float damageAmount, Vector3 pointOfContact, bool shouldExplode)
{
	if (mCanBeDamaged)
	{
		mHealth -= damageAmount;

		if (damageDealer && damageDealer->IsProjectile() && GameObjectManager::Instance()->GetPlayer() != this)
		{
			Projectile * asProjectile = static_cast<Projectile *>(damageDealer);

			// do a kickback in the x direction
			int dir = (asProjectile->DirectionX() < 0.0f) ? -1 : 1;

			m_position.X += dir * kDamageKickback;
		}

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
					Vector3 pos = m_position;
					pos.Y = Bottom();
					pos.Z = pos.Z -1;

					bool loop = false;
					unsigned long loopTime = -1;

					ParticleEmitterManager::Instance()->CreateDirectedSpray(40,
																			Vector3(pos.X, pos.Y + 30, pos.Z),
																			Vector3(0.2f, 0.8f, 0),
																			0.3f,
																			Vector3(3200, 2000, 0),
																			"Media\\bloodparticle.png",
																			7,
																			12,
																			0.8f,
																			2.0f,
																			100,
																			180,
																			3.4,
																			loop,
																			0.3f,
																			1.0f,
																			loopTime,
																			true,
																			2.6f,
																			5.0f,
																			16.0f,
																			0.15f,
																			0.8f);
																			

					ParticleEmitterManager::Instance()->CreateDirectedSpray(40,
																			pos,
																			Vector3(0.2f, 0.8f, 0),
																			0.2f,
																			Vector3(3200, 2000, 0),
																			"Media\\bloodparticle3.png",
																			16,
																			20,
																			0.6f,
																			0.9f,
																			50,
																			120,
																			1,
																			false,
																			0.7f,
																			1.0f,
																			1.0f,
																			true,
																			2.5f,
																			4.0f,
																			16.0f,
																			0.15f,
																			0.8f);

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
					Vector3 pos = m_position;
					pos.Y = CollisionBottom();
					pos.Z = pos.Z - 1;

					float spawnSpreadX = (m_collisionBoxDimensions.X / 100.0f) * 7.0f;
					float spawnSpreadY = (m_collisionBoxDimensions.Y / 100.0f) * 10.0f;

					ParticleEmitterManager::Instance()->CreateRadialSpray(50,
																			pos,
																			Vector3(3200, 2000, 0),
																			"Media\\smoke4.png",
																			1.8f,
																			3.5f,
																			0.5f,
																			1.0f,
																			75,
																			150,
																			1,
																			false,
																			0.5f,
																			0.7f,
																			-1,
																			true,
																			3.0f,
																			0.9f,
																			0.8f,
																			spawnSpreadX * 0.7f,
																			spawnSpreadY * 1.5f);

					ParticleEmitterManager::Instance()->CreateRadialSpray(50,
																			pos,
																			Vector3(3200, 2000, 0),
																			"Media\\smoke.png",
																			1.0f,
																			2.0f,
																			0.5f,
																			1.0f,
																			200,
																			300,
																			1,
																			false,
																			0.7f,
																			1.0f,
																			-1,
																			true,
																			0.1f,
																			0.1f,
																			0.7f,
																			spawnSpreadX * 1.4f,
																			spawnSpreadY * 1.0f);

					AudioManager::Instance()->PlaySoundEffect("explosion/smoke_explosion.wav");
				}

				m_alpha = 0.0f;
				mHasExploded = true;

				// mark ourselves for deletion (only if we are not a player)
				if (GameObjectManager::Instance()->GetPlayer() != this)
				{
					GameObjectManager::Instance()->RemoveGameObject(this);
				}
			}
		}

		SolidMovingSprite::OnDamage(damageDealer, damageAmount, pointOfContact);

		// blood explosion by default
		Vector3 point = m_position + pointOfContact;
		point.Z = m_position.Z;

		if (!mHasExploded || (mHasExploded && !shouldExplode))
		{
			ParticleEmitterManager::Instance()->CreateRadialBloodSpray(10, point, false, -1);
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
		// DrawUtilities::DrawTexture(Vector3(m_position.X, m_position.Y - 150, m_position.Z), Vector2(200, 150), "Media\\characters\\player\\shadow.png");
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

void Character::DoMeleeAttack()
{
	if (!IsOnSolidSurface())
	{
		return;
	}

	if (mIsDoingMelee ||
		mIsRolling)
	{
		return;
	}

	mIsDoingMelee = true;
	mCurrentMeleePhase = kMeleePhase1;
}

void Character::DropDown()
{
	auto solidLine = GetCurrentSolidLineStrip();
	if (solidLine)
	{
		SetCurrentSolidLineDroppingDownThroughId(solidLine->ID());
		if (std::abs(m_velocity.Y < 1.0f))
		{
			m_velocity.Y = -1.0f;
		}
	}
}

void Character::Teleport(float posX, float posY, bool showParticles)
{
	// particles in old position
	if (showParticles)
	{
		Vector3 pos = m_position;
		pos.Y = CollisionBottom();
		pos.Z = pos.Z - 1;

		float spawnSpreadX = (m_collisionBoxDimensions.X / 100.0f) * 7.0f;
		float spawnSpreadY = (m_collisionBoxDimensions.Y / 100.0f) * 10.0f;

		ParticleEmitterManager::Instance()->CreateRadialSpray(20,
			pos,
			Vector3(3200, 2000, 0),
			"Media\\smoke4.png",
			1.8f,
			3.5f,
			0.5f,
			1.0f,
			75,
			150,
			1,
			false,
			0.5f,
			0.7f,
			-1,
			true,
			3.0f,
			0.9f,
			0.8f,
			spawnSpreadX * 0.7f,
			spawnSpreadY * 1.5f);

		ParticleEmitterManager::Instance()->CreateRadialSpray(20,
			pos,
			Vector3(3200, 2000, 0),
			"Media\\smoke.png",
			1.0f,
			2.0f,
			0.5f,
			1.0f,
			200,
			300,
			1,
			false,
			0.7f,
			1.0f,
			-1,
			true,
			0.1f,
			0.1f,
			0.7f,
			spawnSpreadX * 1.4f,
			spawnSpreadY * 1.0f);

		AudioManager::Instance()->PlaySoundEffect("explosion/smoke_explosion.wav");
	}

	m_position.X = posX;
	m_position.Y = posY;
	StopXAccelerating();
	m_velocity.Y = 0.0f;
	m_velocity.X = 0.0f;

	// particles in new position
	if (showParticles)
	{
		Vector3 pos = m_position;
		pos.Y = CollisionBottom();
		pos.Z = pos.Z - 1;

		float spawnSpreadX = (m_collisionBoxDimensions.X / 100.0f) * 7.0f;
		float spawnSpreadY = (m_collisionBoxDimensions.Y / 100.0f) * 10.0f;

		ParticleEmitterManager::Instance()->CreateRadialSpray(20,
			pos,
			Vector3(3200, 2000, 0),
			"Media\\smoke4.png",
			1.8f,
			3.5f,
			0.5f,
			1.0f,
			75,
			150,
			1,
			false,
			0.5f,
			0.7f,
			-1,
			true,
			3.0f,
			0.9f,
			0.8f,
			spawnSpreadX * 0.7f,
			spawnSpreadY * 1.5f);

		ParticleEmitterManager::Instance()->CreateRadialSpray(20,
																pos,
																Vector3(3200, 2000, 0),
																"Media\\smoke.png",
																1.0f,
																2.0f,
																0.5f,
																1.0f,
																200,
																300,
																1,
																false,
																0.7f,
																1.0f,
																-1,
																true,
																0.1f,
																0.1f,
																0.7f,
																spawnSpreadX * 1.4f,
																spawnSpreadY * 1.0f);
	}
}

void Character::SetCrouching(bool value)
{ 
	if (mIsCrouching == true && value == false)
	{
		// come back up
		mWasCrouching = true;
	}

	mIsCrouching = value;
}

bool Character::WillDeflectProjectile(float projectileDirectionX, float projectileCollisionLeft, float projectileCollisionRight)
{
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

void Character::DoDownwardDash()
{
	if (IsOnSolidSurface())
	{
		return;
	}

	if (GetTimeNotOnSolidSurface() < 0.25f)
	{
		return;
	}

	StopYAccelerating();

	m_velocity.Y = -30.0f;

	mCanIncreaseJumpVelocity = false;

	mIsDownwardDashing = true;
}

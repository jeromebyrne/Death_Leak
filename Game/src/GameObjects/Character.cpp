#include "precompiled.h"
#include "Character.h"
#include "AudioManager.h"
#include "Material.h"
#include "particleEmitterManager.h"
#include "orb.h"
#include "Projectile.h"
#include "platform.h"
#include "BombProjectile.h"
#include "WaterBlock.h"
#include "DrawUtilities.h"
#include "CurrencyOrb.h"
#include "SolidLineStrip.h"

float Character::mLastTimePlayedDeathSFX = 0;
static const float kMinTimeBetweenDeathSFX = 0.1f;
static const float kJumpDelay = 0.13f;
static const int kDamageKickback = 20;
static const float kTimeAllowedToJumpAfterLeaveSolidGround = 0.3f;

Character::Character(float x, float y, float z, float width, float height, float breadth): 
	SolidMovingSprite(x,y,z,width, height, breadth),
	m_isJumping(false), 
	m_maxJumpSpeed(10), 
	m_lastTimePlayedFootstep(0.0f), 
	m_footstepTime(0.35f),
	m_sprintFootstepTime(0.25f),
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
	mPlayFootsteps(true),
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
	mTimeOnSolidSurface(0.0f)
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

void Character::Update(float delta)
{
	// update the base classes
	SolidMovingSprite::Update(delta);

	// Game feature - Characters face less friction if they are sprinting whilst on the side of an object
	if (m_velocity.Y > 0 && m_collidingAtSideOfObject && mSprintActive)
	{
		// sliding on the side of an object so reduce gravity resistance
		mCurrentYResistance = m_resistance.Y * 3.0f; // TODO: read this from xml
	}
	else
	{
		mCurrentYResistance = m_resistance.Y;
	}

	if (m_acceleration.Y > 0 && !m_onTopOfOtherSolidObject && !m_collidingAtSideOfObject) // we are accelerating vertically and not on top of another object
	{
		if (m_velocity.Y > -0.5)
		{
			if (!mIsMidAirMovingUp)
			{
				mMidAirMovingUpStartTime = Timing::Instance()->GetTotalTimeSeconds();
			}
			mIsMidAirMovingUp = true;
			mIsMidAirMovingDown = false;
		}
		else if (m_velocity.Y <= -0.5)
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

	if (!GetWaterIsDeep())
	{
		UpdateWaterWadeSFX();
	}

	if (IsOnSolidSurface())
	{
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
	if( !object->IsCharacter() &&
		!object->IsCurrencyOrb() &&
		!object->IsOrb() &&
		!object->IsPlatform() &&
		!object->IsWaterBlock())
	{
		// update the base classes
		if (SolidMovingSprite::OnCollision(object))
		{
			UpdateFootsteps(object);
		}
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

void Character::UpdateWaterWadeSFX()
{
	if (GetAccelX() > 0.0f && mPlayFootsteps && (WasInWaterLastFrame() || GetIsInWater()))
	{
		float currentTime = Timing::Instance()->GetTotalTimeSeconds();

		if (currentTime > m_lastTimePlayedWaterWadeSFX + m_waterWadeSFXTime)
		{
			const char * fileName = "water/wade_2/1.wav";
			int randNum = rand() % 12;

			switch (randNum)
			{
				case 0:
				{
					fileName = "water/wade_2/1.wav";
					break;
				}
				case 1:
				{
					fileName = "water/wade_2/2.wav";
					break;
				}
				case 2:
				{
						  fileName = "water/wade_2/3.wav";
					break;
				}
				case 3:
				{
						  fileName = "water/wade_2/4.wav";
					break;
				}
				case 4:
				{
						  fileName = "water/wade_2/5.wav";
					break;
				}
				case 5:
				{
						  fileName = "water/wade_2/6.wav";
						  break;
				}
				case 6:
				{
						  fileName = "water/wade_2/7.wav";
						  break;
				}
				case 7:
				{
						  fileName = "water/wade_2/8.wav";
						  break;
				}
				case 8:
				{
						  fileName = "water/wade_2/9.wav";
						  break;
				}
				case 9:
				{
						  fileName = "water/wade_2/10.wav";
						  break;
				}
				case 10:
				{
						  fileName = "water/wade_2/11.wav";
						  break;
				}
				case 11:
				{
						  fileName = "water/wade_2/12.wav";
						  break;
				}
				default:
					break;
			}

			AudioManager::Instance()->PlaySoundEffect(fileName);

			m_lastTimePlayedWaterWadeSFX = currentTime;
		}
	}
}

void Character::UpdateFootsteps(SolidMovingSprite * solidObject)
{
	// play footstep sounds if we are running on top the object
	if (mPlayFootsteps && IsOnSolidSurface() && !m_collidingAtSideOfObject)
	{
		if (m_acceleration.X != 0)
		{
			float currentTime = Timing::Instance()->GetTotalTimeSeconds();

			float footstep_delay = mSprintActive ? m_sprintFootstepTime : m_footstepTime;

			if (currentTime > m_lastTimePlayedFootstep + footstep_delay)
			{
				m_lastTimePlayedFootstep = currentTime;

				Material * objectMaterial = solidObject->GetMaterial();
				string particleFile = "";
				if (objectMaterial != 0)
				{
					if (!GetIsInWater() && !WasInWaterLastFrame())
					{
						string soundfile = objectMaterial->GetRandomFootstepSoundFilename();
						AudioManager::Instance()->PlaySoundEffect(soundfile);
					}
					particleFile = objectMaterial->GetRandomParticleTexture();
				}

				Vector3 pos(m_position.X, Bottom(), m_position.Z - 1);
				Vector3 dir(0.1, 0.9, 0);

				if (!mSprintActive)
				{
					bool isInDeepWater = WasInWaterLastFrame() && GetWaterIsDeep();
					if (!particleFile.empty())
					{
						ParticleEmitterManager::Instance()->CreateDirectedSpray(3,
																				pos,
																				dir,
																				0.25,
																				Vector3(1200, 720, 0),
																				particleFile,
																				isInDeepWater ? 0.4f : 1.0f,
																				isInDeepWater ? 1.5f : 4.0f,
																				isInDeepWater ? 1.4f : 0.6f,
																				isInDeepWater ? 2.5f : 0.8f,
																				2,
																				5,
																				0.2,
																				false,
																				0.8,
																				1.0,
																				1,
																				true,
																				isInDeepWater ? 35 : 25,
																				3.0f,
																				1.0f,
																				0.15f,
																				0.7f);
					}
				}
				else
				{
					bool isInDeepWater = WasInWaterLastFrame() && GetWaterIsDeep();
					if (!particleFile.empty())
					{
						ParticleEmitterManager::Instance()->CreateDirectedSpray(8,
																				pos,
																				dir,
																				0.5,
																				Vector3(1200, 720, 0),
																				particleFile,
																				isInDeepWater ? 0.4f : 1.4,
																				isInDeepWater ? 1.5f : 2.8,
																				isInDeepWater ? 1.4f : 0.3f,
																				isInDeepWater ? 2.5f : 0.8f,
																				2,
																				5,
																				0.2,
																				false,
																				0.8,
																				1.0,
																				1,
																				true,
																				isInDeepWater ? 35 : 32,
																				1.5f,
																				0.5f,
																				0.15f,
																				0.5f);
					}
				}
			}
		}
	}
}

void Character::UpdateAnimations()
{
	AnimationPart * bodyPart = m_animation->GetPart("body");
	GAME_ASSERT(bodyPart);

	if (bodyPart)
	{
		string current_body_sequence_name = bodyPart->CurrentSequence()->Name();

		if(mIsMidAirMovingDown) // we are accelerating vertically and not on top of another object
		{
			if (Timing::Instance()->GetTotalTimeSeconds() > mMidAirMovingDownStartTime + kJumpDelay)
			{
				if (current_body_sequence_name != "JumpingDown")
				{
					bodyPart->SetSequence("JumpingDown");
				}
			}

			bodyPart->Animate();
			m_texture = bodyPart->CurrentFrame(); // set the current texture
		}
		else if (mIsMidAirMovingUp)
		{
			if (Timing::Instance()->GetTotalTimeSeconds() > mMidAirMovingUpStartTime + kJumpDelay)
			{
				if (current_body_sequence_name != "JumpingUp")
				{
					bodyPart->SetSequence("JumpingUp");
				}
			}
			
			bodyPart->Animate();
			m_texture = bodyPart->CurrentFrame(); // set the current texture
		}
		else if ((m_velocity.X > 1 || m_velocity.X < -1) && !m_collidingAtSideOfObject) // we are moving left or right and not colliding with the side of an object
		{
			if (mIsStrafing)
			{
				if (current_body_sequence_name != "Strafing")
				{
					bodyPart->SetSequence("Strafing");
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

			if (mMatchAnimFrameRateWithMovement)
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
			
			m_texture = bodyPart->CurrentFrame(); // set the current texture
		}
		else if(m_acceleration.Y > 0 && m_collidingAtSideOfObject) // we have jumped at the side of a wall
		{
			bodyPart->SetSequence("SlidingDown");

			//bodyPart->Animate();
			
			m_texture = bodyPart->CurrentFrame(); // set the current texture
		}
		else
		{
			if(current_body_sequence_name != "Still")
			{
				bodyPart->SetSequence("Still");
			}

			bodyPart->AnimateLooped();
			
			m_texture = bodyPart->CurrentFrame(); // set the current texture
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

	// DEBUGGING REMOVE ME ===========================
	/*bodyPart = m_animation->GetPart("body");
	GAME_ASSERT(bodyPart);

	bodyPart->SetSequence("JumpingDown");
	bodyPart->SetFrame(1);
	m_texture = bodyPart->CurrentFrame();*/
	//================================================

	m_mainBodyTexture = m_texture;
}

bool Character::Jump(float percent)
{
	if (mCurrentJumpsBeforeLand >= mMaxJumpsAllowed &&
		!(WasInWaterLastFrame() && GetWaterIsDeep()))
	{
		return false;
	}

	// allow a small amount of time to jump just after being on a solid object
	if ((m_velocity.Y < 0.0f && !IsOnSolidSurface() &&
		GetTimeNotOnSolidSurface() > kTimeAllowedToJumpAfterLeaveSolidGround) &&
		!(WasInWaterLastFrame() && GetWaterIsDeep()))
	{
		return false;
	}

	if (WasInWaterLastFrame())
	{
		percent *= 0.32f;
	}

	if(percent > 100)
	{
		percent = 100;
	}
	else if(percent <= 0)
	{
		percent = 1;
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

	m_velocity.Y = 0;
	m_direction.Y = 1;
	m_acceleration.Y = (m_maxJumpSpeed/100) * percent;

	++mCurrentJumpsBeforeLand;

	return true;
}

void Character::WallJump(float percent)
{
	if(percent > 100.0f)
	{
		percent = 100.0f;
	}
	else if(percent <= 0)
	{
		percent = 1.0f;
	}
	
	m_velocity.Y = 0;
	m_velocity.X = 0;
	m_direction.Y = 1.0;
	m_direction.X = m_direction.X * -1;
	m_acceleration.Y = (m_maxJumpSpeed * 0.4 /100) * percent;
	m_acceleration.X = (m_maxJumpSpeed * 10 /100) * percent;

	if(m_direction.X < 0)
	{
		// flip the sprite horizontally
		FlipHorizontal();
	}
	else if(m_direction.X > 0)
	{
		// unflip
		UnFlipHorizontal();
	}

	// play jump sound
	AudioManager::Instance()->PlaySoundEffect("jump.wav");
}

void Character::AccelerateX(float directionX)
{
	float deepWaterModifier = (WasInWaterLastFrame() && GetWaterIsDeep()) ? 0.5f : 1.0f;

	float strafeModifier = mIsStrafing ? 0.4f : 1.0f;

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
}

void Character::dropDown()
{
	auto solidLine = GetCurrentSolidLineStrip();
	if (solidLine)
	{
		setCurrentSolidLineDroppingDownThroughId(solidLine->ID());
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

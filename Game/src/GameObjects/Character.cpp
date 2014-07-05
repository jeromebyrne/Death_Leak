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

float Character::mLastTimePlayedDeathSFX = 0;
static const float kMinTimeBetweenDeathSFX = 0.1f;

Character::Character(float x, float y, float z, float width, float height, float breadth): 
	SolidMovingSprite(x,y,z,width, height, breadth),
	m_isJumping(false), 
	m_maxJumpSpeed(10), 
	m_lastTimePlayedFootstep(0.0f), 
	m_footstepTime(0.3f),
	m_sprintFootstepTime(0.15f),
	mAccelXRate(0),
	mHealth(100),
	mMaxHealth(100),
	mSprintVelocityX(30),
	mSprintActive(false),
	mHasExploded(false),
	m_mainBodyTexture(0),
	m_projectileOffset(0,0),
	mLastTimePlayedDamageSound(0.0f),
	mDamageSoundDelayMilli(0.15f),
	mRunAnimFramerateMultiplier(1.0f),
	mPlayFootsteps(true)
{
	mProjectileFilePath = "Media/knife.png";
	mProjectileImpactFilePath = "Media/knife_impact.png";
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
	// Game feature - Characters face less friction if they are sprinting whilst on the side of an object
	if (m_velocity.Y > 0 && m_collidingAtSideOfObject && mSprintActive)
	{
		// sliding on the side of an object so reduce gravity resistance
		mCurrentYResistance = m_resistance.Y * 2; // TODO: read this from xml
	}
	else
	{
		mCurrentYResistance = m_resistance.Y;
	}

	// update the base classes
	SolidMovingSprite::Update(delta);

	if (Camera2D::GetInstance()->IsObjectInView(this)) // adding this in to disable annoying footstep sounds when a character is far away
	{
		// update footsteps 
		if(mPlayFootsteps && (m_acceleration.X > 0.0 || m_acceleration.X < -0.0) && (m_acceleration.Y <= 0 && !m_onTopOfOtherSolidObject) && !m_collidingAtSideOfObject)
		{
			// update footsteps sound
			float currentTime = Timing::Instance()->GetTotalTimeSeconds();

			float footstep_delay = mSprintActive ? m_sprintFootstepTime : m_footstepTime;

			if(currentTime > m_lastTimePlayedFootstep + footstep_delay)
			{
				// get the ground material
				Material* groundMaterial = Environment::Instance()->GroundMaterial();
				
				// play footstep sounds
				string footstepFile = groundMaterial->GetRandomFootstepSoundFilename();
				AudioManager::Instance()->PlaySoundEffect(footstepFile);

				// show ground particles
				Vector3 pos(m_position.X, Bottom(), m_position.Z);
				Vector3 dir(0.1, 0.9, 0);
				string groundParticleFile = groundMaterial->GetRandomParticleTexture();

				if (!mSprintActive)
				{
					ParticleEmitterManager::Instance()->CreateDirectedSpray(5,
																		pos,
																		dir,
																		0.25,
																		Vector3(1200, 720, 0),
																		groundParticleFile,
																		1,
																		4,
																		0.6f,
																		0.8f,
																		2,
																		5,
																		0.2,
																		false,
																		0.4,
																		1.0,
																		1,
																		true,
																		40);
				}
				else
				{
					ParticleEmitterManager::Instance()->CreateDirectedSpray(15,
																		pos,
																		dir,
																		0.5,
																		Vector3(1200, 720, 0),
																		groundParticleFile,
																		2,
																		6,
																		0.6f,
																		1.0f,
																		2,
																		5,
																		0.2,
																		false,
																		0.4,
																		1.0,
																		1,
																		true,
																		50);
				}
				

				m_lastTimePlayedFootstep = currentTime;
				
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

void Character::OnCollision(SolidMovingSprite * object)
{
	bool isPlatformType = dynamic_cast<Platform *>(object);
	if( !dynamic_cast<Character *>(object) &&  
		!dynamic_cast<Orb *>(object) &&
		!isPlatformType &&
		!dynamic_cast<WaterBlock*>(object))
	{
		// update the base classes
		SolidMovingSprite::OnCollision(object);

		// play footstep sounds if we are running on top the object
		if(mPlayFootsteps && m_onTopOfOtherSolidObject && !m_collidingAtSideOfObject)
		{
			if(m_acceleration.X != 0)
			{
				float currentTime = Timing::Instance()->GetTotalTimeSeconds();
				
				float footstep_delay = mSprintActive ? m_sprintFootstepTime : m_footstepTime;

				if(currentTime > m_lastTimePlayedFootstep + footstep_delay)
				{
					m_lastTimePlayedFootstep = currentTime;

					Material * objectMaterial = object->GetMaterial();
					string particleFile = ""; 
					if(objectMaterial != 0)
					{
						string soundfile = objectMaterial->GetRandomFootstepSoundFilename();
						AudioManager::Instance()->PlaySoundEffect(soundfile);
						particleFile = objectMaterial->GetRandomParticleTexture();
					}


					// show ground particles - TODO, should we show the same particles for all objects?
					//Material* groundMaterial = Environment::Instance()->GroundMaterial();
					Vector3 pos(m_position.X, Bottom(), m_position.Z - 1);
					Vector3 dir(0.1, 0.9, 0);

					if (!mSprintActive)
					{
						if (particleFile != "")
						{
							ParticleEmitterManager::Instance()->CreateDirectedSpray(5,
																				pos,
																				dir,
																				0.25,
																				Vector3(1200, 720, 0),
																				particleFile,
																				1,
																				4,
																				0.6f,
																				0.8f,
																				2,
																				5,
																				0.2,
																				false,
																				0.4,
																				1.0,
																				1,
																				true,
																				40);
						}
					}
					else
					{
						if (particleFile != "")
						{
							ParticleEmitterManager::Instance()->CreateDirectedSpray(15,
																				pos,
																				dir,
																				0.5,
																				Vector3(1200, 720, 0),
																				particleFile,
																				2,
																				6,
																				0.6f,
																				1.0f,
																				2,
																				5,
																				0.2,
																				false,
																				0.4,
																				1.0,
																				1,
																				true,
																				50);
						}
					}
				}
			}
		}
	}
	else if (isPlatformType)
	{
		if (Bottom() > object->Y()) // is the bottom of the character above the platform centre point?
		{
			SolidMovingSprite::OnCollision(object);
		}
	}
	
}

void Character::UpdateAnimations()
{
	AnimationPart * bodyPart = m_animation->GetPart("body");
	
	if (bodyPart)
	{
		string current_body_sequence_name = bodyPart->CurrentSequence()->Name();

		if(m_acceleration.Y > 0 && !m_onTopOfOtherSolidObject && !m_collidingAtSideOfObject) // we are accelerating vertically and not on top of another object
		{
			if (m_velocity.Y > -0.5)
			{
				if(current_body_sequence_name != "JumpingUp")
				{
					bodyPart->SetSequence("JumpingUp");
				}
			}
			else if (m_velocity.Y <= -0.5)
			{
				if(current_body_sequence_name != "JumpingDown")
				{
					bodyPart->SetSequence("JumpingDown");
				}
			}

			bodyPart->Animate();
			m_texture = bodyPart->CurrentFrame(); // set the current texture
		}
		else if((m_velocity.X > 1 || m_velocity.X < -1) && !m_collidingAtSideOfObject) // we are moving left or right and not colliding with the side of an object
		{
			if(current_body_sequence_name != "Running")
			{
				bodyPart->SetSequence("Running");
			}

			bodyPart->AnimateLooped();
			
			float animFramerate = mSprintActive ? (m_velocity.X * 1.6f) * mRunAnimFramerateMultiplier :  (m_velocity.X * 1.4f) * mRunAnimFramerateMultiplier;

			if(animFramerate < 0)
			{
				animFramerate *= -1;
			}
			bodyPart->CurrentSequence()->SetFrameRate(animFramerate);
			
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
	m_mainBodyTexture = m_texture;
}

void Character::Jump(int percent)
{
	if(percent > 100)
	{
		percent = 100;
	}
	else if(percent <= 0)
	{
		percent = 1;
	}

	if(m_acceleration.Y == 0)
	{
		// play jump sound
		AudioManager::Instance()->PlaySoundEffect("jump.wav");
	}

	m_velocity.Y = 0;
	m_direction.Y = 1;
	m_acceleration.Y = (m_maxJumpSpeed/100) * percent;

}

void Character::WallJump(int percent)
{
	if(percent > 100)
	{
		percent = 100;
	}
	else if(percent <= 0)
	{
		percent = 1;
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
	if (GetIsSprintActive())
	{
		MovingSprite::AccelerateX(directionX, mAccelXRate * 2);
	}
	else
	{
		MovingSprite::AccelerateX(directionX, mAccelXRate);
	}

	if(directionX < 0)
	{
		// flip the sprite horizontally
		FlipHorizontal();
	}
	else if(directionX > 0)
	{
		// unflip
		UnFlipHorizontal();
	}
}

void Character::OnDamage(float damageAmount, Vector3 pointOfContact, bool shouldExplode)
{
	if (mCanBeDamaged)
	{
		mHealth -= damageAmount;

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
				if (shouldExplode)
				{
					Vector3 pos = m_position;
					pos.Y = Bottom();
					pos.Z = pos.Z -1;

					bool loop = false;
					unsigned long loopTime = -1;

					ParticleEmitterManager::Instance()->CreateDirectedSpray(40,
																			pos,
																			Vector3(0.2f, 0.8f, 0),
																			0.15f,
																			Vector3(3200, 2000, 0),
																			"Media\\bloodparticle2.png",
																			6,
																			18,
																			1.5f,
																			3.0f,
																			200,
																			300,
																			5,
																			loop,
																			0.4f,
																			1.0f,
																			loopTime,
																			true,
																			0.3f);

					ParticleEmitterManager::Instance()->CreateDirectedSpray(40,
																			pos,
																			Vector3(0.2f, 0.8f, 0),
																			0.15f,
																			Vector3(3200, 2000, 0),
																			"Media\\bloodparticle3.png",
																			17,
																			27,
																			3.0f,
																			3.0f,
																			50,
																			100,
																			5,
																			loop,
																			0.4f,
																			1.0f,
																			loopTime,
																			true,
																			4.5f);
				}


				if (m_material)
				{
					// play sound effect
					if (current_time > mLastTimePlayedDeathSFX + kMinTimeBetweenDeathSFX)
					{
						AudioManager::Instance()->PlaySoundEffect(m_material->GetDestroyedSound());

						mLastTimePlayedDeathSFX = current_time;
					}
				
				}

				m_alpha = 0.0f;
				mHasExploded = true;

				// remove any objects we are attached to
				// TODO:

				// mark ourselves for deletion (only if we are not a player)
				if (GameObjectManager::Instance()->GetPlayer() != this)
				{
					GameObjectManager::Instance()->RemoveGameObject(this);
				}
			}
		}

		SolidMovingSprite::OnDamage(damageAmount, pointOfContact);

		// blood explosion by default
		Vector3 point = m_position + pointOfContact;
		point.Z = pointOfContact.Z;

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
	srand(timeGetTime());
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

bool Character::IsOnSolidSurface()
{
	return IsOnGround() || GetIsCollidingOnTopOfObject();
}

void Character::DoMeleeAttack()
{

}

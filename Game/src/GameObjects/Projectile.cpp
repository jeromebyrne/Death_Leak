#include "precompiled.h"
#include "Projectile.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "Material.h"
#include "NPC.h"
#include "Debris.h"
#include "waterblock.h"
#include "SolidLineStrip.h"
#include "DrawUtilities.h"
#include "Game.h"
#include "Breakable.h"
#include "NPCManager.h"

int Projectile::NUM_PROJECTILES_ACTIVE = 0;

Projectile::Projectile(ProjectileOwnerType ownerType, 
						const string & textureFileName,
						const string & impactTextureFilename,
						Vector2 position,
						DepthLayer depthLayer,
						Vector2 dimensions,
						Vector2 collisionDimensions,
						Vector2 direction,
						float damage,
						float speed,
						float maxTimeInActive)
:SolidMovingSprite(position.X,position.Y, depthLayer,dimensions.X,dimensions.Y), 
	m_isActive(true),
	m_wasActiveLastFrame(true),
	m_maxTimeInActive(maxTimeInActive),
	m_damage(damage),
	m_impactTexture(NULL),
	m_impactTextureFilename(impactTextureFilename),
	mCollidedWithProjectile(false),
	mSpinningMovement(false),
	m_timeBecameInactive(0.0f),
	mOwnerType(ownerType),
	mType(kUnknownProjectileType),
	mReboundRotateRate(0.3f),
	mTrailParticlesAttached(false),
	mSpeed(speed)
{
	NUM_PROJECTILES_ACTIVE++; // increase our world projectile count

	m_direction.X = direction.X;
	m_direction.Y = direction.Y;


	m_velocity.X = direction.X * mSpeed;
	m_velocity.Y = direction.Y * mSpeed;

	m_textureFilename = textureFileName;

	m_collisionBoxDimensions.X = collisionDimensions.X;
	m_collisionBoxDimensions.Y = collisionDimensions.Y;

	// blade projectile by default
	mType = kBladeProjectile;

	mIsProjectile = true;
}

Projectile::~Projectile()
{
}

bool Projectile::OnCollision(SolidMovingSprite* object)
{
	if (object->IsDebris())
	{
		// don't want projectiles to be impeded by debris
		return false;
	}

	if (object->IsBreakable())
	{
		if (GetOwnerType() == kNPCProjectile)
		{
			return false;
		}
		if (static_cast<Breakable *>(object)->GetBreakableState() == Breakable::kBroken)
		{
			return false;
		}

		if (NPCManager::Instance()->IsAnyEnemyNPCInWorld())
		{
			return false;
		}
	}

	if (object->IsButterfly())
	{
		if (!mIsInWater)
		{
			object->OnDamage(this, m_damage, Vector2(0.0f,0.0f));
		}
		return false;
	}

	if (object->IsCurrencyOrb())
	{
		return false;
	}

	GAME_ASSERT((object != this));

  	if (m_isActive)
	{
		if (object->IsSolidLineStrip())
		{
			SolidLineStrip * lineStrip = static_cast<SolidLineStrip*>(object);
			HandleSolidLineStripCollision(lineStrip);
			return false;
		}
		else if (mType == kBloodFXProjectile)
		{
			// blood effect particle doesn't impact anything except the solid lines
			return false;
		}

		bool isCharacter = object->IsCharacter();
		
		if (mIsDeflectable && isCharacter)
		{
			GAME_ASSERT(dynamic_cast<Character*>(object));
			Character * asCharacter = static_cast<Character*>(object);

			if (asCharacter->WillDeflectProjectile(m_direction.X, CollisionLeft(), CollisionRight()))
			{
				// the character class handles the projectile deflection
				return false;
			}
		}

		Player * player = GameObjectManager::Instance()->GetPlayer();
		if (object == player && mOwnerType == Projectile::kPlayerProjectile)
		{
			// Player projectiles don't affect the player
			return false;
		}

		NPC * objAsNPC = dynamic_cast<NPC *>(object); // TODO: optimise with NPC flag
		if (objAsNPC && mOwnerType == Projectile::kNPCProjectile)
		{
			// NPC projectiles don't affect NPCs
			return false;
		}

		if (object->IsWaterBlock())
		{
			// npc projectiles don't damage other npc's
			return false;
		}

		if (object->IsProjectile())
		{
			GAME_ASSERT(dynamic_cast<Projectile *>(object));
			Projectile * objAsProj = static_cast<Projectile *>(object);

			if (objAsProj->GetOwnerType() == kPlayerProjectile &&
				GetOwnerType() == kPlayerProjectile)
			{
			
				if (mIsDeflectable && 
					GetProjectileType() == kBladeProjectile &&
					objAsProj->GetProjectileType() == kBombProjectile)
				{
					// the player has fired a blade at their own bomb to blow it up

					GameObjectManager::Instance()->RemoveGameObject(objAsProj);

					ParticleEmitterManager::Instance()->CreateRadialSpray(1,
																			m_position,
																			GetDepthLayer(),
																			Vector2(3000.0f, 3000.0f),
																			"Media\\spark.png",
																			0.2f,
																			1.0f,
																			0.2f,
																			0.4f,
																			24.0f,
																			36.0f,
																			0.0f,
																			false,
																			1.0f,
																			1.0f,
																			0.0f,
																			true,
																			5.0f,
																			0.1f,
																			0.8f,
																			0.0f,
																			0.0f);

					AudioManager::Instance()->PlaySoundEffect("metalclink.wav");

					Game::GetInstance()->DoDamagePauseEffect();

					return false;
				}
			}

			if (mIsDeflectable &&
				objAsProj->GetOwnerType() != GetOwnerType() &&
				objAsProj->GetOwnerType() == kPlayerProjectile &&
				!mCollidedWithProjectile &&
				!objAsProj->mCollidedWithProjectile)
			{
				int yOffset = rand() % 200;
				int randOffsetSign = rand() % 2;
				if (randOffsetSign == 0)
				{
					yOffset *= -1;
				}

				// pick a position behind the projectile to fire back at 
				int objDirXNormal = objAsProj->DirectionX() > 0 ? 1 : -1;
				Vector2 targetPos = Vector2(objAsProj->X() - 200 * objDirXNormal, objAsProj->Y() + yOffset);

				Vector2 direction = objAsProj->Position() - targetPos;
				direction.Normalise();
				m_direction = direction;
				m_velocity = Vector2(direction.X * 25.0f, direction.Y * 25.0f);

				objAsProj->m_isActive = false;
				objAsProj->m_timeBecameInactive = Timing::Instance()->GetTotalTimeSeconds();
				objAsProj->SetVelocityXY(-m_velocity.X * 0.7f, -5.0f);

				AudioManager::Instance()->PlaySoundEffect("metalclink.wav");

				// 50/50 chance of the players projectile being discarded in a collision
				if (randOffsetSign == 0) 
				{
					objAsProj->mCollidedWithProjectile = true;
				}
				
				// never want to damage the player so set as a player projectile
				mOwnerType = kPlayerProjectile;

				if ((object->IsBreakable() || isCharacter) && Camera2D::GetInstance()->IsObjectInView(this))
				{
					Game::GetInstance()->DoDamagePauseEffect();

					if (object->IsPlayer())
					{
						Camera2D::GetInstance()->DoMediumShake();
					}
				}
			}
			return false;
		}

		if (!mRotateToDirection)
		{
			SetRotationAngle(0);
		}

		// get the offset and attach to the hit object
		Vector2 offset = m_position - object->Position();
  		// offset.Z = object->Position().Z - 0.1; // want to show damage effects on front of the object

		// check if it's a character with skeleton animtion
		Animation * objAnimation = object->GetAnimation();
		if (objAnimation)
 		{
			AnimationPart * animPart = objAnimation->GetPart("body");
			AnimationSkeleton * skeleton = objAnimation->GetSkeletonForCurrentSequence("body");
			
			int frameNumber = animPart->FrameNumber();
			bool hasBonesForFrame = skeleton->HasBonesForFrame(frameNumber);

			if (hasBonesForFrame)
			{
				Vector2 intersectPoint; // Not really needed right now
				bool skeletonCollision = skeleton->HasCollidedOnFrame(frameNumber,
																		object->IsHFlipped(),
																		object->LastPosition(),
																		GetCollisionRayStart(),
																		GetCollisionRayEnd(),
																		intersectPoint);
				if (!skeletonCollision)
				{
					skeletonCollision = skeleton->HasCollidedOnFrame(frameNumber,
																	object->IsHFlipped(),
																	object->LastPosition(),
																	GetLastFrameCollisionRayStart(),
																	GetCollisionRayEnd(),
																	intersectPoint);

					if (!skeletonCollision)
					{
						// we had a skeleton but there were no collisions so the projectile missed
						return false;
					}
				}
			}
		}

		// change our sprite to the impact sprite 
		m_texture = m_impactTexture;
		DoBloodProjectilePositionJitter();

		// damage the other object
		if (!WasInWaterLastFrame())
		{
			object->OnDamage(this, m_damage, offset);
		}
		
		// attach the projectile to the hit object
		if (mSpinningMovement && m_velocity.X < 0)
		{
			// spinning projectiles don't naturally orientate so need to flip it when it attaches
			FlipHorizontal();
		}
		AttachTo(GameObjectManager::Instance()->GetObjectByID(object->ID()), offset, GetDepthLayer());

		// stop the projectile
		m_velocity.X = 0;
		m_velocity.Y = 0;
		m_isActive = false;
		m_timeBecameInactive = Timing::Instance()->GetTotalTimeSeconds();

		// show damage particles
		if (!mIsInWater)
		{
			Material * objectMaterial = object->GetMaterial();
			if(objectMaterial != nullptr)
			{
				// where should the particles spray from
				Vector2 particlePos;
				if(m_direction.X > 0)
				{
					particlePos = Vector2(m_position.X,m_position.Y );
				}
				else
				{
					particlePos = Vector2(m_position.X,m_position.Y );
				}
			
				// show particles
				bool loop = false;
				float minLive = 0.5f;
				float maxLive = 1.0f;
				if (object->IsPlayer() || dynamic_cast<NPC*>(object)) // TODO: optimise with flag
				{
					loop = true;
					minLive = 0.3f;
					maxLive = 0.7f;
				}
			
				if (isCharacter)
				{
					ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateDirectedBloodSpray(40,
																										particlePos,
																										Vector2(-m_direction.X, 0.0f),
																										0.15f,
																										true,
																										1.4f);
					if (spray)
					{
						spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(object->ID()), Vector2(-offset.X, -offset.Y), GetDepthLayer());
					}
				}
				else
				{
					// play sound for non-characters, characters handle their sounds in OnDamage
					string soundFile = objectMaterial->GetRandomDamageSoundFilename();
					AudioManager::Instance()->PlaySoundEffect(soundFile);

					string particleTexFile = objectMaterial->GetRandomParticleTexture();
					 ParticleEmitterManager::Instance()->CreateDirectedSpray(15,
																			particlePos,
																			GetDepthLayer(),
																			Vector2(-m_direction.X, 0.0f),
																			0.4f,
																			Vector2(3200.0f, 1200.0f),
																			particleTexFile,
																			0.5f,
																			2.5f,
																			minLive,
																			maxLive,
																			15.0f,
																			30.0f,
																			1.5f,
																			loop,
																			0.7f,
																			1.0f,
																			10.0f,
																			true,
																			2.5f,
																			0.0f,
																			0.0f,
																			0.15f,
																			0.8f);
				}
			}
		}

		if ((object->IsBreakable() || isCharacter) && Camera2D::GetInstance()->IsObjectInView(this))
		{
			Game::GetInstance()->DoDamagePauseEffect();

			if (object->IsPlayer())
			{
				Camera2D::GetInstance()->DoMediumShake();
			}
		}
	}

	return false;
}

void Projectile::Update(float delta)
{
	float targetDelta = Timing::Instance()->GetTargetDelta();
	float percentDelta = delta / targetDelta;

	if (percentDelta > 2.0f)
	{
		percentDelta = 2.0f;
	}

	if (GetIsInWater())
	{
		DoWaterAccelerationBubbles();
	}

	if (m_isActive)
	{
		// we dont need complicated movement so we'll ignore the MovingSprite class
		Sprite::Update(delta);

		m_velocity.Y -= 0.1f * percentDelta;

		// rotate appropriately
		Vector2 dir = Vector2(m_velocity.X, m_velocity.Y);
		dir.Normalise();

		if (mRotateToDirection)
		{
			if (!mIsInWater)
			{
				if (dir.Y < 0)
				{
					if (dir.X > 0)
					{
						SetRotationAngle(-acos(dir.Dot(Vector2(1, 0))));
					}
					else
					{
						SetRotationAngle(acos(dir.Dot(Vector2(1, 0))));
						FlipVertical();
					}
				}
				else
				{
					if (dir.X > 0)
					{
						SetRotationAngle(acos(dir.Dot(Vector2(1, 0))));
					}
					else
					{
						SetRotationAngle(-acos(dir.Dot(Vector2(1, 0))));
						FlipVertical();
					}
				}
			}
		}
		else if (mSpinningMovement)
		{
			if (!mIsInWater)
			{
				SetRotationAngle((GetRotationAngle() - (20.0f * delta)));
			}
			else
			{
				SetRotationAngle((GetRotationAngle() - (20.0f * delta)));
			}
		}

		// nice simple update
		if (mIsInWater)
		{
			m_velocity.X *= 0.91f; // slow down significantly
			m_velocity.Y *= 0.95f; // slow down significantly
		}
		m_position += (m_velocity * percentDelta) * mProjectileResistance;
	}
	else
	{
		// nice simple update
		m_position += m_velocity * percentDelta;

		// we dont need complicated movement so we'll ignore the MovingSprite class
		Sprite::Update(delta);

		if (mCollidedWithProjectile)
		{
			SetRotationAngle(GetRotationAngle() + mReboundRotateRate);
			m_position.Y -= 1.5f * percentDelta; // fake gravity
		}

		float currentTime = Timing::Instance()->GetTotalTimeSeconds();
		float timeToDie = m_timeBecameInactive + m_maxTimeInActive;

		float timeToLive = timeToDie - currentTime;

		if (mDoAlphaFadeOut)
		{
			m_alpha = timeToLive / m_maxTimeInActive;
		}
		if (mDoScaleFadeOut )
		{
			float timeToDiePercent = timeToLive / m_maxTimeInActive;

			float scaleInTimePercent = 0.95f;

			float scaleOutTimePercent = 0.2f;
			
			if (timeToDiePercent > scaleInTimePercent)
			{
				float timeSlice = (1.0f - scaleInTimePercent);
				float scale = 1.0f - ((timeToDiePercent - scaleInTimePercent) / timeSlice);
				SetMatrixScaleX(scale);
				SetMatrixScaleY(scale);
			}
			else if (timeToDiePercent < scaleOutTimePercent)
			{
				float scale = 1.0f - ((scaleOutTimePercent - timeToDiePercent) / scaleOutTimePercent);
				m_alpha = scale; // also alpha out for scaling objects, just later
				// SetMatrixScaleX(scale);
				// SetMatrixScaleY(scale);
			}
		}

		if (currentTime > timeToDie)
		{
			// time to kill ourselves
			GameObjectManager::Instance()->RemoveGameObject(this, true);
		}
	}

	if (!mTrailParticlesAttached)
	{
		mTrailParticlesAttached = true;

		/*
		ParticleSpray * spray =
		ParticleEmitterManager::Instance()->CreateDirectedSpray(10,
																Vector3(m_position.X, m_position.Y, m_position.Z - 0.01f),
																Vector3(-m_direction.X, -m_direction.Y, 0),
																0.25,
																Vector3(1200, 720, 0),
																"Media//projectile_trail.png",
																0.0f,
																0.0f,
																0.1f,
																0.3f,
																30,
																30,
																0.0,
																true,
																1.0,
																1.0,
																1,
																true,
																0.6,
																2.0f,
																0.0f,
																0.15f,
																0.5f);
		if (spray)
		{
			spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(ID()), Vector3(0, 0, 0));
		}
		*/
	}

	if (!m_isActive)
	{
		Sprite::Update(delta);
	}

	mIsInWater = false;
}

void Projectile::Draw(ID3D10Device * device, Camera2D * camera)
{
	SolidMovingSprite::Draw(device, camera);
}

void Projectile::Scale(float xScale, float yScale, bool scalePosition)
{
	SolidMovingSprite::Scale(xScale, yScale, scalePosition);
	
	// scale the velocity
	// only use xScale
	m_velocity.X = m_velocity.X * xScale;
	m_velocity.Y = m_velocity.Y * xScale;
}

void Projectile::LoadContent(ID3D10Device * graphicsdevice)
{
	SolidMovingSprite::LoadContent(graphicsdevice);

	// load the impact texture
	m_impactTexture = TextureManager::Instance()->LoadTexture(m_impactTextureFilename.c_str());
}

void Projectile::HandleSolidLineStripCollision(SolidLineStrip * solidLineStrip)
{
	Vector2 collisionPosition;

	unsigned int collisionLineIndex = 0;
	if (solidLineStrip->GetProjectileCollisionData(this, collisionPosition, collisionLineIndex))
	{
		if (!mRotateToDirection)
		{
			SetRotationAngle(0);
		}

		// damage the other object
		if (!mIsInWater)
		{
			// TODO: the particle offset is notworking properly
			//OnDamage(m_damage, collisionPosition);
		}

		// stop the projectile
		m_velocity.X = 0;
		m_velocity.Y = 0;
		m_isActive = false;
		m_timeBecameInactive = Timing::Instance()->GetTotalTimeSeconds();

		Material * objectMaterial = solidLineStrip->GetMaterial();
		if (objectMaterial != nullptr)
		{
			// where should the particles spray from
			Vector2 particlePos = solidLineStrip->Position() - collisionPosition;

			// show particles
			bool loop = false;
			float minLive = 0.35f;
			float maxLive = 0.75f;

			// play sound for non-characters, characters handle their sounds in OnDamage

			if (!GetIsInWater())
			{
				string soundFile = objectMaterial->GetRandomDamageSoundFilename();
				AudioManager::Instance()->PlaySoundEffect(soundFile);
			}

			string particleTexFile = objectMaterial->GetRandomParticleTexture();
			bool isInWater = WasInWaterLastFrame();
			ParticleEmitterManager::Instance()->CreateDirectedSpray(10,
																	particlePos,
																	GetDepthLayer(),
																	Vector2(-m_direction.X, -m_direction.Y),
																	0.4f,
																	Vector2(3200.0f, 1200.0f),
																	particleTexFile,
																	isInWater ? 0.4f : 1.0f,
																	isInWater ? 1.5f : 4.0f,
																	isInWater ? 1.4f : minLive,
																	isInWater ? 2.5f : maxLive,
																	10.0f,
																	30.0f,
																	0.7f,
																	loop,
																	0.7f,
																	1.0f,
																	10.0f,
																	true,
																	3.5f,
																	1.5f,
																	3.0f,
																	0.15f,
																	0.6f);

			if (objectMaterial->GetIsPierceable())
			{
				// change our sprite to the impact sprite 
				m_texture = m_impactTexture;

				DoBloodProjectilePositionJitter();
			}
			else
			{
				ReboundOffSolidLine(solidLineStrip, collisionLineIndex);
			}
		}
		else
		{
			// change our sprite to the impact sprite 
			m_texture = m_impactTexture;

			DoBloodProjectilePositionJitter();
		}
	}
}

void Projectile::DoBloodProjectilePositionJitter()
{
	if (mType == kBloodFXProjectile)
	{
		// randomly jitter the y position so that it doesn't leave a uniform pattern
		float randY = ((rand() % 100) * 0.01f) * 20.0f;
		int negativeSign = rand() % 2 == 1;

		if (negativeSign)
		{
			SetY(Y() - randY);
		}
		else
		{
			SetY(Y() + randY);
		}

		SetDepthLayer(GameObject::kGroundBlood);
	}
}

void Projectile::ReboundOffSolidLine(SolidLineStrip * solidLine, unsigned lineIndex)
{
	GAME_ASSERT(solidLine);
	if (!solidLine)
	{
		return;
	}

	const Vector2 lineNormal = solidLine->GetNormalForLineIndex(lineIndex);

	m_direction.X = lineNormal.X;
	m_direction.Y = lineNormal.Y;

	if ((m_direction.Y < 0 &&
		lineNormal.Y > 0) ||
		m_direction.Y > 0 &&
		lineNormal.Y < 0)
	{
		m_direction.Y *= -1.0f;
	}
	
	int randOffsetY = rand() % 20;
	int randOffsetX = rand() % 20;
	int randOffsetSign = rand() % 2;
	if (randOffsetSign == 0)
	{
		randOffsetX *= -1;
	}

	m_direction.X += randOffsetX;
	m_direction.Y += randOffsetY;

	m_direction.Normalise();

	m_velocity = Vector2(m_direction.X * 2.0f, m_direction.Y * 6.0f);
	m_isActive = false;
	m_timeBecameInactive = Timing::Instance()->GetTotalTimeSeconds();
	mCollidedWithProjectile = true; // this makes it spin away
	m_maxTimeInActive = 0.25f;
	mReboundRotateRate = 0.09f;
}

void Projectile::DebugDraw(ID3D10Device * graphicsdevice)
{
	SolidMovingSprite::DebugDraw(graphicsdevice);

	DrawUtilities::DrawLine(GetCollisionRayStart(), GetCollisionRayEnd());
}

Vector2 Projectile::GetCollisionRayStart() const
{
	return Vector2(CollisionCentreX(), CollisionCentreY());
}

Vector2 Projectile::GetCollisionRayEnd() const
{
	return Vector2(CollisionCentreX() + (DirectionX() * CollisionDimensions().X * 0.5f),
					CollisionCentreY() + (DirectionY() * CollisionDimensions().X * 0.5f));
}

Vector2 Projectile::GetLastFrameCollisionRayStart()
{
	Vector2 posDiff = m_position - m_lastPosition;

	return Vector2(CollisionCentreX() - posDiff.X, CollisionCentreY() - posDiff.Y);
}
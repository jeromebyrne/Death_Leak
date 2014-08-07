#include "precompiled.h"
#include "Projectile.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "Material.h"
#include "NPC.h"
#include "orb.h"
#include "waterblock.h"

int Projectile::NUM_PROJECTILES_ACTIVE = 0;

Projectile::Projectile(ProjectileOwnerType ownerType, 
						const char* textureFileName,
						const char * impactTextureFilename,
						Vector3 position,
						Vector2 dimensions,
						Vector2 collisionDimensions,
						Vector2 direction,
						float damage,
						float speed,
						int maxTimeInActive)
:SolidMovingSprite(position.X,position.Y,position.Z,dimensions.X,dimensions.Y), 
	m_isActive(true),
	m_wasActiveLastFrame(true),
	m_maxTimeInActive(maxTimeInActive),
	m_damage(damage),
	m_impactTexture(NULL),
	m_impactTextureFilename(impactTextureFilename),
	mCollidedWithProjectile(false),
	mSpinningMovement(false),
	m_timeBecameInactive(0),
	mOwnerType(ownerType),
	mType(kUnknownProjectileType)
{
	NUM_PROJECTILES_ACTIVE++; // increase our world projectile count

	m_direction.X = direction.X;
	m_direction.Y = direction.Y;

	m_velocity.X = direction.X * speed;
	m_velocity.Y = direction.Y * speed;

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

void Projectile::OnCollision(SolidMovingSprite* object)
{
	if (object->IsSolidLine())
	{
		return;
	}

	if (object->IsButterfly())
	{
		if (!mIsInWater)
		{
			object->OnDamage(m_damage, Vector3(0,0,0));
		}
		return;
	}

	GAME_ASSERT((object != this));

  	if(m_isActive)
	{
		//GAME_ASSERT(mOwnerType != Projectile::kUnknownProjectile);

		Player * player = GameObjectManager::Instance()->GetPlayer();
		if (object == player && mOwnerType == Projectile::kPlayerProjectile)
		{
			// Player projectiles don't affect the player
			return;
		}

		NPC * objAsNPC = dynamic_cast<NPC *>(object);
		if (objAsNPC && mOwnerType == Projectile::kNPCProjectile)
		{
			// NPC projectiles don't affect NPCs
			return;
		}

		if (object->IsWaterBlock())
		{
			// npc projectiles don't damage other npc's
			return;
		}

		if (object->IsProjectile())
		{
			GAME_ASSERT(dynamic_cast<Projectile *>(object));
			Projectile * objAsProj = static_cast<Projectile *>(object);

			if (objAsProj->getOwnerType() == kPlayerProjectile &&
				getOwnerType() == kPlayerProjectile)
			{
				if (getProjectileType() == kBladeProjectile &&
					objAsProj->getProjectileType() == kBombProjectile)
				{
					// the player has fired a blade at their own bomb to blow it up

					GameObjectManager::Instance()->RemoveGameObject(objAsProj);

					ParticleEmitterManager::Instance()->CreateRadialSpray(6,
																			m_position,
																			Vector3(3000, 3000, 1),
																			"Media\\spark.png",
																			0.2f,
																			1.0f,
																			0.2f,
																			0.4f,
																			24,
																			36,
																			0.0f,
																			false,
																			1.0f,
																			1.0f,
																			0,
																			true,
																			5.0);

					AudioManager::Instance()->PlaySoundEffect("metalclink.wav");
					return;
				}
			}

			if (objAsProj->getOwnerType() != getOwnerType() &&
				objAsProj->getOwnerType() == kPlayerProjectile &&
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
				Vector3 targetPos = Vector3(objAsProj->X() - 200 * objDirXNormal, objAsProj->Y() + yOffset, objAsProj->Z());

				Vector3 direction = objAsProj->Position() - targetPos;
				direction.Normalise();
				m_direction = direction;
				m_velocity = Vector3(direction.X * 25, direction.Y * 25, 1);

				objAsProj->m_isActive = false;
				objAsProj->m_timeBecameInactive = Timing::Instance()->GetTotalTimeSeconds();
				objAsProj->SetVelocityXYZ(-m_velocity.X * 0.7, -5, 0);

				ParticleEmitterManager::Instance()->CreateRadialSpray(6, 
																	  m_position,
																	  Vector3(3000,3000, 1),
																	  "Media\\spark.png",
																	  0.2f,
																	  1.0f,
																	  0.2f,
																	  0.4f,
																	  24,
																	  36,
																	  0.0f,
																	  false,
																	  1.0f,
																	  1.0f,
																	  0,
																	  true,
																	  5.0);

				AudioManager::Instance()->PlaySoundEffect("metalclink.wav");

				// 50/50 chance of the players projectile being discarded in a collision
				if (randOffsetSign == 0) 
				{
					objAsProj->mCollidedWithProjectile = true;
				}
				
				// never want to damage the player so set as a player projectile
				mOwnerType = kPlayerProjectile;
			}
			return;
		}

		if (mSpinningMovement)
		{
			SetRotationAngle(0);
		}

		// get the offset and attach to the hit object
		Vector3 offset = m_position - object->Position();
  		offset.Z = object->Position().Z - 0.1; // want to show damage effects on front of the object

		// check if it's a character with skeleton animtion
		if (object->GetAnimation())
 		{
 			bool skel_collision = false;
			vector<AnimationSequence::SkeletonPart> skeleton = object->GetAnimation()->GetSkeletonPartsCurrentFrame("body");
			vector<AnimationSequence::SkeletonPart>::const_iterator iter = skeleton.begin();
			for (;iter != skeleton.end(); ++iter)
			{
				Vector3 bone_world_pos;
				if (object->IsHFlipped())
				{
					bone_world_pos = Vector3(object->Position().X - (*iter).Offset.X,
											 object->Position().Y + (*iter).Offset.Y,
											 m_position.Z);
				}
				else
				{
					bone_world_pos = Vector3(object->Position().X + (*iter).Offset.X,
											 object->Position().Y + (*iter).Offset.Y,
											 m_position.Z);
				}

				float bone_radius = (*iter).Radius;

				Vector3 difference = m_position - bone_world_pos;
				
				float diff_length_sqr = difference.LengthSquared();
				float rad_sqr = bone_radius * bone_radius;

				if (difference.LengthSquared() < bone_radius * bone_radius)
 				{
					Vector3 diff_normal = difference;
					diff_normal.Normalise();
					skel_collision = true;
					m_position.X = (bone_world_pos.X + (diff_normal.X * (bone_radius * 0.5f)));
					m_position.Y = (bone_world_pos.Y + (diff_normal.Y * (bone_radius * 0.5f)));
					offset.X = m_position.X - object->Position().X;
					offset.Y = m_position.Y - object->Position().Y;
					break;
				}
			}

			// we had a skeleton specified but no collisions so just return
			if (!skel_collision && skeleton.size() > 0)
			{
 				return;
			}
		}

		// change our sprite to the impact sprite 
		m_texture = m_impactTexture;

		// damage the other object
		if (!mIsInWater)
		{
			object->OnDamage(m_damage, offset);
		}
		
		// attach the projectile to the hit object
		AttachTo(GameObjectManager::Instance()->GetObjectByID(object->ID()), offset);

		// stop the projectile
		m_velocity.X = 0;
		m_velocity.Y = 0;
		m_isActive = false;
		m_timeBecameInactive = Timing::Instance()->GetTotalTimeSeconds();

		// show damage particles
		if (!mIsInWater)
		{
			Material * objectMaterial = object->GetMaterial();
			if(objectMaterial != 0)
			{
				// where should the particles spray from
				Vector3 particlePos;
				if(m_direction.X > 0)
				{
					//particlePos = Vector3(Right(),m_position.Y, m_position.Z -1 );
					particlePos = Vector3(m_position.X,m_position.Y, m_position.Z -1 );
				}
				else
				{
					//particlePos = Vector3(Left(),m_position.Y, m_position.Z -1 );
					particlePos = Vector3(m_position.X,m_position.Y, m_position.Z -1 );
				}
			
				// show particles
				bool loop = false;
				float minLive = 0.5f;
				float maxLive = 1.0f;
				if (dynamic_cast<Player*>(object) || dynamic_cast<NPC*>(object))
				{
					loop = true;
					minLive = 0.3f;
					maxLive = 0.7f;
				}
			
				if (object->IsCharacter())
				{
					ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateDirectedBloodSpray(40,
																										particlePos,
																										Vector3(-m_direction.X, 0, 0),
																										0.15f,
																										true,
																										1.4f);
					if (spray)
					{
						spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(object->ID()), Vector3(-offset.X, -offset.Y, 0));
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
																			Vector3(-m_direction.X, 0, 0),
																			0.4,
																			Vector3(3200, 1200, 0),
																			particleTexFile,
																			0.5,
																			2.5,
																			minLive,
																			maxLive,
																			15,
																			30,
																			1.5,
																			loop,
																			0.7,
																			1.0,
																			10.0f,
																			true,
																			2.5);
				}
			}
		}
	}
}

void Projectile::Update(float delta)
{
	float targetDelta =  Timing::Instance()->GetTargetDelta();
	float percentDelta = delta / targetDelta;

	if (percentDelta > 1.2f)
	{
		percentDelta = 1.2f; 
	}

	if(m_isActive)
	{
		// apply gravity to the velocity
		if (!mIsInWater)
		{
			m_velocity.Y -= 0.15f * percentDelta;
		}
		else
		{
			m_velocity.Y -= 0.10f * percentDelta;
		}
		
		// rotate appropriately
		Vector2 dir = Vector2(m_velocity.X, m_velocity.Y);
		dir.Normalise();

		if (!mSpinningMovement)
		{
			if (!mIsInWater)
			{
				if (dir.Y < 0)
				{
					if (dir.X > 0)
					{
						SetRotationAngle( -acos(dir.Dot(Vector2(1,0))));
					}
					else
					{
						SetRotationAngle( acos(dir.Dot(Vector2(1,0))));
					}
				}
				else
				{
					if (dir.X > 0)
					{
						SetRotationAngle( acos(dir.Dot(Vector2(1,0))));
					}
					else
					{
						SetRotationAngle( -acos(dir.Dot(Vector2(1,0))));
					}
				}
			}
		}
		else
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
			m_velocity.X *= 0.88f; // slow down significantly
			m_velocity.Y *= 0.9f; // slow down significantly
		}
		m_position += m_velocity * percentDelta;

		// we dont need complicated movement so we'll ignore the MovingSprite class
		Sprite::Update(delta);
	}
	else
	{
		//if(m_wasActiveLastFrame)
		//{
			// nice simple update
			m_position += m_velocity * percentDelta;

			// we dont need complicated movement so we'll ignore the MovingSprite class
			Sprite::Update(delta);

			//m_wasActiveLastFrame = false;
		//}

		if (mCollidedWithProjectile)
		{
			SetRotationAngle(GetRotationAngle() + 0.3f);
		}

		/*if(NUM_PROJECTILES_ACTIVE > MAX_PROJECTLES_ALLOWED)
		{
			GameObjectManager::Instance()->RemoveGameObject_RunTime(this); // just kill straight away
		}
		else // else we fade out of existance
		{*/
			float currentTime = Timing::Instance()->GetTotalTimeSeconds();
			float timeToDie = m_timeBecameInactive + m_maxTimeInActive;

			float timeToLive = timeToDie - currentTime;                                                      
			m_alpha = timeToLive / m_maxTimeInActive;

			if(currentTime > timeToDie)
			{
				// time to kill ourselves
				GameObjectManager::Instance()->RemoveGameObject(this, true);
			}
		//}
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


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
mOwnerType(ownerType)
{
	NUM_PROJECTILES_ACTIVE++; // increase our world projectile count

	m_direction.X = direction.X;
	m_direction.Y = direction.Y;

	m_velocity.X = direction.X * speed;
	m_velocity.Y = direction.Y * speed;

	m_textureFilename = textureFileName;

	m_collisionBoxDimensions.X = collisionDimensions.X;
	m_collisionBoxDimensions.Y = collisionDimensions.Y;
}

Projectile::~Projectile()
{
}

void Projectile::OnCollision(SolidMovingSprite* object)
{
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

		if (dynamic_cast<WaterBlock*>(object))
		{
			// npc projectiles don't damage other npc's
			return;
		}

		Projectile * objAsProj = dynamic_cast<Projectile *>(object);
		if (objAsProj)
		{
			return;

			LOG_INFO("TODO: come back to this when refactor is over");
			/*
			if (objAsProj->getOwnerType() != getOwnerType())
			{
				// set the player as the owner of the projectile and fire it back at the enemy
				Vector3 enemyPosition = ->Position();
				
				Vector3 ownerCollisionBounds;

				// offset a little rather than straight back at the enemy origin
				Character * ownerAsCharacter = dynamic_cast<Character *>(m_owner);
				if (ownerAsCharacter)
				{
					ownerCollisionBounds = ownerAsCharacter->CollisionDimensions();
				}
 				int yOffset = rand() % (int)(ownerCollisionBounds.Y * 0.7);
				int randOffsetSign = rand() % 2;
				if (randOffsetSign == 0)
				{
					yOffset *= -1;
				}
				enemyPosition.Y = enemyPosition.Y + yOffset + ownerAsCharacter->CollisionBoxOffset().Y;

				Vector3 direction = enemyPosition - m_position;
				direction.Normalise();
				m_velocity = Vector3(direction.X * 17, direction.Y * 17, 1);

				objAsProj->m_isActive = false;
				objAsProj->m_timeBecameInactive = Timing::Instance()->GetTotalTimeSeconds();
				objAsProj->SetVelocityXYZ(-m_velocity.X * 0.5, -5, 0);

				ParticleEmitterManager::Instance()->CreateRadialSpray(10, m_position, Vector3(3000,3000, 1), "Media\\spark.png", 2, 5, 0.4f, 0.6f, 30, 40, 2.0, false, 0.8,0.9,0,true, 4.0);
				AudioManager::Instance()->PlaySoundEffect("metalclink.wav");

				objAsProj->mCollidedWithProjectile = true;
				m_owner = player;
			}
			return;
			*/
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
		AttachTo(GameObjectManager::Instance()->GetObjectByID(object->ID()), Vector3(0, 0, 0));

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
			
				if (dynamic_cast<Character*>(object))
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
				SetRotationAngle((GetRotationAngle() + m_velocity.LengthSquared() * 0.001f));
			}
			else
			{
				SetRotationAngle((GetRotationAngle() + m_velocity.LengthSquared() * 0.0002f));
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
			SetRotationAngle(GetRotationAngle() + 0.3);
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


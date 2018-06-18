#include "precompiled.h"
#include "NPC.h"
#include "AIStateFollow.h"
#include "AIStateRepel.h"
#include "AIStateRangeAttack.h"
#include "AIStateGroundAnimalWander.h"
#include "AudioManager.h"
#include "projectile.h"
#include "timing.h"
#include <math.h>
#include "Debris.h"
#include "particleemittermanager.h"
#include "NPCManager.h"
#include "AIStateButterflyWander.h"
#include "AIStateFlying.h"

static float kMinReloadTime = 1.5f;
static float kMaxReloadTime = 2.5f;
static float kDamageComboWindowDelay = 0.5f;
static int kDamageInARowToStun = 6;
static float kTimeStunned = 3.0f;

static const float kHealthBarDimensionsX = 128.0f * 0.5f;
static const float kHealthBarDimensionsY = 8.0f;
static const float kHealthBarOverlayDimensionsX = 128.0f * 0.5f;
static const float kHealthBarOverlayDimensionsY = 8.0f;

static const D3DXVECTOR2 kDefaultTex1 = D3DXVECTOR2(0.0f, 0.0f);
static const D3DXVECTOR2 kDefaultTex2 = D3DXVECTOR2(1.0f, 0.0f);
static const D3DXVECTOR2 kDefaultTex3 = D3DXVECTOR2(1.0f, 1.0f);
static const D3DXVECTOR2 kDefaultTex4 = D3DXVECTOR2(0.0f, 1.0f);

NPC::NPC(float x, float y, DepthLayer depthLayer, float width, float height) :
	Character(x, y, depthLayer, width, height),
	m_player(nullptr),
	m_friendlyFollowState(nullptr),
	m_currentState(nullptr),
	mCheckNPCOverlapCollisions(true),
	mLastFireTime(0.0f),
	mNextFireTime(0.0f),
	m_repelState(nullptr),
	m_rangeAttackState(nullptr),
	mCurrentHealthMeterScale(1.0f),
	m_butterflyWander(nullptr),
	mAddHealthBar(true),
	mIsPlayerEnemy(false),
	mRandHealthBarOffsetY(0.0f),
	mGroundAnimalWanderState(nullptr),
	mFlyingState(nullptr)
{
	mHealth = 20.0f;
	mMaxHealth = 20.0f;
	mProjectileFilePath = "Media/ninjastar.png";
	mProjectileImpactFilePath = "Media/ninjastar_impact.png";
	mHealthMeterHealthBeforeDecrease = mHealth;

	NPCManager::Instance()->AddNPC(this);

	mLastFireTime = Timing::Instance()->GetTotalTimeSeconds();

	mNextFireTime = (rand() % ((int)((kMaxReloadTime - kMinReloadTime) * 100.0f)) + kMinReloadTime * 100.0f);
	mNextFireTime *= 0.01f;

	// initialise the states
	m_friendlyFollowState = new AIStateFollow(this);
	m_repelState = new AIStateRepel(this);
	m_rangeAttackState = new AIStateRangeAttack(this);
	m_butterflyWander = new AIStateButterflyWander(this);
	mGroundAnimalWanderState = new AIStateGroundAnimalWander(this);
	mFlyingState = new AIStateFlying(this);

	// set to default state
	SetState(AIState::kRangeAttack);

	mAlwaysUpdate = true;
}

NPC::~NPC(void)
{
	NPCManager::Instance()->RemoveNPC(this);
}

void NPC::Update(float delta)
{
	Character::Update(delta);

	if (mDamageInARowCountdown >0.0f)
	{
		mDamageInARowCountdown -= delta;
		if (mDamageInARowCountdown <= 0.0f)
		{
			mDamageInARowCount = 0;
			mDamageInARowCountdown = 0.0f;
		}
	}

	if (!m_player)
	{
		m_player = GameObjectManager::Instance()->GetPlayer();
	}

	if (m_currentState)
	{
		m_currentState->Update(delta);
	}

	if (mHealthBarUnderlaySprite)
	{
		mHealthBarUnderlaySprite->Update(delta);
	}

	if (mHealthBarSprite)
	{
		mHealthBarSprite->Update(delta);
	}

	if (mHealthBarOverlaySprite)
	{
		mHealthBarOverlaySprite->Update(delta);
	}

	UpdateHealthBar(delta);
}

void NPC::Initialise()
{
	Character::Initialise();

	if (mAddHealthBar)
	{
		AddHealthBar();

		UpdateHealthBar(1.0f);
	}
}

void NPC::SetState(AIState::AIStateType state)
{
	switch (state)
	{
		case AIState::kFriendlyFollowing:
		{
			GAME_ASSERT(m_friendlyFollowState);
			m_currentState = m_friendlyFollowState;
			m_currentState->OnTransition();
			break;
		}
		case AIState::kRepel:
		{
			GAME_ASSERT(m_repelState);
			m_currentState = m_repelState;
			m_currentState->OnTransition();
			break;
		}
		case AIState::kRangeAttack:
		{
			GAME_ASSERT(m_rangeAttackState);
			m_currentState = m_rangeAttackState;
			m_currentState->OnTransition();
			break;
		}
		case AIState::kButterflyWander:
		{
			GAME_ASSERT(m_butterflyWander);
			m_currentState = m_butterflyWander;
			m_currentState->OnTransition();
			break;
		}
		case AIState::kGroundAnimalWander:
		{
			GAME_ASSERT(mGroundAnimalWanderState);
			m_currentState = mGroundAnimalWanderState;
			m_currentState->OnTransition();
			break;
		}
		case AIState::kFlying:
		{
			GAME_ASSERT(mFlyingState);
			m_currentState = mFlyingState;
			m_currentState->OnTransition();
			break;
		}
		case AIState::kNone:
		{
			// do.. nothing
			m_currentState = nullptr;
			break;
		}
		default:
		{
			GAME_ASSERT(false);
			break;
		}
	}
}

void NPC::FireProjectileAtObject(GameObject * target)
{
	if (!target)
	{
		GAME_ASSERT(target);
		return;
	}

	if (mLastFireTime + mNextFireTime < Timing::Instance()->GetTotalTimeSeconds())
	{
		float randYOffset = rand() % 220;

		unsigned randSign = rand() % 2;
		if (randSign == 1)
		{
			// don't fire right into the ground
			randYOffset *= -0.20f;
		}

		Vector2 dir = Vector2(target->Position().X - m_position.X, (target->Position().Y + randYOffset) - m_position.Y);
		dir.Normalise();

		GameObjectManager::Instance()->AddGameObject(FireWeapon(dir));

		mLastFireTime = Timing::Instance()->GetTotalTimeSeconds();

		mNextFireTime = (rand() % ((int)((kMaxReloadTime - kMinReloadTime) * 100.0f)) + kMinReloadTime * 100.0f);
		mNextFireTime *= 0.01f;
	}
}

void NPC::OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode)
{
	mDamageInARowCountdown = kDamageComboWindowDelay;
	++mDamageInARowCount;

	if (!IsStunned() && IsOnSolidSurface() && mDamageInARowCount >= kDamageInARowToStun)
	{
		Stun(kTimeStunned);
	}

	Character::OnDamage(damageDealer, damageAmount, pointOfContact, shouldExplode);

	// throw out some orbs
	int particleNUmPerOrb = 25;
	if (mHasExploded && mExplodesGruesomely)
	{
		// decapitate
		// if (m_isAnimated && m_animationFile == "XmlFiles\\ninjaAnimation2.xml")
		// {
		float speedMultiplier = 1.6f;
			{
				Debris * head = new Debris(nullptr, Vector3(m_position.X, m_position.Y + 50, m_position.Z - 1.1f), Vector3(54, 60, 0), Vector3(30, 30, 0), "Media\\characters\\ninja_enemy_1\\decapitated_head.png", true, speedMultiplier);
				GameObjectManager::Instance()->AddGameObject(head);

				ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, m_position, true, 2.0f);
				if (spray)
				{
					spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(head->ID()), Vector3(0, 0, 0));
				}
			}

			{
				Debris * arm1 = new Debris(nullptr, Vector3(m_position.X - 50, m_position.Y + 50, m_position.Z - 1.1f), Vector3(80, 80, 0), Vector3(30, 30, 0), "Media\\characters\\ninja_enemy_1\\arm_destroyed.png", true, speedMultiplier);
				GameObjectManager::Instance()->AddGameObject(arm1);

				ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, m_position, true, 2.0f);
				if (spray)
				{
					spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(arm1->ID()), Vector3(0, 0, 0));
				}
			}

			{
				Debris * arm2 = new Debris(nullptr, Vector3(m_position.X + 50, m_position.Y + 50, m_position.Z - 1.1f), Vector3(80, 80, 0), Vector3(30, 30, 0), "Media\\characters\\ninja_enemy_1\\arm_destroyed.png", true, speedMultiplier);
				GameObjectManager::Instance()->AddGameObject(arm2);

				ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, m_position, true, 2.0f);
				if (spray)
				{
					spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(arm2->ID()), Vector3(0, 0, 0));
				}
			}

			{
				Debris * leg1 = new Debris(nullptr, Vector3(m_position.X + 50, m_position.Y - 50, m_position.Z - 1.1f), Vector3(117, 153, 0), Vector3(30, 30, 0), "Media\\characters\\ninja_enemy_1\\leg_destroyed.png", true, speedMultiplier);
				GameObjectManager::Instance()->AddGameObject(leg1);

				ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, m_position, true, 2.0f);
				if (spray)
				{
					spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(leg1->ID()), Vector3(0, 0, 0));
				}
			}

			{
				Debris * leg2 = new Debris(nullptr, Vector3(m_position.X - 50, m_position.Y - 50, m_position.Z - 1.1f), Vector3(117, 153, 0), Vector3(30, 30, 0), "Media\\characters\\ninja_enemy_1\\leg_destroyed.png", true, speedMultiplier);
				GameObjectManager::Instance()->AddGameObject(leg2);

				ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, m_position, true, 2.0f);
				if (spray)
				{
					spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(leg2->ID()), Vector3(0, 0, 0));
				}
			}
		// }
		/*
		else if (m_isAnimated && m_animationFile == "XmlFiles\\ninjaAnimation3.xml")
		{
			{
				Debris * head = new Debris(nullptr, Vector3(m_position.X, m_position.Y + 50, m_position.Z - 1.1f), Vector3(54, 60, 0), Vector3(30, 30, 0), "Media\\characters\\ninja_enemy_3\\decapitated_head.png", true, 1.4f);
				GameObjectManager::Instance()->AddGameObject(head);

				ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, m_position, true, 2.0f);
				if (spray)
				{
					spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(head->ID()), Vector3(0, 0, 0));
				}
			}

			{
				Debris * arm1 = new Debris(nullptr, Vector3(m_position.X - 50, m_position.Y + 50, m_position.Z - 1.1f), Vector3(80, 80, 0), Vector3(30, 30, 0), "Media\\characters\\ninja_enemy_3\\arm_destroyed.png", true, 1.4f);
				GameObjectManager::Instance()->AddGameObject(arm1);

				ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, m_position, true, 2.0f);
				if (spray)
				{
					spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(arm1->ID()), Vector3(0, 0, 0));
				}
			}

			{
				Debris * arm2 = new Debris(nullptr, Vector3(m_position.X + 50, m_position.Y + 50, m_position.Z - 1.1f), Vector3(80, 80, 0), Vector3(30, 30, 0), "Media\\characters\\ninja_enemy_3\\arm_destroyed.png", true, 1.4f);
				GameObjectManager::Instance()->AddGameObject(arm2);

				ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, m_position, true, 2.0f);
				if (spray)
				{
					spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(arm2->ID()), Vector3(0, 0, 0));
				}
			}

			{
				Debris * leg1 = new Debris(nullptr, Vector3(m_position.X + 50, m_position.Y - 50, m_position.Z - 1.1f), Vector3(117, 153, 0), Vector3(30, 30, 0), "Media\\characters\\ninja_enemy_3\\leg_destroyed.png", true, 1.4f);
				GameObjectManager::Instance()->AddGameObject(leg1);

				ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, m_position, true, 2.0f);
				if (spray)
				{
					spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(leg1->ID()), Vector3(0, 0, 0));
				}
			}

			{
				Debris * leg2 = new Debris(nullptr, Vector3(m_position.X - 50, m_position.Y - 50, m_position.Z - 1.1f), Vector3(117, 153, 0), Vector3(30, 30, 0), "Media\\characters\\ninja_enemy_1\\leg_destroyed.png", true, 1.4f);
				GameObjectManager::Instance()->AddGameObject(leg2);

				ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, m_position, true, 2.0f);
				if (spray)
				{
					spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(leg2->ID()), Vector3(0, 0, 0));
				}
			}

			Debris * hat = new Debris(nullptr, Vector3(m_position.X, m_position.Y + 75, m_position.Z - 1.1f), Vector3(67, 29, 0), Vector3(30, 30, 0), "Media\\objects\\ricehat.png", false, 2.0f);
			GameObjectManager::Instance()->AddGameObject(hat);
		}
		else if (m_isAnimated && m_animationFile == "XmlFiles\\player_female_animation.xml")
		{
			{
				Debris * head = new Debris(nullptr, Vector3(m_position.X, m_position.Y + 250, m_position.Z - 1.1f), Vector3(54, 60, 0), Vector3(15, 15, 0), "Media\\characters\\female\\decapitated_head.png", true, 1.0f);
				GameObjectManager::Instance()->AddGameObject(head);

				ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, m_position, true, 2.0f);
				if (spray)
				{
					spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(head->ID()), Vector3(0, 0, 0));
				}
			}

			{
				Debris * arm1 = new Debris(nullptr, Vector3(m_position.X - 50, m_position.Y + 50, m_position.Z - 1.1f), Vector3(80, 80, 0), Vector3(30, 30, 0), "Media\\characters\\female\\arm_destroyed.png", true, 1.0f);
				GameObjectManager::Instance()->AddGameObject(arm1);

				ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, m_position, true, 2.0f);
				if (spray)
				{
					spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(arm1->ID()), Vector3(0, 0, 0));
				}
			}

			{
				Debris * arm2 = new Debris(nullptr, Vector3(m_position.X + 50, m_position.Y + 50, m_position.Z - 1.1f), Vector3(80, 80, 0), Vector3(30, 30, 0), "Media\\characters\\female\\arm_destroyed.png", true, 1.0f);
				GameObjectManager::Instance()->AddGameObject(arm2);

				ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, m_position, true, 2.0f);
				if (spray)
				{
					spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(arm2->ID()), Vector3(0, 0, 0));
				}
			}

			{
				Debris * leg1 = new Debris(nullptr, Vector3(m_position.X + 50, m_position.Y - 50, m_position.Z - 1.1f), Vector3(117, 153, 0), Vector3(30, 30, 0), "Media\\characters\\female\\leg_destroyed.png", true, 1.0f);
				GameObjectManager::Instance()->AddGameObject(leg1);

				ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, m_position, true, 2.0f);
				if (spray)
				{
					spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(leg1->ID()), Vector3(0, 0, 0));
				}
			}

			{
				Debris * leg2 = new Debris(nullptr, Vector3(m_position.X - 50, m_position.Y - 50, m_position.Z - 1.1f), Vector3(117, 153, 0), Vector3(30, 30, 0), "Media\\characters\\female\\leg_destroyed.png", true, 1.0f);
				GameObjectManager::Instance()->AddGameObject(leg2);

				ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, m_position, true, 2.0f);
				if (spray)
				{
					spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(leg2->ID()), Vector3(0, 0, 0));
				}
			}
		}
		*/

		// Meat chunks
		list<GameObject *> drawables;
		GameObjectManager::Instance()->GetTypesOnScreen<DrawableObject>(drawables);

		float orbCountMultiplier = 1.0f;
		if (drawables.size() < 200)
		{
			// don;t change anything
		}
		else if (drawables.size() < 250)
		{
			orbCountMultiplier = 0.5f;
		}
		else if (drawables.size() < 300)
		{
			orbCountMultiplier = 0.2f;
		}
		else
		{
			orbCountMultiplier = 0.0f;
		}

		for (int i = 0; i < (10 * orbCountMultiplier); ++i)
		{
			Vector3 orb_pos = m_position + pointOfContact;
			orb_pos.Z = 40;

			int randOrb = rand() % 3;
			switch (randOrb)
			{
				case 0:
					{
						Debris * orb = new Debris(nullptr, orb_pos, Vector3(40, 40, 0), Vector3(15, 15, 0), "Media\\orb.png", false, 1.0f);
						GameObjectManager::Instance()->AddGameObject(orb);

						ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, Vector3(orb_pos.X, orb_pos.Y, orb_pos.Z - 0.1), true, 2.0f);
						if (spray)
						{
							spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(orb->ID()), Vector3(0,0,0));
						}
						break;
					}
				case 1:
					{
						Debris * orb = new Debris(nullptr, orb_pos, Vector3(50, 50, 0), Vector3(25, 25, 0), "Media\\orb2.png", false, 1.0f);
						GameObjectManager::Instance()->AddGameObject(orb);

						ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, Vector3(orb_pos.X, orb_pos.Y, orb_pos.Z - 0.1), true, 2.0f);
						if (spray)
						{
							spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(orb->ID()), Vector3(0, 0, 0));
						}
						break;
					}
				case 2:
					{
						Debris * orb = new Debris(nullptr, orb_pos, Vector3(60, 60, 0), Vector3(30, 30, 0), "Media\\orb3.png", false, 1.0f);
						GameObjectManager::Instance()->AddGameObject(orb);

						ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, Vector3(orb_pos.X, orb_pos.Y, orb_pos.Z - 0.1), true, 2.0f);
						if (spray)
						{
							spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(orb->ID()), Vector3(0, 0, 0));
						}
						break;
					}
				default:
					{
						Debris * orb = new Debris(nullptr, orb_pos, Vector3(45, 45, 0), Vector3(15, 15, 0), "Media\\orb.png", false, 1.0f);
						GameObjectManager::Instance()->AddGameObject(orb);

						ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, Vector3(orb_pos.X, orb_pos.Y, orb_pos.Z - 0.1), true, 2.0f);
						if (spray)
						{
							spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(orb->ID()), Vector3(0, 0, 0));
						}

						break;
					}
			}
		}
	}
	else
	{
		// randomly cry
		int randNum = rand() % 100;

		if (randNum > 80)
		{
			int otherRandNum = rand() % 3;

			switch (otherRandNum)
			{
				case 0:
				{
					AudioManager::Instance()->PlaySoundEffect("character\\male_ninja\\damage_2\\1.wav");
					break;
				}
				case 1:
				{
					AudioManager::Instance()->PlaySoundEffect("character\\male_ninja\\damage_2\\2.wav");
					break;
				}
				case 2:
				{
					AudioManager::Instance()->PlaySoundEffect("character\\male_ninja\\damage_2\\3.wav");
					break;
				}
				default:
				{
					GAME_ASSERT(false);
					break;
				}
			};
		}
	}
}

Projectile * NPC::FireWeapon(Vector2 direction)
{
	Vector3 pos = m_position;
	pos.X = (direction.X > 0) ? pos.X + m_projectileOffset.X : pos.X -= m_projectileOffset.X;
	pos.Y += m_projectileOffset.Y;
	pos.Z -= 1;
	
	if (direction.X > 0)
	{
		pos.X += m_projectileOffset.X;
	}
	else
	{
		pos.X -= m_projectileOffset.X;
	}
	
	float speed = mSprintActive ? 26 : 21;

	// TODO: ideally want these properties configurable per character
	Projectile * p = new Projectile(Projectile::kNPCProjectile,
									mProjectileFilePath.c_str(),
									mProjectileImpactFilePath.c_str(), 
									pos,
									Vector2(20,20),
									Vector2(25,8),
									direction,
									2,
									speed,
									0.25f);

	p->SetSpinningMovement(true); // spinning for ninja stars 
	
	if (m_isAnimated && m_animation)
	{
		AnimationPart * armPart = m_animation->GetPart("arm");

		if (armPart)
		{
			armPart->Restart();
			armPart->Animate();
		}
	}

	if (direction.X > 0)
	{
		p->UnFlipVertical();
	}
	else
	{
		p->FlipVertical();
	}

	if (!WasInWaterLastFrame())
	{
		PlayRandomWeaponFireSound();
	}

	return p;
}

void NPC::Draw(ID3D10Device * device, Camera2D * camera)
{
	Character::Draw(device, camera);

	Vector3 healthBarPos = Vector3(m_position.X,
									m_position.Y + (m_collisionBoxDimensions.Y * 0.5f) + mCollisionBoxOffset.Y + 5 + mRandHealthBarOffsetY,
									m_position.Z);
	if (mHealthMeterHealthBeforeDecrease > mHealth && mHealthBarUnderlaySprite)
	{
		// TODO: set depth layer
		mHealthBarUnderlaySprite->SetXY(healthBarPos.X, healthBarPos.Y, healthBarPos.Z);

		if (mHealthBarUnderlaySprite->IsChangeRequired())
		{
			mHealthBarUnderlaySprite->ApplyChange(device);
		}
		mHealthBarUnderlaySprite->Draw(device, camera);
	}
	if (mHealthBarSprite)
	{
		float percentHealth = mHealth / mMaxHealth;

		float offsetX = kHealthBarDimensionsX - kHealthBarDimensionsX * percentHealth;
		offsetX *= 0.5f;

		// TODO: set depth layer
		mHealthBarSprite->SetXYZ(healthBarPos.X - offsetX, healthBarPos.Y, healthBarPos.Z);

		// apply any changes needed
		if (mHealthBarSprite->IsChangeRequired())
		{
			mHealthBarSprite->ApplyChange(device);
		}
		mHealthBarSprite->Draw(device, camera);
	}
	if (mHealthBarOverlaySprite)
	{
		// TODO: set depth layer
		mHealthBarOverlaySprite->SetXY(healthBarPos.X, healthBarPos.Y, healthBarPos.Z);

		// apply any changes needed
		if (mHealthBarOverlaySprite->IsChangeRequired())
		{
			mHealthBarOverlaySprite->ApplyChange(device);
		}
		mHealthBarOverlaySprite->Draw(device, camera);
	}
}

void NPC::AddHealthBar()
{
	mRandHealthBarOffsetY = rand() % 15;

	if (!mHealthBarUnderlaySprite)
	{
		mHealthBarUnderlaySprite = new Sprite();
		mHealthBarUnderlaySprite->SetTextureFilename("Media\\characters\\health_bar_back.png");
		mHealthBarUnderlaySprite->SetIsNativeDimensions(false);
		mHealthBarUnderlaySprite->SetDimensionsXY(kHealthBarDimensionsX, kHealthBarDimensionsY);
		mHealthBarUnderlaySprite->SetXY(X(), Y(), Z() - 1); // TODO: set depth layer
		mHealthBarUnderlaySprite->LoadContent(Graphics::GetInstance()->Device());
		mHealthBarUnderlaySprite->Initialise();

		mHealthBarUnderlaySprite->Scale(Graphics::GetInstance()->BackBufferWidth() / 1920,
			Graphics::GetInstance()->BackBufferHeight() / 1080,
			false);
	}
	if (!mHealthBarSprite)
	{
		mHealthBarSprite = new Sprite();
		mHealthBarSprite->SetTextureFilename("Media\\characters\\health_bar.png");
		mHealthBarSprite->SetIsNativeDimensions(false);
		mHealthBarSprite->SetDimensionsXY(kHealthBarDimensionsX, kHealthBarDimensionsY);
		mHealthBarSprite->SetXY(X(), Y(), Z() - 1); // TODO: set depth layer
		mHealthBarSprite->LoadContent(Graphics::GetInstance()->Device());
		mHealthBarSprite->Initialise();

		mHealthBarSprite->Scale(Graphics::GetInstance()->BackBufferWidth() / 1920,
			Graphics::GetInstance()->BackBufferHeight() / 1080,
			false);
	}
	if (!mHealthBarOverlaySprite)
	{
		mHealthBarOverlaySprite = new Sprite();
		mHealthBarOverlaySprite->SetTextureFilename("Media\\characters\\health_bar_overlay.png");
		mHealthBarOverlaySprite->SetIsNativeDimensions(false);
		mHealthBarOverlaySprite->SetDimensionsXY(kHealthBarOverlayDimensionsX, kHealthBarOverlayDimensionsY);
		mHealthBarOverlaySprite->SetXY(X(), Y(), Z() - 1); // TODO: set depth layer
		mHealthBarOverlaySprite->LoadContent(Graphics::GetInstance()->Device());
		mHealthBarOverlaySprite->Initialise();

		mHealthBarOverlaySprite->Scale(Graphics::GetInstance()->BackBufferWidth() / 1920,
									   Graphics::GetInstance()->BackBufferHeight() / 1080,
									   false);
	}
}

void NPC::DebugDraw(ID3D10Device *  device)
{
	Character::DebugDraw(device);

	if (mHealthBarUnderlaySprite)
	{
		mHealthBarUnderlaySprite->DebugDraw(device);
	}
	if (mHealthBarSprite)
	{
		mHealthBarSprite->DebugDraw(device);
	}
	if (mHealthBarOverlaySprite)
	{
		mHealthBarOverlaySprite->DebugDraw(device);
	}
}

void NPC::SetupDebugDraw()
{
	Character::SetupDebugDraw();

	if (mHealthBarUnderlaySprite)
	{
		mHealthBarUnderlaySprite->SetupDebugDraw();
	}
	if (mHealthBarSprite)
	{
		mHealthBarSprite->SetupDebugDraw();
	}
	if (mHealthBarOverlaySprite)
	{
		mHealthBarOverlaySprite->SetupDebugDraw();
	}
}

void NPC::UpdateHealthBar(float delta)
{
	if (!mHealthBarUnderlaySprite)
	{
		return;
	}

	if (!mHealthBarSprite)
	{
		return;
	}

	GAME_ASSERT(mMaxHealth > 0.0f);
	if (mMaxHealth <= 0.0f)
	{
		return;
	}

	float percentHealth = mHealth / mMaxHealth;

	mHealthBarSprite->SetMatrixScaleX(percentHealth * mCurrentHealthMeterScale);
	mHealthBarSprite->SetMatrixScaleY(mCurrentHealthMeterScale);

	if (mHealthMeterHealthBeforeDecrease > mHealth)
	{
		mHealthMeterHealthBeforeDecrease -= delta * 6.0f;
		if (mHealthMeterHealthBeforeDecrease < mHealth)
		{
			mHealthMeterHealthBeforeDecrease = mHealth;
		}
	}
}


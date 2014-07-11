#include "precompiled.h"
#include "NPC.h"
#include "AIStateFollow.h"
#include "AIStateRepel.h"
#include "AIStateRangeAttack.h"
#include "AudioManager.h"
#include "projectile.h"
#include "timing.h"
#include <math.h>
#include "orb.h"
#include "particleemittermanager.h"
#include "NPCManager.h"

static float kMinReloadTime = 0.8f;
static float kMaxReloadTime = 2.0f;

static const float kHealthBarDimensionsX = 73.0f;
static const float kHealthBarDimensionsY = 7.2f;
static const float kHealthBarOverlayDimensionsX = 75.0f;
static const float kHealthBarOverlayDimensionsY = 7.5f;

static const D3DXVECTOR2 kDefaultTex1 = D3DXVECTOR2(0, 0);
static const D3DXVECTOR2 kDefaultTex2 = D3DXVECTOR2(1, 0);
static const D3DXVECTOR2 kDefaultTex3 = D3DXVECTOR2(1, 1);
static const D3DXVECTOR2 kDefaultTex4 = D3DXVECTOR2(0, 1);

NPC::NPC(float x, float y, float z, float width, float height, float breadth) :
	Character(x, y, z, width, height, breadth),
	m_player(0),
	m_friendlyFollowState(nullptr),
	m_currentState(nullptr),
	mCheckNPCOverlapCollisions(true),
	mLastFireTime(0),
	mNextFireTime(0),
	m_repelState(nullptr),
	m_rangeAttackState(nullptr),
	mCurrentHealthMeterScale(1.0f)
{
	mHealth = 40.0f;
	mMaxHealth = 40.0f;
	mProjectileFilePath = "Media/ninjastar.png";
	mProjectileImpactFilePath = "Media/ninjastar_impact.png";

	NPCManager::Instance()->AddNPC(this);

	mLastFireTime = Timing::Instance()->GetTotalTimeSeconds();

	mNextFireTime = (rand() % ((int)((kMaxReloadTime - kMinReloadTime) * 100)) + kMinReloadTime * 100.0f);
	mNextFireTime *= 0.01f;

	// initialise the states
	m_friendlyFollowState = new AIStateFollow(this);
	m_repelState = new AIStateRepel(this);
	m_rangeAttackState = new AIStateRangeAttack(this);

	// set to default state
	SetState(AIState::kRangeAttack);
}

NPC::~NPC(void)
{
	NPCManager::Instance()->RemoveNPC(this);
}

void NPC::Update(float delta)
{
	Character::Update(delta);

	if (!m_player)
	{
		m_player = GameObjectManager::Instance()->GetPlayer();
	}

	if (m_currentState)
	{
		m_currentState->Update();
	}

	if (mHealthBarSprite)
	{
		mHealthBarSprite->Update(delta);
	}

	if (mHealthBarOverlaySprite)
	{
		mHealthBarOverlaySprite->Update(delta);
	}

	if (mCurrentHealthMeterScale != 1.0f)
	{
		mCurrentHealthMeterScale -= delta * 4.0f;
		if (mCurrentHealthMeterScale < 1.0f)
		{
			mCurrentHealthMeterScale = 1.0f;
		}

		UpdateHealthBar();
	}
}

void NPC::Initialise()
{
	Character::Initialise();

	AddHealthBar();

	UpdateHealthBar();
}

void NPC::XmlRead(TiXmlElement * element)
{
	Character::XmlRead(element);
}

void NPC::XmlWrite(TiXmlElement * element)
{
	Character::XmlWrite(element);
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
		float randYOffset = rand() % 100;

		unsigned randSign = rand() % 2;
		if (randSign == 1)
		{
			randYOffset *= -1;
		}

		Vector2 dir = Vector2(target->Position().X - m_position.X, (target->Position().Y + randYOffset) - m_position.Y);
		dir.Normalise();

		GameObjectManager::Instance()->AddGameObject(FireWeapon(dir));

		mLastFireTime = Timing::Instance()->GetTotalTimeSeconds();

		mNextFireTime = (rand() % ((int)((kMaxReloadTime - kMinReloadTime) * 100)) + kMinReloadTime * 100.0f);
		mNextFireTime *= 0.01f;
	}
}

void NPC::OnDamage(float damageAmount, Vector3 pointOfContact, bool shouldExplode)
{
	Character::OnDamage(damageAmount, pointOfContact, shouldExplode);

	mCurrentHealthMeterScale = 2.0f;

	UpdateHealthBar();

	// throw out some orbs
	int particleNUmPerOrb = 25;
	if (mHasExploded)
	{
		// decapitate
		if (m_isAnimated && m_animationFile == "XmlFiles\\ninjaAnimation2.xml")
		{

			Orb * head = new Orb(m_player, Vector3(m_position.X, m_position.Y + 250, m_position.Z - 1.1f), Vector3(54, 60, 0), Vector3(15, 15, 0), "Media\\characters\\ninja_enemy_1\\decapitated_head.png", true);
			GameObjectManager::Instance()->AddGameObject(head);

			Orb * arm1 = new Orb(m_player, Vector3(m_position.X - 50, m_position.Y + 50, m_position.Z - 1.1f), Vector3(80, 80, 0), Vector3(15, 15, 0), "Media\\characters\\ninja_enemy_1\\arm_destroyed.png", true);
			GameObjectManager::Instance()->AddGameObject(arm1);

			Orb * arm2 = new Orb(m_player, Vector3(m_position.X + 50, m_position.Y + 50, m_position.Z - 1.1f), Vector3(80, 80, 0), Vector3(15, 15, 0), "Media\\characters\\ninja_enemy_1\\arm_destroyed.png", true);
			GameObjectManager::Instance()->AddGameObject(arm2);

			Orb * leg1 = new Orb(m_player, Vector3(m_position.X + 50, m_position.Y - 50, m_position.Z - 1.1f), Vector3(117, 153, 0), Vector3(15, 15, 0), "Media\\characters\\ninja_enemy_1\\leg_destroyed.png", true);
			GameObjectManager::Instance()->AddGameObject(leg1);

			Orb * leg2 = new Orb(m_player, Vector3(m_position.X - 50, m_position.Y - 50, m_position.Z - 1.1f), Vector3(117, 153, 0), Vector3(15, 15, 0), "Media\\characters\\ninja_enemy_1\\leg_destroyed.png", true);
			GameObjectManager::Instance()->AddGameObject(leg2);
		}

		if (m_isAnimated && m_animationFile == "XmlFiles\\player_female_animation.xml")
		{

			Orb * head = new Orb(m_player, Vector3(m_position.X, m_position.Y + 250, m_position.Z - 1.1f), Vector3(54, 60, 0), Vector3(15, 15, 0), "Media\\characters\\female\\decapitated_head.png", true);
			GameObjectManager::Instance()->AddGameObject(head);

			Orb * arm1 = new Orb(m_player, Vector3(m_position.X - 50, m_position.Y + 50, m_position.Z - 1.1f), Vector3(80, 80, 0), Vector3(15, 15, 0), "Media\\characters\\female\\arm_destroyed.png", true);
			GameObjectManager::Instance()->AddGameObject(arm1);

			Orb * arm2 = new Orb(m_player, Vector3(m_position.X + 50, m_position.Y + 50, m_position.Z - 1.1f), Vector3(80, 80, 0), Vector3(15, 15, 0), "Media\\characters\\female\\arm_destroyed.png", true);
			GameObjectManager::Instance()->AddGameObject(arm2);

			Orb * leg1 = new Orb(m_player, Vector3(m_position.X + 50, m_position.Y - 50, m_position.Z - 1.1f), Vector3(117, 153, 0), Vector3(15, 15, 0), "Media\\characters\\female\\leg_destroyed.png", true);
			GameObjectManager::Instance()->AddGameObject(leg1);

			Orb * leg2 = new Orb(m_player, Vector3(m_position.X - 50, m_position.Y - 50, m_position.Z - 1.1f), Vector3(117, 153, 0), Vector3(15, 15, 0), "Media\\characters\\female\\leg_destroyed.png", true);
			GameObjectManager::Instance()->AddGameObject(leg2);
		}

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
						Orb * orb = new Orb(m_player, orb_pos, Vector3(40, 40, 0), Vector3(15, 15, 0), "Media\\orb.png", false);
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
						Orb * orb = new Orb(m_player, orb_pos , Vector3(50, 50, 0), Vector3(25, 25, 0), "Media\\orb2.png", false);
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
						Orb * orb = new Orb(m_player, orb_pos , Vector3(60, 60, 0), Vector3(30, 30, 0), "Media\\orb3.png", false);
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
						Orb * orb = new Orb(m_player, orb_pos , Vector3(45, 45, 0), Vector3(15, 15, 0), "Media\\orb.png", false);
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
	
	float speed = mSprintActive ? 45 : 35;

	// TODO: ideally want these properties configurable per character
	Projectile * p = new Projectile(Projectile::kNPCProjectile,
									mProjectileFilePath.c_str(),
									mProjectileImpactFilePath.c_str(), 
									pos,
									Vector2(20,20),
									Vector2(30 * 1.2,8),
									direction,
									5,
									speed,
									2.5f);

	p->SetSpinningMovement(true); // spinning for ninja stars 
	
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
		p->FlipVertical();
	}

	PlayRandomWeaponFireSound();

	return p;
}

void NPC::Draw(ID3D10Device * device, Camera2D * camera)
{
	Character::Draw(device, camera);

	if (mHealthBarSprite)
	{
		// apply any changes needed
		if (mHealthBarSprite->IsChangeRequired())
		{
			mHealthBarSprite->ApplyChange(device);
		}
		mHealthBarSprite->Draw(device, camera);
	}
	if (mHealthBarOverlaySprite)
	{
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
	if (!mHealthBarSprite)
	{
		mHealthBarSprite = unique_ptr<Sprite>(new Sprite());
		mHealthBarSprite->SetTextureFilename("Media\\characters\\health_bar.png");
		mHealthBarSprite->SetIsNativeDimensions(false);
		mHealthBarSprite->SetDimensionsXYZ(kHealthBarDimensionsX, kHealthBarDimensionsY, 0);
		mHealthBarSprite->SetXYZ(X(), Y(), Z() - 1);
		mHealthBarSprite->LoadContent(Graphics::GetInstance()->Device());
		mHealthBarSprite->Initialise();

		mHealthBarSprite->Scale(Graphics::GetInstance()->BackBufferWidth() / 1920,
			Graphics::GetInstance()->BackBufferHeight() / 1080,
			false);

		mHealthBarSprite->AttachTo(GameObjectManager::Instance()->GetObjectByID(ID()), 
									Vector3(0,((m_collisionBoxDimensions.Y * 0.5f) + mCollisionBoxOffset.Y) + 20, 0),
									false);
	}
	if (!mHealthBarOverlaySprite)
	{
		mHealthBarOverlaySprite = unique_ptr<Sprite>(new Sprite());
		mHealthBarOverlaySprite->SetTextureFilename("Media\\characters\\health_bar_overlay.png");
		mHealthBarOverlaySprite->SetIsNativeDimensions(false);
		mHealthBarOverlaySprite->SetDimensionsXYZ(kHealthBarOverlayDimensionsX, kHealthBarOverlayDimensionsY, 0);
		mHealthBarOverlaySprite->SetXYZ(X(), Y(), Z() - 1);
		mHealthBarOverlaySprite->LoadContent(Graphics::GetInstance()->Device());
		mHealthBarOverlaySprite->Initialise();

		mHealthBarOverlaySprite->Scale(Graphics::GetInstance()->BackBufferWidth() / 1920,
									   Graphics::GetInstance()->BackBufferHeight() / 1080,
									   false);

		mHealthBarOverlaySprite->AttachTo(GameObjectManager::Instance()->GetObjectByID(ID()),
											Vector3(0, ((m_collisionBoxDimensions.Y * 0.5f) + mCollisionBoxOffset.Y) + 20, 0),
											false);
	}
}

void NPC::DebugDraw(ID3D10Device *  device)
{
	Character::DebugDraw(device);

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

	if (mHealthBarSprite)
	{
		mHealthBarSprite->SetupDebugDraw();
	}
	if (mHealthBarOverlaySprite)
	{
		mHealthBarOverlaySprite->SetupDebugDraw();
	}
}

void NPC::UpdateHealthBar()
{
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
	mHealthBarSprite->SetAttachmentOffsetX(-((((kHealthBarDimensionsX - (kHealthBarDimensionsX * percentHealth))) * mCurrentHealthMeterScale) * 0.5f) );

	if (mHealthBarOverlaySprite)
	{
		mHealthBarOverlaySprite->SetMatrixScaleX(mCurrentHealthMeterScale);
		mHealthBarOverlaySprite->SetMatrixScaleY(mCurrentHealthMeterScale);
	}
}

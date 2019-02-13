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

static float kMinReloadTime = 1.25f;
static float kMaxReloadTime = 2.0f;
static float kDamageComboWindowDelay = 0.5f;
static int kDamageInARowToStun = 6;
static float kTimeStunned = 2.0f;

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
	if (mHasExploded && mExplodesGruesomely)
	{
		DoNinjaExplosion();

		auto player = GameObjectManager::Instance()->GetPlayer();

		float playerHealth = player != nullptr ? player->GetHealth() : 0.0f;

		if (playerHealth < 30.0f)
		{
			// if health is low then there is an n% chance to drop a health pickup
			int randChance = rand() % 100;

			if (randChance < 26)
			{
				GameObjectManager::Instance()->SpawnHealthIncrease(m_position);
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
	Vector2 pos = m_position;
	pos.X = (direction.X > 0.0f) ? pos.X + m_projectileOffset.X : pos.X -= m_projectileOffset.X;
	pos.Y += m_projectileOffset.Y;
	
	if (direction.X > 0.0f)
	{
		pos.X += m_projectileOffset.X;
	}
	else
	{
		pos.X -= m_projectileOffset.X;
	}
	
	float speed = mSprintActive ? 15.0f : 10.0f;

	// TODO: ideally want these properties configurable per character
	Projectile * p = new Projectile(Projectile::kNPCProjectile,
									mProjectileFilePath.c_str(),
									mProjectileImpactFilePath.c_str(), 
									pos,
									GameObject::kNinjaStarProjectile, //TODO: this won't always be a ninja star
									Vector2(20.0f,20.0f),
									Vector2(25.0f,8.0f),
									direction,
									2.0f,
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

	Vector2 healthBarPos = Vector2(m_position.X,
									m_position.Y + (m_collisionBoxDimensions.Y * 0.5f) + mCollisionBoxOffset.Y + 5 + mRandHealthBarOffsetY);
	if (mHealthMeterHealthBeforeDecrease > mHealth && mHealthBarUnderlaySprite)
	{
		mHealthBarUnderlaySprite->SetXY(healthBarPos.X, healthBarPos.Y);
		mHealthBarUnderlaySprite->SetDepthLayer(GameObject::kNpc);

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

		mHealthBarSprite->SetXY(healthBarPos.X - offsetX, healthBarPos.Y);
		mHealthBarSprite->SetDepthLayer(GameObject::kNpc);

		// apply any changes needed
		if (mHealthBarSprite->IsChangeRequired())
		{
			mHealthBarSprite->ApplyChange(device);
		}
		mHealthBarSprite->Draw(device, camera);
	}
	if (mHealthBarOverlaySprite)
	{
		mHealthBarOverlaySprite->SetXY(healthBarPos.X, healthBarPos.Y);
		mHealthBarOverlaySprite->SetDepthLayer(GameObject::kNpc);

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
		mHealthBarUnderlaySprite->SetXY(X(), Y());
		mHealthBarUnderlaySprite->SetDepthLayer(GameObject::kNpc);
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
		mHealthBarSprite->SetXY(X(), Y());
		mHealthBarSprite->SetDepthLayer(GameObject::kNpc);
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
		mHealthBarOverlaySprite->SetXY(X(), Y());
		mHealthBarOverlaySprite->SetDepthLayer(GameObject::kNpc);
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

void NPC::DoNinjaExplosion()
{
	int numParticlesPerLimb = 30;

	if (m_isAnimated && m_animationFile == "XmlFiles\\animation\\ninjaAnimation.xml")
	{
		SpawnLimb("Media\\characters\\enemy_1\\exploded\\ricehat.png", false, 1.3f);
		SpawnLimb("Media\\characters\\enemy_1\\exploded\\head.png", true, 0.8f);
		SpawnLimb("Media\\characters\\enemy_1\\exploded\\forearm_back.png", true, 0.95f);
		SpawnLimb("Media\\characters\\enemy_1\\exploded\\forearm_back.png", true, 1.1f);
		SpawnLimb("Media\\characters\\enemy_1\\exploded\\hand_front.png", true, 0.65f);
		SpawnLimb("Media\\orb.png", true, 1.25f);
		SpawnLimb("Media\\orb2.png", true, 1.20f);
		SpawnLimb("Media\\orb2.png", true, 0.94f);
		SpawnLimb("Media\\orb2.png", true, 0.79f);
		SpawnLimb("Media\\orb3.png", true, 1.12f);
	}
}

void NPC::SpawnLimb(const string & filename, bool attachParticles, float speedMultiplier)
{
	Debris * limb = new Debris(nullptr,
								Vector2(m_position.X, m_position.Y + 50),
								GetDepthLayer(),
								Vector2(80, 80),
								Vector2(30, 30),
								filename.c_str(),
								true,
								speedMultiplier);

	limb->SetGravityApplyAmount(0.25f);
	limb->SetBouncable(true);
	limb->SetBounceDampening(0.55f);
	limb->SetResistanceXY(0.99f, 0.95f);
	limb->SetCollidesWithOtherDebris(false);

	GameObjectManager::Instance()->AddGameObject(limb);

	if (attachParticles)
	{
		ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(10, m_position, true, 3.0f);
		if (spray)
		{
			spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(limb->ID()), Vector2(0, 0), GetDepthLayer());
		}
	}
}


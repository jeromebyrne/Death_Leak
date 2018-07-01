#include "precompiled.h"
#include "butterfly.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "material.h"
#include "Debris.h"
#include "AIStateFollow.h"
#include "AIStateRepel.h"

Butterfly::Butterfly(void) :
	NPC()
{
	mIsButterfly = true;
	mHealth = 1.0f; // 1 hit kills for rabbits
	mMatchAnimFrameRateWithMovement = false;
	mAddHealthBar = false;
}


Butterfly::~Butterfly(void)
{
}

void Butterfly::OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode)
{
	if (GetDepthLayer() != GameObject::kGround)
	{
		// hack to stop butterfly's in the background being affected
		return;
	}
	m_alpha = 0.0f;
	mHasExploded = true;

	GameObjectManager::Instance()->RemoveGameObject(this);
	
	Debris * wing1 = new Debris(nullptr, Vector2(m_position.X, m_position.Y), GetDepthLayer(),  Vector2(32.0f, 32.0f), Vector2(15.0f, 15.0f), "Media\\characters\\butterfly\\dead_wing_1.png", true, 0.15f);
	GameObjectManager::Instance()->AddGameObject(wing1);

	Debris * wing2 = new Debris(nullptr, Vector2(m_position.X + 10, m_position.Y), GetDepthLayer(), Vector2(32.0f, 32.0f), Vector2(15.0f, 15.0f), "Media\\characters\\butterfly\\dead_wing_2.png", true, 0.15f);
	GameObjectManager::Instance()->AddGameObject(wing2);
}

void Butterfly::Initialise()
{
	NPC::Initialise();

	mCheckNPCOverlapCollisions = false;

	SetState(AIState::kButterflyWander);

	m_applyGravity = true;
	m_resistance.X = 0.99f;
	m_resistance.Y = 6.0f;
	m_maxVelocity.X = 0.5f;
	m_maxVelocity.Y = 5.0f;
	mAccelXRate = 0.5f;

	if (GetDepthLayer() > GameObject::kGround)
	{
		// hack to stop butterfly's in the background being affected
		m_passive = true;
	}
}

void Butterfly::UpdateAnimations()
{
	AnimationPart * bodyPart = m_animation->GetPart("body");
	GAME_ASSERT(bodyPart);

	if (bodyPart != nullptr)
	{
		if (bodyPart->CurrentSequence()->Name() != "Still")
		{
			bodyPart->SetSequence("Still");
		}

		if (GetVelocity().Y < -2.0f)
		{
			float animFramerate = std::abs((GetVelocity().Y * 4.0f));

			bodyPart->CurrentSequence()->SetFrameRate(animFramerate);
		}
		else
		{
			float animFramerate = 45;

			bodyPart->CurrentSequence()->SetFrameRate(animFramerate);
		}
		

		bodyPart->AnimateLooped();

		m_texture = bodyPart->CurrentFrame(); // set the current texture
	}

	m_mainBodyTexture = m_texture;
}

bool Butterfly::OnCollision(SolidMovingSprite * object)
{
	if (GetDepthLayer() > GameObject::kGround)
	{
		// hack to stop butterfly's in the background being affected
		return false;
	}

	return NPC::OnCollision(object);
}
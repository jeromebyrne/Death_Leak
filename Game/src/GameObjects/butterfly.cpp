#include "precompiled.h"
#include "butterfly.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "material.h"
#include "Orb.h"
#include "AIStateFollow.h"
#include "AIStateRepel.h"

Butterfly::Butterfly(void) :
	NPC()
{
	mIsButterfly = true;
	mHealth = 1; // 1 hit kills for rabbits
	mMatchAnimFrameRateWithMovement = false;
	mAddHealthBar = false;
}


Butterfly::~Butterfly(void)
{
}

void Butterfly::OnDamage(float damageAmount, Vector3 pointOfContact, bool shouldExplode)
{
	if (m_position.Z > 99)
	{
		// hack to stop butterfly's in the background being affected
		return;
	}
	m_alpha = 0.0f;
	mHasExploded = true;

	GameObjectManager::Instance()->RemoveGameObject(this);

	Orb * wing1 = new Orb(nullptr, Vector3(m_position.X, m_position.Y, m_position.Z - 1.1f), Vector3(32, 32, 0), Vector3(15, 15, 0), "Media\\characters\\butterfly\\dead_wing_1.png", true, 0.15f);
	GameObjectManager::Instance()->AddGameObject(wing1);

	Orb * wing2 = new Orb(nullptr, Vector3(m_position.X + 10, m_position.Y, m_position.Z - 1.1f), Vector3(32, 32, 0), Vector3(15, 15, 0), "Media\\characters\\butterfly\\dead_wing_2.png", true, 0.15f);
	GameObjectManager::Instance()->AddGameObject(wing2);
}

void Butterfly::Initialise()
{
	NPC::Initialise();

	mCheckNPCOverlapCollisions = false;

	mPlayFootsteps = false;

	SetState(AIState::kButterflyWander);

	m_applyGravity = true;
	m_resistance.X = 0.99f;
	m_resistance.Y = 6.0f;
	m_maxVelocity.X = 0.5f;
	m_maxVelocity.Y = 5.0f;
	mAccelXRate = 0.5f;

	if (m_position.Z > 99)
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

void Butterfly::OnCollision(SolidMovingSprite * object)
{
	if (m_position.Z > 99)
	{
		// hack to stop butterfly's in the background being affected
		return;
	}

	NPC::OnCollision(object);
}
#include "precompiled.h"
#include "Rabbit.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "material.h"
#include "Debris.h"
#include "AIStateFollow.h"
#include "AIStateRepel.h"

Rabbit::Rabbit(void) :
	NPC()
{
	mHealth = 0.01f; // 1 hit kills for rabbits/rats
	mAddHealthBar = false;
}

Rabbit::~Rabbit(void)
{
}

void Rabbit::OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode)
{
	Character::OnDamage(damageDealer, damageAmount, pointOfContact, false);

	int particleNUmPerOrb = 25;
	if (mHasExploded)
	{
		list<GameObject *> drawables;
		GameObjectManager::Instance()->GetTypesOnScreen<DrawableObject>(drawables);

		float orbCountMultiplier = 1.0f;
		if (drawables.size() < 200)
		{
			// don't change anything
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
			Vector2 orb_pos = m_position + pointOfContact;

			int randOrb = rand() % 3;
			switch (randOrb)
			{
				case 0:
					{
						Debris * orb = new Debris(nullptr, orb_pos, GetDepthLayer(), Vector2(20.0f, 20.0f), Vector2(15.0f, 15.0f), "Media\\orb.png", false, 0.5f);
						GameObjectManager::Instance()->AddGameObject(orb);

						/*
						ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, Vector2(orb_pos.X, orb_pos.Y), true, 2.0f);
						if (spray)
						{
							spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(orb->ID()), Vector2(0.0f, 0.0f), GetDepthLayer());
						}
						*/
						break;
					}
				case 1:
					{
						Debris * orb = new Debris(nullptr, orb_pos, GetDepthLayer(), Vector2(25.0f, 25.0f), Vector2(25.0f, 25.0f), "Media\\orb2.png", false, 0.5f);
						GameObjectManager::Instance()->AddGameObject(orb);

						/*
						ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, Vector2(orb_pos.X, orb_pos.Y), true, 2.0f);
						if (spray)
						{
							spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(orb->ID()), Vector2(0.0f, 0.0f), GetDepthLayer());
						}
						*/
						break;
					}
				case 2:
					{
						Debris * orb = new Debris(nullptr, orb_pos, GetDepthLayer(), Vector2(30.0f, 30.0f), Vector2(30.0f, 30.0f), "Media\\orb3.png", false, 0.5f);
						GameObjectManager::Instance()->AddGameObject(orb);

						/*
						ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, Vector2(orb_pos.X, orb_pos.Y), true, 2.0f);
						if (spray)
						{
							spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(orb->ID()), Vector2(0.0f, 0.0f), GetDepthLayer());
						}
						*/
						break;
					}
				default:
					{
						Debris * orb = new Debris(nullptr, orb_pos, GetDepthLayer(), Vector2(23.0f, 23.0f), Vector2(15.0f, 15.0f), "Media\\orb.png", false, 0.5f);
						GameObjectManager::Instance()->AddGameObject(orb);

						/*
						ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, Vector2(orb_pos.X, orb_pos.Y), true, 2.0f);
						if (spray)
						{
							spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(orb->ID()), Vector2(0.0f, 0.0f), GetDepthLayer());
						}
						*/

						break;
					}
			}
		}
	}
}

void Rabbit::Initialise()
{
	NPC::Initialise();

	int randStopDistance = rand() % 100;

	m_friendlyFollowState->SetStopDistance(120 + randStopDistance);
	m_friendlyFollowState->SetFollowRadius(500);
	m_repelState->SetStopDistance(120 + randStopDistance);
	m_repelState->SetFollowRadius(500);

	mRunAnimFramerateMultiplier = 1.8f;

	mCheckNPCOverlapCollisions = false;

	SetState(AIState::kGroundAnimalWander);
}
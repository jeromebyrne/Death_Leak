#include "precompiled.h"
#include "Rabbit.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "material.h"
#include "Orb.h"
#include "AIStateFollow.h"

Rabbit::Rabbit(void) :
	NPC()
{
	mHealth = 1; // 1 hit kills for rabbits
}


Rabbit::~Rabbit(void)
{
}

void Rabbit::OnDamage(float damageAmount, Vector3 pointOfContact, bool shouldExplode)
{
	Character::OnDamage(damageAmount, pointOfContact, false);

	int particleNUmPerOrb = 25;
	if (mHasExploded)
	{
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
						Orb * orb = new Orb(m_player, orb_pos, Vector3(20, 20, 0), Vector3(15, 15, 0), "Media\\orb.png", false);
						GameObjectManager::Instance()->AddGameObject(orb);

						ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, Vector3(orb_pos.X, orb_pos.Y, orb_pos.Z - 0.1), true, 2.0f);
						if (spray)
						{
							spray->AttachTo(GameObjectManager::Instance()->GetObjectByID(orb->ID()), Vector3(0, 0, 0));
						}
						break;
					}
				case 1:
					{
						Orb * orb = new Orb(m_player, orb_pos , Vector3(25, 25, 0), Vector3(25, 25, 0), "Media\\orb2.png", false);
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
						Orb * orb = new Orb(m_player, orb_pos , Vector3(30, 30, 0), Vector3(30, 30, 0), "Media\\orb3.png", false);
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
						Orb * orb = new Orb(m_player, orb_pos , Vector3(23, 23, 0), Vector3(15, 15, 0), "Media\\orb.png", false);
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

void Rabbit::Initialise()
{
	NPC::Initialise();

	int randStopDistance = rand() % 100;

	m_friendlyFollowState->SetStopDistance(120 + randStopDistance);
	m_friendlyFollowState->SetFollowRadius(500);

	mRunAnimFramerateMultiplier = 3.5f;

	mCheckNPCOverlapCollisions = false;

	mPlayFootsteps = false;
}
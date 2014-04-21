#include "precompiled.h"
#include "NPC.h"
#include "AIStateFollow.h"
#include "AudioManager.h"
#include "projectile.h"
#include "timing.h"
#include <math.h>
#include "orb.h"
#include "particleemittermanager.h"
#include "NPCManager.h"

NPC::NPC(float x, float y, float z, float width, float height, float breadth):
Character(x, y, z, width, height, breadth),
m_player(0),
m_friendlyFollowState(nullptr),
m_currentState(nullptr),
mCheckNPCOverlapCollisions(true)
{
	mHealth = 30.0f;
	mProjectileFilePath = "Media/ninjastar.png";
	mProjectileImpactFilePath = "Media/ninjastar_impact.png";

	NPCManager::Instance()->AddNPC(this);
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
}

void NPC::Initialise()
{
	Character::Initialise();

	// initialise the states
	m_friendlyFollowState = new AIStateFollow(this);

	// initialise the current state
	SetState(AIState::kFriendlyFollowing);
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
				m_currentState = m_friendlyFollowState;
				m_currentState->OnTransition();
				break;
			}
		default:
			{
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

	// just using this static for testing, fire time should be determined by weapon type (not implemented)
	static float last_fire_time = 0.0f;

	if (last_fire_time + 0.3f < Timing::Instance()->GetTotalTimeSeconds())
	{
		Vector2 dir = Vector2(target->Position().X - m_position.X, target->Position().Y - m_position.Y);
		dir.Normalise();

		GameObjectManager::Instance()->AddDrawableObject_RunTime(FireWeapon(dir));

		last_fire_time = Timing::Instance()->GetTotalTimeSeconds();
	}
}

void NPC::OnDamage(float damageAmount, Vector3 pointOfContact, bool shouldExplode)
{
	Character::OnDamage(damageAmount, pointOfContact, shouldExplode);

	// throw out some orbs
	int particleNUmPerOrb = 25;
	if (mHasExploded)
	{
		// decapitate
		if (m_isAnimated && m_animationFile == "XmlFiles\\ninjaAnimation2.xml")
		{

			Orb * head = new Orb(m_player, Vector3(m_position.X, m_position.Y + 250, m_position.Z - 1.1f), Vector3(54, 60, 0), Vector3(15, 15, 0), "Media\\characters\\ninja_enemy_1\\decapitated_head.png", true);
			GameObjectManager::Instance()->AddDrawableObject_RunTime(head);

			Orb * arm1 = new Orb(m_player, Vector3(m_position.X - 50, m_position.Y + 50, m_position.Z - 1.1f), Vector3(80, 80, 0), Vector3(15, 15, 0), "Media\\characters\\ninja_enemy_1\\arm_destroyed.png", true);
			GameObjectManager::Instance()->AddDrawableObject_RunTime(arm1);

			Orb * arm2 = new Orb(m_player, Vector3(m_position.X + 50, m_position.Y + 50, m_position.Z - 1.1f), Vector3(80, 80, 0), Vector3(15, 15, 0), "Media\\characters\\ninja_enemy_1\\arm_destroyed.png", true);
			GameObjectManager::Instance()->AddDrawableObject_RunTime(arm2);

			Orb * leg1 = new Orb(m_player, Vector3(m_position.X + 50, m_position.Y - 50, m_position.Z - 1.1f), Vector3(117, 153, 0), Vector3(15, 15, 0), "Media\\characters\\ninja_enemy_1\\leg_destroyed.png", true);
			GameObjectManager::Instance()->AddDrawableObject_RunTime(leg1);

			Orb * leg2 = new Orb(m_player, Vector3(m_position.X - 50, m_position.Y - 50, m_position.Z - 1.1f), Vector3(117, 153, 0), Vector3(15, 15, 0), "Media\\characters\\ninja_enemy_1\\leg_destroyed.png", true);
			GameObjectManager::Instance()->AddDrawableObject_RunTime(leg2);
		}

		if (m_isAnimated && m_animationFile == "XmlFiles\\player_female_animation.xml")
		{

			Orb * head = new Orb(m_player, Vector3(m_position.X, m_position.Y + 250, m_position.Z - 1.1f), Vector3(54, 60, 0), Vector3(15, 15, 0), "Media\\characters\\female\\decapitated_head.png", true);
			GameObjectManager::Instance()->AddDrawableObject_RunTime(head);

			Orb * arm1 = new Orb(m_player, Vector3(m_position.X - 50, m_position.Y + 50, m_position.Z - 1.1f), Vector3(80, 80, 0), Vector3(15, 15, 0), "Media\\characters\\female\\arm_destroyed.png", true);
			GameObjectManager::Instance()->AddDrawableObject_RunTime(arm1);

			Orb * arm2 = new Orb(m_player, Vector3(m_position.X + 50, m_position.Y + 50, m_position.Z - 1.1f), Vector3(80, 80, 0), Vector3(15, 15, 0), "Media\\characters\\female\\arm_destroyed.png", true);
			GameObjectManager::Instance()->AddDrawableObject_RunTime(arm2);

			Orb * leg1 = new Orb(m_player, Vector3(m_position.X + 50, m_position.Y - 50, m_position.Z - 1.1f), Vector3(117, 153, 0), Vector3(15, 15, 0), "Media\\characters\\female\\leg_destroyed.png", true);
			GameObjectManager::Instance()->AddDrawableObject_RunTime(leg1);

			Orb * leg2 = new Orb(m_player, Vector3(m_position.X - 50, m_position.Y - 50, m_position.Z - 1.1f), Vector3(117, 153, 0), Vector3(15, 15, 0), "Media\\characters\\female\\leg_destroyed.png", true);
			GameObjectManager::Instance()->AddDrawableObject_RunTime(leg2);
		}

		// Meat chunks
		list<DrawableObject*> drawables;
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
						GameObjectManager::Instance()->AddDrawableObject_RunTime(orb);

						ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, Vector3(orb_pos.X, orb_pos.Y, orb_pos.Z - 0.1), true, 2.0f);
						if (spray)
						{
							spray->AttachToSprite(orb, Vector2(0,0));
						}
						break;
					}
				case 1:
					{
						Orb * orb = new Orb(m_player, orb_pos , Vector3(50, 50, 0), Vector3(25, 25, 0), "Media\\orb2.png", false);
						GameObjectManager::Instance()->AddDrawableObject_RunTime(orb);

						ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, Vector3(orb_pos.X, orb_pos.Y, orb_pos.Z - 0.1), true, 2.0f);
						if (spray)
						{
							spray->AttachToSprite(orb, Vector2(0,0));
						}
						break;
					}
				case 2:
					{
						Orb * orb = new Orb(m_player, orb_pos , Vector3(60, 60, 0), Vector3(30, 30, 0), "Media\\orb3.png", false);
						GameObjectManager::Instance()->AddDrawableObject_RunTime(orb);

						ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, Vector3(orb_pos.X, orb_pos.Y, orb_pos.Z - 0.1), true, 2.0f);
						if (spray)
						{
							spray->AttachToSprite(orb, Vector2(0,0));
						}
						break;
					}
				default:
					{
						Orb * orb = new Orb(m_player, orb_pos , Vector3(45, 45, 0), Vector3(15, 15, 0), "Media\\orb.png", false);
						GameObjectManager::Instance()->AddDrawableObject_RunTime(orb);

						ParticleSpray * spray = ParticleEmitterManager::Instance()->CreateRadialBloodSpray(particleNUmPerOrb, Vector3(orb_pos.X, orb_pos.Y, orb_pos.Z - 0.1), true, 2.0f);
						if (spray)
						{
							spray->AttachToSprite(orb, Vector2(0,0));
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
	Projectile * p = new Projectile(mProjectileFilePath.c_str(), mProjectileImpactFilePath.c_str(), this, pos, Vector2(20,20), Vector2(30 * 1.2,8), direction, 5, speed, 2.5f);
	p->SetSpinningMovement(true); // spinning for ninja stars 

	mActiveProjectiles.insert(p);
	
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

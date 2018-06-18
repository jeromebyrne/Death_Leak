#include "precompiled.h"
#include "GhostEnemy.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "material.h"
#include "Debris.h"
#include "AIStateFollow.h"
#include "AIStateRepel.h"
#include "Game.h"
#include "Projectile.h"

GhostEnemy::GhostEnemy(void) :
	NPC()
{
	mProjectileFilePath = "Media/puke_temp.png";
	mProjectileImpactFilePath = "Media/puke_impact_temp.png";
	mExplodesGruesomely = false;
	mHealth = 10.0f;
	mEmitsBlood = false;
}


GhostEnemy::~GhostEnemy(void)
{
}

void GhostEnemy::OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode)
{
	Character::OnDamage(damageDealer, damageAmount, pointOfContact, shouldExplode);

	// randomly cry
	int randNum = rand() % 100;

	if (randNum > 80)
	{
		int otherRandNum = rand() % 3;

		switch (otherRandNum)
		{
		case 0:
		{
			AudioManager::Instance()->PlaySoundEffect("character\\scream.wav");
			break;
		}
		case 1:
		{
			break;
		}
		case 2:
		{
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


bool GhostEnemy::OnCollision(SolidMovingSprite * object)
{
	return false;
}


void GhostEnemy::Initialise()
{
	NPC::Initialise();

	mCheckNPCOverlapCollisions = false;

	m_applyGravity = false;

	m_maxVelocity.X = 2.5f + ((rand()%80) * 0.1f);
	m_maxVelocity.Y = 3.5f; 

	m_passive = true; // This is so ghosts pass through stuff

	mProjectileSpeed = 4.5f + ((rand() % 30) * 0.1f);

	/*
	if (m_position.Z > 99)
	{
		// hack to stop butterfly's in the background being affected
		m_passive = true;
	}
	*/

	SetState(AIState::kFlying); // let's just handle our own state
}

/*
void GhostEnemy::UpdateAnimations()
{
	AnimationPart * bodyPart = m_animation->GetPart("body");
	GAME_ASSERT(bodyPart);

	if (bodyPart != nullptr)
	{
		if (mIsGliding)
		{
			if (bodyPart->CurrentSequence()->Name() != "Still")
			{
				bodyPart->SetSequence("Still");

				bodyPart->CurrentSequence()->SetFrameRate(1);
			}
		}
		else
		{
			if (bodyPart->CurrentSequence()->Name() != "Running")
			{
				bodyPart->SetSequence("Running");

				bodyPart->CurrentSequence()->SetFrameRate(9);
			}
		}
		
		bodyPart->AnimateLooped();

		m_texture = bodyPart->CurrentFrame(); // set the current texture
	}

	m_mainBodyTexture = m_texture;
}
*/

/*
bool GhostEnemy::OnCollision(SolidMovingSprite * object)
{
	if (m_position.Z > 99)
	{
		// hack to stop butterfly's in the background being affected
		return false;
	}

	return NPC::OnCollision(object);
}
*/

void GhostEnemy::Update(float delta)
{
	NPC::Update(delta);

	if (Game::GetIsLevelEditMode())
	{
		return;
	}
}

void GhostEnemy::XmlRead(TiXmlElement * element)
{
	NPC::XmlRead(element);

	// mTravelOffset = XmlUtilities::ReadAttributeAsFloat(element, "travel_offset", "value");
}

void GhostEnemy::XmlWrite(TiXmlElement * element)
{
	NPC::XmlWrite(element);

	/*
	TiXmlElement * travelOffset = new TiXmlElement("travel_offset");
	travelOffset->SetAttribute("value", Utilities::ConvertDoubleToString(mTravelOffset).c_str());
	element->LinkEndChild(travelOffset);
	*/
}

Projectile * GhostEnemy::FireWeapon(Vector2 direction)
{
	Vector2 pos = m_position;
	pos.X = (direction.X > 0) ? pos.X + m_projectileOffset.X : pos.X -= m_projectileOffset.X;
	pos.Y += m_projectileOffset.Y;

	if (direction.X > 0)
	{
		pos.X += m_projectileOffset.X;
	}
	else
	{
		pos.X -= m_projectileOffset.X;
	}

	// TODO: ideally want these properties configurable per character
	Projectile * p = new Projectile(Projectile::kNPCProjectile,
		mProjectileFilePath.c_str(),
		mProjectileImpactFilePath.c_str(),
		pos,
		GameObject::kGhostVomitProjectile,
		Vector2(20.0f, 20.0f),
		Vector2(25.0f, 8.0f),
		direction,
		2.0f,
		mProjectileSpeed,
		2.0f);

	p->SetSpinningMovement(false);
	p->SetProjectileResistance(0.9f);
	p->SetIsDeflectable(false);

	if (direction.X > 0.0f)
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
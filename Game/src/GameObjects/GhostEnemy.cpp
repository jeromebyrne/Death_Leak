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
}


GhostEnemy::~GhostEnemy(void)
{
}

/*
void GhostEnemy::OnDamage(GameObject * damageDealer, float damageAmount, Vector3 pointOfContact, bool shouldExplode)
{
	// TODO:
}
*/

void GhostEnemy::Initialise()
{
	NPC::Initialise();

	mCheckNPCOverlapCollisions = false;

	m_applyGravity = false;

	m_maxVelocity.X = 7.0f;
	m_maxVelocity.Y = 3.5f; 

	m_passive = true; // This is so ghosts pass through stuff

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

	float speed = 0.0f;

	// TODO: ideally want these properties configurable per character
	Projectile * p = new Projectile(Projectile::kNPCProjectile,
		mProjectileFilePath.c_str(),
		mProjectileImpactFilePath.c_str(),
		pos,
		Vector2(20, 20),
		Vector2(25, 8),
		direction,
		2,
		speed,
		2.0f);

	p->SetSpinningMovement(false);
	p->SetProjectileResistance(0.4f); // load gravity puke
	p->SetIsDeflectable(false);

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
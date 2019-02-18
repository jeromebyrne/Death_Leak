#include "precompiled.h"
#include "SkeletonEnemy.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "material.h"
#include "Debris.h"
#include "AIStateFollow.h"
#include "AIStateRepel.h"
#include "Game.h"
#include "Projectile.h"

SkeletonEnemy::SkeletonEnemy(void) :
	NPC()
{
	mProjectileFilePath = "Media/puke_temp.png";
	mProjectileImpactFilePath = "Media/puke_impact_temp.png";
	mExplodesGruesomely = false;
	mHealth = 10.0f;
	mEmitsBlood = false;
}

SkeletonEnemy::~SkeletonEnemy(void)
{
}

void SkeletonEnemy::OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode)
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


bool SkeletonEnemy::OnCollision(SolidMovingSprite * object)
{
	return false;
}


void SkeletonEnemy::Initialise()
{
	NPC::Initialise();

	mCheckNPCOverlapCollisions = true;

	m_applyGravity = true;

	m_maxVelocity.X = 2.5f + ((rand()%80) * 0.1f);
	m_maxVelocity.Y = 3.5f; 

	m_passive = false; // This is so ghosts pass through stuff

	mProjectileSpeed = 4.5f + ((rand() % 30) * 0.1f);

	SetState(AIState::kRangeAttack);
}

void SkeletonEnemy::Update(float delta)
{
	NPC::Update(delta);

	if (Game::GetIsLevelEditMode())
	{
		return;
	}
}

void SkeletonEnemy::XmlRead(TiXmlElement * element)
{
	NPC::XmlRead(element);

	// mTravelOffset = XmlUtilities::ReadAttributeAsFloat(element, "travel_offset", "value");
}

void SkeletonEnemy::XmlWrite(TiXmlElement * element)
{
	NPC::XmlWrite(element);

	/*
	TiXmlElement * travelOffset = new TiXmlElement("travel_offset");
	travelOffset->SetAttribute("value", Utilities::ConvertDoubleToString(mTravelOffset).c_str());
	element->LinkEndChild(travelOffset);
	*/
}

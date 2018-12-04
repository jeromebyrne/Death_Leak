#include "precompiled.h"
#include "HealthIncreasePickup.h"
#include "AudioManager.h"
#include "ParticleEmitterManager.h"

void HealthIncreasePickup::DoPickup()
{
	AudioManager::Instance()->PlaySoundEffect("character/drink_health_upgrade.wav");

	Player * p = GameObjectManager::Instance()->GetPlayer();

	if (p == nullptr)
	{
		return;
	}

	p->IncreaseHealth(mHealthIncreaseAmount);

	DoPickupEffects(p);

	GameObjectManager::Instance()->RemoveGameObject(this);
}

void HealthIncreasePickup::DoPickupEffects(Player * player)
{
	// TODO:
}


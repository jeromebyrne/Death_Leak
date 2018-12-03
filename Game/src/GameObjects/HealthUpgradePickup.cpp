#include "precompiled.h"
#include "HealthUpgradePickup.h"
#include "AudioManager.h"
#include "ParticleEmitterManager.h"

void HealthUpgradePickup::DoPickup()
{
	AudioManager::Instance()->PlaySoundEffect("character/drink_health_upgrade.wav");

	Player * p = GameObjectManager::Instance()->GetPlayer();

	if (p == nullptr)
	{
		return;
	}

	int currentMaxHealth = p->GetMaxHealth();

	int newMaxHealth = currentMaxHealth + mHealthUpgradeAmount;

	p->SetMaxHealth(newMaxHealth);

	DoPickupEffects(p);

	GameObjectManager::Instance()->RemoveGameObject(this);
}

void HealthUpgradePickup::DoPickupEffects(Player * player)
{
	Camera2D::GetInstance()->DoBigShake();

	ParticleEmitterManager::Instance()->CreateDirectedSpray(1,
		player->Position(),
		player->GetDepthLayer(),
		Vector2(0.0f, 0.0f),
		0.1f,
		Vector2(3200.0f, 1200.0f),
		"Media\\explosion_lines.png",
		1.0f,
		1.0f,
		0.1f,
		0.1f,
		256.0f,
		256.0f,
		0.0f,
		false,
		1.0f,
		1.0f,
		0.0f,
		true,
		12.0f,
		0.0f,
		0.0f,
		0.05f,
		0.1f,
		true);
}

void HealthUpgradePickup::Initialise()
{
	Pickup::Initialise();
}


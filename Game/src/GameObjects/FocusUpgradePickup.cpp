#include "precompiled.h"
#include "FocusUpgradePickup.h"
#include "AudioManager.h"
#include "ParticleEmitterManager.h"

void FocusUpgradePickup::DoPickup()
{
	AudioManager::Instance()->PlaySoundEffect("character/drink_health_upgrade.wav");

	Player * p = GameObjectManager::Instance()->GetPlayer();

	if (p == nullptr)
	{
		return;
	}

	int currentMaxFocus = p->GetMaxFocusAmount();

	int newMaxFocus = currentMaxFocus == 0 ? mFirstFocusUpgradeAmount : currentMaxFocus + mFocusUpgradeAmount;

	p->SetMaxFocus(newMaxFocus);

	DoPickupEffects(p);

	GameObjectManager::Instance()->RemoveGameObject(this);
}

void FocusUpgradePickup::DoPickupEffects(Player * player)
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

void FocusUpgradePickup::Initialise()
{
	Pickup::Initialise();
}
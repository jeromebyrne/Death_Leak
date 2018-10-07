#include "precompiled.h"
#include "KeyPickup.h"
#include "AudioManager.h"
#include "ParticleEmitterManager.h"
#include "SaveManager.h"

void KeyPickup::DoPickup()
{
	AudioManager::Instance()->PlaySoundEffect("character/drink_health_upgrade.wav");

	Player * p = GameObjectManager::Instance()->GetPlayer();

	if (p == nullptr)
	{
		return;
	}

	if (!mKeyId.empty())
	{
		SaveManager::GetInstance()->SetHasDoorkey(mKeyId, true);
	}

	DoPickupEffects(p);

	GameObjectManager::Instance()->RemoveGameObject(this);
}

void KeyPickup::DoPickupEffects(Player * player)
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
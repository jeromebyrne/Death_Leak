#include "precompiled.h"
#include "HealthUpgradePickup.h"
#include "AudioManager.h"
#include "ParticleEmitterManager.h"
#include "TextObject.h"

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


	/*
	{
		TextObject * levelUpText = new TextObject(m_position.X, m_position.Y, GetDepthLayer(), 3.0f);

		levelUpText->SetFont("Jing Jing");
		levelUpText->SetFontColor(1.0f, 0.0f, 0.0f);
		levelUpText->SetFontSize(50.0f);
		levelUpText->SetStringKey("health_upgrade_notification");
		levelUpText->SetNoClip(true);

		levelUpText->SetDimensionsXY(200.0f, 250.0f);

		GameObjectManager::Instance()->AddGameObject(levelUpText);

		levelUpText->AttachTo(GameObjectManager::Instance()->GetObjectByID(player->ID()), Vector2(0.0f, 125.0f), GameObject::kPlayer, false);
	}
	*/
}
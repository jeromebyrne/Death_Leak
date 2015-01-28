#include "precompiled.h"
#include "NPCTrigger.h"
#include "Player.h"
#include "NinjaSpawner.h"
#include "ParticleEmitterManager.h"

NPCTrigger::NPCTrigger() :
	GameObject(),
	mCooldownTime(7.0f),
	mCurrentCooldownTime(0.0f)
{

}

void NPCTrigger::Update(float delta) 
{
	GameObject::Update(delta);

	if (mCurrentCooldownTime > 0.0f)
	{
		mCurrentCooldownTime -= delta;
		return;
	}
	mCurrentCooldownTime = 0.0f;

	Player * player = GameObjectManager::Instance()->GetPlayer();
	GAME_ASSERT(player);

	if (!player)
	{
		return;
	}

	bool playerCollision = Utilities::IsSolidSpriteInRectangle(player, m_position.X, m_position.Y, m_dimensions.X, m_dimensions.Y);

	if (playerCollision)
	{
		NinjaSpawner ninjaSpawner;
		ninjaSpawner.SpawnMultiple(5, Vector2(player->X(), player->Y()), Vector2(1200, 1200));
		Timing::Instance()->SetTimeModifierForNumSeconds(0.1f, 3.5f);
		ParticleEmitterManager::Instance()->CreateDirectedSpray(1,
																Vector3(Camera2D::GetInstance()->X(), Camera2D::GetInstance()->Y(), 2),
																Vector3(0, 1, 0),
																0.1,
																Vector3(3200, 1200, 0),
																"Media\\exclamation.png",
																1.0f,
																1.0f,
																0.3f,
																0.3f,
																273.5f,
																273.5f,
																0.0f,
																false,
																1.0f,
																1.0f,
																0.0f,
																true,
																2.0f,
																0.0f,
																0.0f,
																0.05f,
																0.6f,
																true);

		mCurrentCooldownTime = mCooldownTime;
	}
}
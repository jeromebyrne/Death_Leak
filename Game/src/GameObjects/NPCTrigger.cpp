#include "precompiled.h"
#include "NPCTrigger.h"
#include "Player.h"
#include "NinjaSpawner.h"
#include "ParticleEmitterManager.h"

NPCTrigger::NPCTrigger() :
	GameObject(),
	mCooldownTime(7.0f),
	mCurrentCooldownTime(0.0f),
	mNumEnemies(0)

{
	mAlwaysUpdate = true;
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
		ninjaSpawner.SpawnMultiple(mNumEnemies, Vector2(player->X(), player->Y()), Vector2(1200, 1200));
		Timing::Instance()->SetTimeModifierForNumSeconds(0.1f, 3.5f);
		ParticleEmitterManager::Instance()->CreateDirectedSpray(1,
																Vector2(Camera2D::GetInstance()->X(), Camera2D::GetInstance()->Y()),
																GameObject::kNpc,
																Vector2(0.0f, 1.0f),
																0.1f,
																Vector2(3200.0f, 1200.0f),
																"Media\\skull_icon_2.png",
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

void NPCTrigger::XmlRead(TiXmlElement * element) 
{
	GameObject::XmlRead(element);

	mCooldownTime = XmlUtilities::ReadAttributeAsFloat(element, "cooldown_time", "value");
	mNumEnemies = XmlUtilities::ReadAttributeAsFloat(element, "num_enemies", "value");
}

void NPCTrigger::XmlWrite(TiXmlElement * element) 
{
	GameObject::XmlWrite(element);

	TiXmlElement * cooldownElem = new TiXmlElement("cooldown_time");
	cooldownElem->SetDoubleAttribute("value", mCooldownTime);
	element->LinkEndChild(cooldownElem);

	TiXmlElement * numEnemiesElem = new TiXmlElement("num_enemies");
	numEnemiesElem->SetDoubleAttribute("value", mNumEnemies);
	element->LinkEndChild(numEnemiesElem);
}
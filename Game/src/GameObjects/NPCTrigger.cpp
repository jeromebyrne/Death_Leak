#include "precompiled.h"
#include "NPCTrigger.h"
#include "Player.h"
#include "NinjaSpawner.h"
#include "ParticleEmitterManager.h"
#include "Game.h"
#include "DrawUtilities.h"
#include "GhostEnemySpawner.h"
#include "SkeletonEnemySpawner.h"
#include "SaveManager.h"

static const float kCooldownTime = 120000; 

NPCTrigger::NPCTrigger() :
	GameObject(),
	mNumEnemies(0)

{
	mAlwaysUpdate = true;
}

void NPCTrigger::Initialise()
{
	GameObject::Initialise();

	string levelFilename = GameObjectManager::Instance()->GetCurrentLevelFile();

	double lastTimeSpawned = SaveManager::GetInstance()->GetLastTimeNPCSpawnerTriggered(levelFilename, ID());

	double currentTime = timeGetTime();

	double timeDiff = currentTime - lastTimeSpawned;

	if (timeDiff < kCooldownTime)
	{
		RemoveTriggerFromLevel();
	}
}

void NPCTrigger::RemoveTriggerFromLevel()
{
	if (Game::GetInstance()->GetIsLevelEditMode())
	{
		return;
	}

	GameObjectManager::Instance()->RemoveGameObject(this);

	mIsRemoved = true;
}

void NPCTrigger::Update(float delta) 
{
	if (mIsRemoved)
	{
		return;
	}

	GameObject::Update(delta);

#if _DEBUG
	if (Game::GetInstance()->GetIsLevelEditMode())
	{
		return;
	}
#endif

	Player * player = GameObjectManager::Instance()->GetPlayer();
	GAME_ASSERT(player);

	if (!player)
	{
		return;
	}

	bool playerCollision = Utilities::IsSolidSpriteInRectangle(player, m_position.X, m_position.Y, m_dimensions.X, m_dimensions.Y);

	if (playerCollision)
	{
		SpawnEnemies(player);

		RecordLastSpawnTime();

		RemoveTriggerFromLevel();
	}
}

void NPCTrigger::SpawnEnemies(Player * player)
{
	switch (mType)
	{
		case kNinja:
		{
			SpawnNinjas(player);
			break;
		}
		case kGhost:
		{
			SpawnGhosts(player);
			break;
		}
		case kSkeleton:
		{
			SpawnSkeletons(player);
			break;
		}
	}
}

void NPCTrigger::RecordLastSpawnTime()
{
	string levelFile = GameObjectManager::Instance()->GetCurrentLevelFile();
	SaveManager::GetInstance()->SetLastTimeNPCSpawnerTriggered(levelFile, ID(), (double)timeGetTime());
}

void NPCTrigger::SpawnNinjas(Player * player)
{
	NinjaSpawner ninjaSpawner;
	ninjaSpawner.SpawnMultiple(mNumEnemies, Vector2(player->X(), player->Y()), Vector2(1200, 1200));

	// Timing::Instance()->SetTimeModifierForNumSeconds(0.1f, 3.5f);
	ShowSpawnIcon();
}

void NPCTrigger::SpawnGhosts(Player * player)
{
	GhostEnemySpawner spawner;
	spawner.SpawnMultiple(mNumEnemies, Vector2(player->X(), player->Y()), Vector2(1200, 1200));
	// Timing::Instance()->SetTimeModifierForNumSeconds(0.1f, 3.5f);
	ShowSpawnIcon();
}

void NPCTrigger::SpawnSkeletons(Player * player)
{
	SkeletonEnemySpawner spawner;
	spawner.SpawnMultiple(mNumEnemies, Vector2(player->X(), player->Y()), Vector2(1200, 1200));
	// Timing::Instance()->SetTimeModifierForNumSeconds(0.1f, 3.5f);
	ShowSpawnIcon();
}

void NPCTrigger::ShowSpawnIcon()
{
	ParticleEmitterManager::Instance()->CreateDirectedSpray(1,
															Vector2(Camera2D::GetInstance()->X(), Camera2D::GetInstance()->Y()),
															GameObject::kFarForeground,
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
}

void NPCTrigger::XmlRead(TiXmlElement * element) 
{
	GameObject::XmlRead(element);

	mNumEnemies = XmlUtilities::ReadAttributeAsFloat(element, "num_enemies", "value");
	string enemyTypeStr = XmlUtilities::ReadAttributeAsString(element, "num_enemies", "enemy_type");
	mType = GetNPCTypeFromString(enemyTypeStr);
}

void NPCTrigger::XmlWrite(TiXmlElement * element) 
{
	GameObject::XmlWrite(element);

	TiXmlElement * numEnemiesElem = new TiXmlElement("num_enemies");
	numEnemiesElem->SetDoubleAttribute("value", mNumEnemies);
	numEnemiesElem->SetAttribute("enemy_type", GetNPCStringFromType(mType).c_str());
	element->LinkEndChild(numEnemiesElem);
}

NPCTrigger::NpcType NPCTrigger::GetNPCTypeFromString(const string & typeAsString)
{
	if (typeAsString == "kNinja")
	{
		return kNinja;
	}
	else if (typeAsString == "kGhost")
	{
		return kGhost;
	}
	else if (typeAsString == "kSkeleton")
	{
		return kSkeleton;
	}

	GAME_ASSERT(false);
	return kUnknown;
}

string NPCTrigger::GetNPCStringFromType(NpcType type)
{
	switch (type)
	{
		case kNinja:
		{
			return "kNinja";
		}
		case kGhost:
		{
			return "kGhost";
		}
		case kSkeleton:
		{
			return "kSkeleton";
		}
		default:
		{
			GAME_ASSERT(false);
			return "kUnknown";
		}
	}

	GAME_ASSERT(false);
	return "kUnkown";
}

void NPCTrigger::DebugDraw(ID3D10Device *  device)
{
	GameObject::DebugDraw(device);

	DrawUtilities::DrawTexture(Vector3(m_position.X, m_position.Y, 3),
								GetLevelEditSelectionDimensions(),
								"Media\\editor\\skull_icon.png");
}
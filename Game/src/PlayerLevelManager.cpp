#include "precompiled.h"
#include "PlayerLevelManager.h"

PlayerLevelManager * PlayerLevelManager::m_instance = nullptr;

PlayerLevelManager * PlayerLevelManager::GetInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new PlayerLevelManager();
	}

	return m_instance;
}
PlayerLevelManager::PlayerLevelManager(void) :
	mPlayerCurrentLevel(1)
{
}

PlayerLevelManager::~PlayerLevelManager(void)
{
}

void PlayerLevelManager::Initialise()
{
	CreateLevelUpData();
}

void PlayerLevelManager::CreateLevelUpData()
{
	mPlayerLevelData.clear();

	// This ideally needs to be read from a config file but just doing this for now
	unsigned int maxLevel = 5;
	unsigned unitsRequiredToFirstLevelUp = 500;
	float levelRampMultiplier = 2.0f;
	for (unsigned int i = 1; i <= maxLevel; ++i)
	{
		PlayerLevelInfo info;
		info.CanLevelUp = (i != maxLevel);
		info.UnitsRequiredToLevelUp = i == 1 ? unitsRequiredToFirstLevelUp :  unitsRequiredToFirstLevelUp * ((i * i) * levelRampMultiplier);
		
		mPlayerLevelData[i] = info;
	}
}

float PlayerLevelManager::GetPercentTowardsLevelUp(unsigned int currentLevel, unsigned int currentXPUnits) const
{
	if (currentLevel > mPlayerLevelData.size())
	{
		GAME_ASSERT(false);
		return 0.0f;
	}

	PlayerLevelInfo info = mPlayerLevelData.at(currentLevel);
	if (!info.CanLevelUp)
	{
		// level up cap
		return 0.99f;
	}

	int needed = info.UnitsRequiredToLevelUp - currentXPUnits;
	if (needed < 1)
	{
		return 1.0f;
	}

	if (info.UnitsRequiredToLevelUp < 1)
	{
		GAME_ASSERT(false);
		return 0.0f;
	}

	return (float)currentXPUnits / (float)info.UnitsRequiredToLevelUp;
}

bool PlayerLevelManager::ShouldLevelUp(unsigned int currentLevel, unsigned int currentXPUnits) const
{
	if (currentLevel > mPlayerLevelData.size())
	{
		GAME_ASSERT(false);
		return false;
	}

	PlayerLevelInfo info = mPlayerLevelData.at(currentLevel);
	if (!info.CanLevelUp)
	{
		// level up cap
		return false;
	}

	int needed = info.UnitsRequiredToLevelUp - currentXPUnits;
	if (needed < 1)
	{
		return true;
	}

	return false;
}
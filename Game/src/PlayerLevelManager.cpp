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
PlayerLevelManager::PlayerLevelManager(void)
{
}

PlayerLevelManager::~PlayerLevelManager(void)
{
}
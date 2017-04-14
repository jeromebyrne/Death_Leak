#include "precompiled.h"
#include "InventoryManager.h"

InventoryManager * InventoryManager::m_instance = nullptr;

InventoryManager * InventoryManager::GetInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new InventoryManager();
	}

	return m_instance;
}
InventoryManager::InventoryManager(void)
{
}

InventoryManager::~InventoryManager(void)
{
}

void InventoryManager::Initialise()
{
	// TODO: read keys from save file
}

bool InventoryManager::HasKey(const std::string & keyId)
{
	// TODO:
	return false;
}

void InventoryManager::SetHasKey(const std::string & keyId, bool value)
{
	// TODO:
}
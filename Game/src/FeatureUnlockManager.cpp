#include "precompiled.h"
#include "FeatureUnlockManager.h"

FeatureUnlockManager * FeatureUnlockManager::m_instance = nullptr;

FeatureUnlockManager * FeatureUnlockManager::GetInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new FeatureUnlockManager();
	}

	return m_instance;
}
FeatureUnlockManager::FeatureUnlockManager(void)
{
}

FeatureUnlockManager::~FeatureUnlockManager(void)
{
}

void FeatureUnlockManager::Initialise()
{
	// check the svae file for features unlocked
	// then populate the cache

	// TODO
}

bool FeatureUnlockManager::IsFeatureUnlocked(const FeatureType type)
{
	if (mFeatureUnlockCache.find(type) == mFeatureUnlockCache.end())
	{
		return false;
	}

	return mFeatureUnlockCache[type];
}

void FeatureUnlockManager::SetFeatureUnlocked(const FeatureType type, bool unlocked)
{
	mFeatureUnlockCache[type] = unlocked;
}
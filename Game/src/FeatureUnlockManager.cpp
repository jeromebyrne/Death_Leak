#include "precompiled.h"
#include "FeatureUnlockManager.h"
#include "SaveManager.h"

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
}

bool FeatureUnlockManager::IsFeatureUnlocked(const FeatureType type)
{
#ifdef _DEBUG
	// testing
	// return true;
#endif

	return SaveManager::GetInstance()->IsGameFeatureUnlocked(type);
}

void FeatureUnlockManager::SetFeatureUnlocked(const FeatureType type)
{
	SaveManager::GetInstance()->SetGameFeatureUnlocked(type);
}

FeatureUnlockManager::FeatureType FeatureUnlockManager::GetFeatureTypeFromString(const string & asString)
{
	if (asString == "kDoubleJump")
	{
		return kDoubleJump;
	}
	else if (asString == "kCrouchJump")
	{
		return kCrouchJump;
	}
	else if (asString == "kDownwardDash")
	{
		return kDownwardDash;
	}
	else if (asString == "kRoll")
	{
		return kRoll;
	}
	else if (asString == "kSlowMotion")
	{
		return kSlowMotion;
	}
	else if (asString == "kDeflection")
	{
		return kDeflection;
	}
	
	GAME_ASSERT(false);
	return kNone;
}

string FeatureUnlockManager::GetFeatureAsString(FeatureType featureType)
{
	switch (featureType)
	{
		case kDoubleJump:
		{
			return "kDoubleJump";
		}
		case kCrouchJump:
		{
			return "kCrouchJump";
		}
		case kDownwardDash:
		{
			return "kDownwardDash";
		}
		case kRoll:
		{
			return "kRoll";
		}
		case kSlowMotion:
		{
			return "kSlowMotion";
		}
		case kDeflection:
		{
			return "kDeflection";
		}
	}

	return "kNone";
}
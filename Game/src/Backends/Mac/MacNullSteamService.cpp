#include "Backends/Mac/MacNullSteamService.h"

#include <algorithm>
#include <iostream>

bool MacNullSteamService::Initialise(int appId)
{
    mAppId = appId;
    return true;
}

void MacNullSteamService::Update()
{
}

bool MacNullSteamService::IsSteamDeck() const
{
    return false;
}

void MacNullSteamService::UnlockAchievement(const std::string& achievementId)
{
    if (std::find(mUnlockedAchievements.begin(), mUnlockedAchievements.end(), achievementId) != mUnlockedAchievements.end())
    {
        return;
    }

    mUnlockedAchievements.push_back(achievementId);
    std::cout << "steam stub achievement: " << achievementId << "\n";
}

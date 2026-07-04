#ifndef DEATHLEAK_BACKENDS_MAC_MACNULLSTEAMSERVICE_H
#define DEATHLEAK_BACKENDS_MAC_MACNULLSTEAMSERVICE_H

#include "Engine/Services/ISteamService.h"

#include <string>
#include <vector>

class MacNullSteamService final : public ISteamService
{
public:
    bool Initialise(int appId) override;
    void Update() override;
    bool IsSteamDeck() const override;
    void UnlockAchievement(const std::string& achievementId) override;

    int AppId() const { return mAppId; }
    const std::vector<std::string>& UnlockedAchievements() const { return mUnlockedAchievements; }

private:
    int mAppId = 0;
    std::vector<std::string> mUnlockedAchievements;
};

#endif

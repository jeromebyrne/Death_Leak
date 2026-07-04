#ifndef DEATHLEAK_ENGINE_SERVICES_ISTEAMSERVICE_H
#define DEATHLEAK_ENGINE_SERVICES_ISTEAMSERVICE_H

#include <string>

class ISteamService
{
public:
    virtual ~ISteamService() = default;

    virtual bool Initialise(int appId) = 0;
    virtual void Update() = 0;
    virtual bool IsSteamDeck() const = 0;
    virtual void UnlockAchievement(const std::string& achievementId) = 0;
};

#endif

#ifndef UIGAMEHUDSCREEN_H
#define UIGAMEHUDSCREEN_H

#include "uiscreen.h"

class UIMeter;

class UIGameHudScreen : public UIScreen
{

public:
	UIGameHudScreen(string name);
	virtual ~UIGameHudScreen(void);

	virtual void Update();

	virtual void Initialise();

private:

	void DoHealthMeterUpgrade(Player * player);

	void UpdatePlayerHealthMeter(Player * player);

	void UpdatePlayerStaminaMeter(Player * player);
	UIMeter * mPlayerHealthMeter;
	UIMeter * mPlayerXPMeter;
	float mLastPlayerHealth = 0.0f;

	bool mDoingHealthMeterUpgrade = false;

	ISound * mMeterUpgradeSound = nullptr;
};

#endif

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

	void DoFocusMeterUpgrade(Player * player);
	void UpdatePlayerFocusMeter(Player * player);

	UIMeter * mPlayerHealthMeter;
	UIMeter * mPlayerXPMeter;
	float mLastPlayerMaxHealth = 0.0f;
	float mLastPlayerMaxFocus = 0.0f;

	bool mDoingHealthMeterUpgrade = false;
	bool mDoingFocusMeterUpgrade = false;

	ISound * mMeterUpgradeSound = nullptr;
};

#endif

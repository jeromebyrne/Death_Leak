#ifndef UIGAMEHUDSCREEN_H
#define UIGAMEHUDSCREEN_H

#include "uiscreen.h"

class UIMeter;
class SaveManager;

class UIGameHudScreen : public UIScreen
{

public:
	UIGameHudScreen(string name);
	virtual ~UIGameHudScreen(void);

	virtual void Update() override;

	virtual void Draw(ID3D10Device * device) override;

	virtual void Initialise() override;

private:

	void DoHealthMeterUpgrade(Player * player);
	void UpdatePlayerHealthMeter(Player * player);

	void DoFocusMeterUpgrade(Player * player);
	void UpdatePlayerFocusMeter(Player * player);

	void InitialiseText();

	UIMeter * mPlayerHealthMeter;
	UIMeter * mPlayerXPMeter;
	float mLastPlayerMaxHealth = 0.0f;
	float mLastPlayerMaxFocus = 0.0f;

	bool mDoingHealthMeterUpgrade = false;
	bool mDoingFocusMeterUpgrade = false;

	ISound * mMeterUpgradeSound = nullptr;

	ID3DX10Font* mOrbCountText = nullptr;

	SaveManager * mSaveManagerCached = nullptr;

	int mLastOrbCount = -1;
	string mOrbtext;
};

#endif

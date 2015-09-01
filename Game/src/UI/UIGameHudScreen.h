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

	UIMeter * mPlayerHealthMeter;
	UIMeter * mPlayerXPMeter;
};

#endif

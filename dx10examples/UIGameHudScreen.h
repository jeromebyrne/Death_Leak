#ifndef UIGAMEHUDSCREEN_H
#define UIGAMEHUDSCREEN_H

#include "uiscreen.h"

class UIMeter;

class UIGameHudScreen : public UIScreen
{
private:

	UIMeter * mPlayerHealthMeter;

public:
	UIGameHudScreen(string name);
	virtual ~UIGameHudScreen(void);

	virtual void Update();

	virtual void Initialise();
};

#endif

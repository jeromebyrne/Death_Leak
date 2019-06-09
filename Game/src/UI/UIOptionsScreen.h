#ifndef UIOPTIONSSCREEN_H
#define UIOPTIONSSCREEN_H

#include "uiscreen.h"

class UICheckbox;

class UIOptionsScreen : public UIScreen
{

public:
	UIOptionsScreen(string name);
	virtual ~UIOptionsScreen(void);

	virtual void Initialise() override;

private:

	UICheckbox * mSfxCheckbox = nullptr;
	UICheckbox * mMusicCheckbox = nullptr;
	
};

#endif

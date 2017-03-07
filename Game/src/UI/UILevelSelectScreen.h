#ifndef UILEVELSELECTSCREEN_H
#define UILEVELSELECTSCREEN_H

#include "uiscreen.h"

class UILevelSelectScreen : public UIScreen
{
public:

	UILevelSelectScreen(string name);
	virtual ~UILevelSelectScreen(void);

	virtual void Update() override;

	virtual void Initialise() override;

	virtual void LoadContent(ID3D10Device * graphicsdevice) override;

private:

	void CreateLevelEntries();

	void AddLevelButtons();

	map<string, string> mLevelEntries;
};

#endif

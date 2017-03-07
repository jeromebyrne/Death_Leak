#ifndef UIQUICKPLAYSCREEN_H
#define UIQUICKPLAYSCREEN_H

#include "uiscreen.h"

class UIQuickPlayScreen : public UIScreen
{
public:

	UIQuickPlayScreen(string name);
	virtual ~UIQuickPlayScreen(void);

	virtual void Update() override;

	virtual void Initialise() override;

	virtual void LoadContent(ID3D10Device * graphicsdevice) override;

private:

	void CreateLevelEntries();

	void AddLevelButtons();

	map<string, string> mLevelEntries;
};

#endif

#ifndef UIMAINMENUSCREEN_H
#define UIMAINMENUSCREEN_H

#include "uiscreen.h"

class UIMainMenuScreen : public UIScreen
{

public:
	UIMainMenuScreen(string name);
	virtual ~UIMainMenuScreen(void);

	virtual void Update() override;

	virtual void Draw(ID3D10Device * device) override;

	virtual void Initialise() override;

private:

	void UpdateGamepadWarningWidget();

	void UpdateDebugWidgets();

	UIWidget* m_GamePadWarningWidget = nullptr;
	UIWidget* m_LevelEditorButton = nullptr;
	UIWidget* m_LevelSelectButton = nullptr;
};

#endif

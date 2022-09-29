#include "precompiled.h"
#include "UIMainMenuScreen.h"
#include "UIMeter.h"
#include "GameObjectManager.h"
#include "SaveManager.h"
#include "PlayerLevelManager.h"
#include "AudioManager.h"
#include "Game.h"
#include "GamePad.h"

extern void PostDestroyMessage();

UIMainMenuScreen::UIMainMenuScreen(string name) :
	UIScreen(name)
{
}

UIMainMenuScreen::~UIMainMenuScreen(void)
{
}

void UIMainMenuScreen::Update()
{
	UIScreen::Update();

	UpdateGamepadWarningWidget();

	// if pressing escap on teh main menu then quit to desktop
	if (GetAsyncKeyState(VK_ESCAPE) < 0)
	{
		PostDestroyMessage();
	}
}

void UIMainMenuScreen::Initialise()
{
	UIScreen::Initialise();

	m_GamePadWarningWidget = m_widgetMap["8_gamepad_warning"];

	if (m_widgetMap.find("8_1_level_editor") != m_widgetMap.end())
	{
		m_LevelEditorButton = m_widgetMap["8_1_level_editor"];
	}

	if (m_widgetMap.find("9_quick_play") != m_widgetMap.end())
	{
		m_LevelSelectButton = m_widgetMap["9_quick_play"];
	}

	UpdateDebugWidgets();
}

void UIMainMenuScreen::UpdateDebugWidgets()
{
	if (m_LevelEditorButton)
	{
#ifdef _DEBUG
		m_LevelEditorButton->SetVisible(true);
		m_LevelEditorButton->SetIsProcessInput(true);
#else
		m_LevelEditorButton->SetVisible(false);
		m_LevelEditorButton->SetIsProcessInput(false);
#endif
	}

	if (m_LevelSelectButton)
	{
#ifdef _DEBUG
		m_LevelSelectButton->SetVisible(true);
#else
		m_LevelSelectButton->SetVisible(false);
#endif
	}
}

void UIMainMenuScreen::UpdateGamepadWarningWidget()
{
	if (m_GamePadWarningWidget == nullptr)
	{
		return;
	}

	m_GamePadWarningWidget->SetVisible(!GamePad::GetPad1() || !GamePad::GetPad1()->IsConnected());
}

void UIMainMenuScreen::Draw(ID3D10Device * device)
{
	UIScreen::Draw(device);
}


#include "precompiled.h"
#include "UIQuickPlayScreen.h"
#include "GameObjectManager.h"
#include "UIButton.h"

static const unsigned kButtonIndexStart = 9000;

UIQuickPlayScreen::UIQuickPlayScreen(string name) :
	UIScreen(name)
{
}

UIQuickPlayScreen::~UIQuickPlayScreen(void)
{
}

void UIQuickPlayScreen::Update()
{
	UIScreen::Update();
}

void UIQuickPlayScreen::Initialise()
{
	UIScreen::Initialise();

	CreateLevelEntries();

	AddLevelButtons();
}

void UIQuickPlayScreen::CreateLevelEntries()
{
	mLevelEntries["Test Level"] = "XmlFiles\\levels\\test_level.xml";
	mLevelEntries["Underwater Cave"] = "XmlFiles\\underwater_cave.xml";
	mLevelEntries["Village 2"] = "XmlFiles\\village_2.xml";
	mLevelEntries["Waterfall level"] = "XmlFiles\\waterfall_level.xml";
	mLevelEntries["Village Outskirts"] = "XmlFiles\\village_outskirts.xml";
	mLevelEntries["Field"] = "XmlFiles\\field_1.xml";
	mLevelEntries["Village"] = "XmlFiles\\village.xml";
	mLevelEntries["Save Shrine Test"] = "XmlFiles\\levels\\save_shrine_test_level.xml";
	mLevelEntries["Door Test 1"] = "XmlFiles\\levels\\door_test_1.xml";
	mLevelEntries["Door Test 2"] = "XmlFiles\\levels\\door_test_2.xml";
}

void UIQuickPlayScreen::AddLevelButtons()
{
	float startX = -940.0f;
	float startY = 480.0f;

	unsigned count = 0;
	for (const auto & l : mLevelEntries)
	{
		UIButton * button = new UIButton();
		button->SetBottomLeft(Vector2(startX, startY - count * 45));
		button->SetDimensions(Vector2(650, 35));
		button->SetIsProcessInput(true);
		button->SetButtonAssets("Media\\UI\\level_edit\\button_default.png",
								"Media\\UI\\level_edit\\button_default_2.png",
								"Media\\UI\\level_edit\\button_default.png");
		button->SetUseStandardEffect(true);

		button->LoadContent(Graphics::GetInstance()->Device());
		button->Initialise();

		button->SetButtonText(l.first, "Arial", 30, Vector3(0.1f, 0.1f, 0.1f));

		// assign events
		{
			// pop ui
			{
				EventStruct es;
				es.EventName = "popui";
				es.EventParams.push_back("level_select");
				button->AssignEventAction("actionspressup", es);
			}

			// stop sounds
			{
				EventStruct es;
				es.EventName = "stopallsounds";
				button->AssignEventAction("actionspressup", es);
			}

			// push level edit
			{
				EventStruct es;
				es.EventName = "pushui";
				es.EventParams.push_back("game_hud");
				button->AssignEventAction("actionspressup", es);
			}

			// push level 
			{
				EventStruct es;
				es.EventName = "loadlevel";
				es.EventParams.push_back(l.second);
				button->AssignEventAction("actionspressup", es);
			}
		}
		
		std::string keyPrefix = Utilities::ConvertDoubleToString(kButtonIndexStart + count);
		m_widgetMap[keyPrefix + "_level_edit_button"] = button;

		++count;
	}
}

void UIQuickPlayScreen::LoadContent(ID3D10Device * graphicsdevice)
{
	UIScreen::LoadContent(graphicsdevice);
}


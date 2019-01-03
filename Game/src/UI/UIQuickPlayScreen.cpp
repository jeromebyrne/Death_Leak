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
	mLevelEntries["a. Test Level"] = "XmlFiles\\levels\\test.xml";
	mLevelEntries["b. Test Level 2"] = "XmlFiles\\levels\\test_2.xml";
	mLevelEntries["c. Hot Springs"] = "XmlFiles\\levels\\hot_springs.xml";
	mLevelEntries["d. Cliff"] = "XmlFiles\\levels\\cliff.xml";
	mLevelEntries["e. Cliff Windmill"] = "XmlFiles\\levels\\cliff_windmill.xml";
	mLevelEntries["f. Ninja Arena 1"] = "XmlFiles\\levels\\first_ninja_arena.xml";
	mLevelEntries["g. Dojo"] = "XmlFiles\\levels\\dojo.xml";
	mLevelEntries["h. Cave Platforming 1"] = "XmlFiles\\levels\\cave_platforming_1.xml";
	mLevelEntries["i. Grass Exploration 1"] = "XmlFiles\\levels\\grass_exploration_1.xml";
	mLevelEntries["j. Grass Exploration 2"] = "XmlFiles\\levels\\grass_exploration_2.xml";
	mLevelEntries["k. Grass Exploration 3"] = "XmlFiles\\levels\\grass_exploration_3.xml";
	mLevelEntries["l. Grass Exploration Dead End Below 1"] = "XmlFiles\\levels\\grass_exploration_dead_end_below_1.xml";
	mLevelEntries["m. Grass Exploration Dead End Below 2"] = "XmlFiles\\levels\\grass_exploration_dead_end_below_2.xml";
	mLevelEntries["n. Water Well 1"] = "XmlFiles\\levels\\water_well_1.xml";
	mLevelEntries["o. Inside Hanging Shadow"] = "XmlFiles\\levels\\inside_hanging_shadow.xml";
	mLevelEntries["p. Catacombs Entrance 1"] = "XmlFiles\\levels\\catacombs_entrance_1.xml";
	mLevelEntries["q. Catacombs Entrance 2"] = "XmlFiles\\levels\\catacombs_entrance_2.xml";
	mLevelEntries["r. Grass River 1"] = "XmlFiles\\levels\\grass_river_1.xml";
	mLevelEntries["s. Underwater Cave 1"] = "XmlFiles\\levels\\underwater_cave.xml";
	mLevelEntries["s2. Underwater Cave 1.5"] = "XmlFiles\\levels\\underwater_cave_1_5.xml";
	mLevelEntries["t. Underwater Cave 2"] = "XmlFiles\\levels\\underwater_cave_2.xml";
	mLevelEntries["u. Underwater Cave 3"] = "XmlFiles\\levels\\underwater_cave_3.xml";
	mLevelEntries["v. Underwater Cave 4"] = "XmlFiles\\levels\\underwater_cave_4.xml";
	mLevelEntries["w. Underwater Cave 5"] = "XmlFiles\\levels\\underwater_cave_5.xml";
}

void UIQuickPlayScreen::AddLevelButtons()
{
	float startX = -940.0f;
	float startY = 500.0f;

	unsigned count = 0;
	for (const auto & l : mLevelEntries)
	{
		UIButton * button = new UIButton();
		button->SetBottomLeft(Vector2(startX, startY - count * 30));
		button->SetDimensions(Vector2(550, 25));
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


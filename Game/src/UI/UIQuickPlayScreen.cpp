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
	mLevelEntries["A. Test Level"] = "XmlFiles\\levels\\test.xml";
	mLevelEntries["B. Test Level 2"] = "XmlFiles\\levels\\test_2.xml";
	mLevelEntries["C. Hot Springs"] = "XmlFiles\\levels\\hot_springs.xml";
	mLevelEntries["D. Cliff"] = "XmlFiles\\levels\\cliff.xml";
	mLevelEntries["E. Cliff Windmill"] = "XmlFiles\\levels\\cliff_windmill.xml";
	mLevelEntries["F. Ninja Arena 1"] = "XmlFiles\\levels\\first_ninja_arena.xml";
	mLevelEntries["G. Dojo"] = "XmlFiles\\levels\\dojo.xml";
	mLevelEntries["H. Cave Platforming 1"] = "XmlFiles\\levels\\cave_platforming_1.xml";
	mLevelEntries["I. Grass Exploration 1"] = "XmlFiles\\levels\\grass_exploration_1.xml";
	mLevelEntries["J. Grass Exploration 2"] = "XmlFiles\\levels\\grass_exploration_2.xml";
	mLevelEntries["K. Grass Exploration 3"] = "XmlFiles\\levels\\grass_exploration_3.xml";
	mLevelEntries["L. Grass Exploration Dead End Below 1"] = "XmlFiles\\levels\\grass_exploration_dead_end_below_1.xml";
	mLevelEntries["M. Grass Exploration Dead End Below 2"] = "XmlFiles\\levels\\grass_exploration_dead_end_below_2.xml";
	mLevelEntries["N. Water Well 1"] = "XmlFiles\\levels\\water_well_1.xml";
	mLevelEntries["O. Inside Hanging Shadow"] = "XmlFiles\\levels\\inside_hanging_shadow.xml";
	mLevelEntries["P. Catacombs Entrance 1"] = "XmlFiles\\levels\\catacombs_entrance_1.xml";
	mLevelEntries["Q. Catacombs Entrance 2"] = "XmlFiles\\levels\\catacombs_entrance_2.xml";
	mLevelEntries["R. Grass River 1"] = "XmlFiles\\levels\\grass_river_1.xml";
	mLevelEntries["S. Underwater Cave 1"] = "XmlFiles\\levels\\underwater_cave.xml";
	mLevelEntries["S2. Underwater Cave 1.5"] = "XmlFiles\\levels\\underwater_cave_1_5.xml";
	mLevelEntries["T. Underwater Cave 2"] = "XmlFiles\\levels\\underwater_cave_2.xml";
	mLevelEntries["U. Underwater Cave 3"] = "XmlFiles\\levels\\underwater_cave_3.xml";
	mLevelEntries["V. Underwater Cave 4"] = "XmlFiles\\levels\\underwater_cave_4.xml";
	mLevelEntries["W. Underwater Cave 5"] = "XmlFiles\\levels\\underwater_cave_5.xml";
	mLevelEntries["X. Underwater Cave 6"] = "XmlFiles\\levels\\underwater_cave_6.xml";
	mLevelEntries["Y. Waterfall Bottom"] = "XmlFiles\\levels\\waterfall_bottom.xml";
	mLevelEntries["Z. Waterfall Top"] = "XmlFiles\\levels\\waterfall_top.xml";
	mLevelEntries["a. Forest Exploration 1"] = "XmlFiles\\levels\\forest_exploration_1.xml";
	mLevelEntries["b. Forest Exploration 2"] = "XmlFiles\\levels\\forest_exploration_2.xml";
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


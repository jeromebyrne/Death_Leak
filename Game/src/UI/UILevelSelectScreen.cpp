#include "precompiled.h"
#include "UILevelSelectScreen.h"
#include "GameObjectManager.h"
#include "UIButton.h"

static const unsigned kButtonIndexStart = 9000;

UILevelSelectScreen::UILevelSelectScreen(string name) :
	UIScreen(name)
{
}

UILevelSelectScreen::~UILevelSelectScreen(void)
{
}

void UILevelSelectScreen::Update()
{
	UIScreen::Update();
}

void UILevelSelectScreen::Initialise()
{
	UIScreen::Initialise();

	CreateLevelEntries();

	AddLevelButtons();
}

void UILevelSelectScreen::CreateLevelEntries()
{
	// mLevelEntries["A. Test Level"] = "XmlFiles\\levels\\test.xml";
	// mLevelEntries["B. Test Level 2"] = "XmlFiles\\levels\\test_2.xml";
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
	// mLevelEntries["Q. Catacombs Entrance 2"] = "XmlFiles\\levels\\catacombs_entrance_2.xml";
	// mLevelEntries["R. Grass River 1"] = "XmlFiles\\levels\\grass_river_1.xml";
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
	mLevelEntries["c. Forest Exploration Barn"] = "XmlFiles\\levels\\forest_exploration_barn.xml";
	mLevelEntries["d. Village Central"] = "XmlFiles\\levels\\village_central.xml";
	mLevelEntries["e. Village Shrine"] = "XmlFiles\\levels\\village_shrine.xml";
	mLevelEntries["f. Village Agri"] = "XmlFiles\\levels\\village_agri.xml";
	mLevelEntries["g. Village Forts"] = "XmlFiles\\levels\\village_forts.xml";
}

void UILevelSelectScreen::AddLevelButtons()
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
				es.EventParams.push_back("level_select_edit");
				button->AssignEventAction("actionspressup", es);
			}

			// push game loading ui
			{
				EventStruct es;
				es.EventName = "pushui";
				es.EventParams.push_back("gameloading");
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
				es.EventParams.push_back("level_edit_hud");
				button->AssignEventAction("actionspressup", es);
			}

			// pop ui
			{
				EventStruct es;
				es.EventName = "popui";
				es.EventParams.push_back("gameloading");
				button->AssignEventAction("actionspressup", es);
			}

			// push level edit UI
			{
				EventStruct es;
				es.EventName = "leveledit";
				es.EventParams.push_back(l.second);
				button->AssignEventAction("actionspressup", es);
			}
		}
		
		std::string keyPrefix = Utilities::ConvertDoubleToString(kButtonIndexStart + count);
		m_widgetMap[keyPrefix + "_level_edit_button"] = button;

		++count;
	}
}

void UILevelSelectScreen::LoadContent(ID3D10Device * graphicsdevice)
{
	UIScreen::LoadContent(graphicsdevice);
}

/*
<button name = "31_level_1"
	process_input = "true"
	posX = "-880" posY = "200" width = "480" height = "270"
	image = "Media\UI\level_edit\level_1_blur.png"
	image_focus = "Media\UI\level_edit\level_1.png"
	image_click = "Media\UI\level_edit\level_1.png"
	hflip = "false" vflip = "false"
	alpha = "1.0"
	use_standard_effect = "true">
	<actionspressup>
	<fadeout time = "150" / >
	<popui ui = "level_select_edit" / >
	<pushui ui = "gameloading" / >
	<fadein time = "150" / >
	<stopallsounds / >
	<leveledit level = "XmlFiles\village.xml" / >
	<popui ui = "gameloading" / >
	<pushui ui = "level_edit_hud" / >
	< / actionspressup>

	<actionspressdown>
	<playsoundeffect sound = "UI\\click.wav" / >
	< / actionspressdown>
	< / button>

	<button name = "32_level_2"
	process_input = "true"
	posX = "-300" posY = "200" width = "480" height = "270"
	image = "Media\UI\level_edit\level_2_blur.png"
	image_focus = "Media\UI\level_edit\level_2.png"
	image_click = "Media\UI\level_edit\level_2.png"
	hflip = "false" vflip = "false"
	alpha = "1.0"
	use_standard_effect = "true">
	<actionspressup>
	<fadeout time = "150" / >
	<popui ui = "level_select_edit" / >
	<pushui ui = "gameloading" / >
	<fadein time = "150" / >
	<stopallsounds / >
	<leveledit level = "XmlFiles\field_1.xml" / >
	<popui ui = "gameloading" / >
	<pushui ui = "level_edit_hud" / >
	< / actionspressup>

	<actionspressdown>
	<playsoundeffect sound = "UI\\click.wav" / >
	< / actionspressdown>
	< / button>

	<button name = "33_level_3"
	process_input = "true"
	posX = "280" posY = "200" width = "480" height = "270"
	image = "Media\UI\level_edit\level_3_blur.png"
	image_focus = "Media\UI\level_edit\level_3.png"
	image_click = "Media\UI\level_edit\level_3.png"
	hflip = "false" vflip = "false"
	alpha = "1.0"
	use_standard_effect = "true">
	<actionspressup>
	<fadeout time = "150" / >
	<popui ui = "level_select_edit" / >
	<pushui ui = "gameloading" / >
	<fadein time = "150" / >
	<stopallsounds / >
	<leveledit level = "XmlFiles\village_outskirts.xml" / >
	<popui ui = "gameloading" / >
	<pushui ui = "level_edit_hud" / >
	< / actionspressup>

	<actionspressdown>
	<playsoundeffect sound = "UI\\click.wav" / >
	< / actionspressdown>
	< / button>

	<button name = "34_level_4"
	process_input = "true"
	posX = "-520" posY = "-150" width = "480" height = "270"
	image = "Media\UI\level_edit\level_4_blur.png"
	image_focus = "Media\UI\level_edit\level_4.png"
	image_click = "Media\UI\level_edit\level_4.png"
	hflip = "false" vflip = "false"
	alpha = "1.0"
	use_standard_effect = "true">
	<actionspressup>
	<fadeout time = "150" / >
	<popui ui = "level_select_edit" / >
	<pushui ui = "gameloading" / >
	<fadein time = "150" / >
	<stopallsounds / >
	<leveledit level = "XmlFiles\waterfall_level.xml" / >
	<popui ui = "gameloading" / >
	<pushui ui = "level_edit_hud" / >
	< / actionspressup>

	<actionspressdown>
	<playsoundeffect sound = "UI\\click.wav" / >
	< / actionspressdown>
	< / button>

	<button name = "35_level_5"
	process_input = "true"
	posX = "40" posY = "-150" width = "480" height = "270"
	image = "Media\UI\level_edit\level_5_blur.png"
	image_focus = "Media\UI\level_edit\level_5.png"
	image_click = "Media\UI\level_edit\level_5.png"
	hflip = "false" vflip = "false"
	alpha = "1.0"
	use_standard_effect = "true">
	<actionspressup>
	<fadeout time = "150" / >
	<popui ui = "level_select_edit" / >
	<pushui ui = "gameloading" / >
	<fadein time = "150" / >
	<stopallsounds / >
	<leveledit level = "XmlFiles\underwater_cave.xml" / >
	<popui ui = "gameloading" / >
	<pushui ui = "level_edit_hud" / >
	< / actionspressup>

	<actionspressdown>
	<playsoundeffect sound = "UI\\click.wav" / >
	< / actionspressdown>
	< / button>

	<button name = "36_level_6"
	process_input = "true"
	posX = "-150" posY = "-450" width = "480" height = "270"
	image = "Media\UI\level_edit\level_5_blur.png"
	image_focus = "Media\UI\level_edit\level_5.png"
	image_click = "Media\UI\level_edit\level_5.png"
	hflip = "false" vflip = "false"
	alpha = "1.0"
	use_standard_effect = "true">
	<actionspressup>
	<fadeout time = "150" / >
	<popui ui = "level_select_edit" / >
	<pushui ui = "gameloading" / >
	<fadein time = "150" / >
	<stopallsounds / >
	<leveledit level = "XmlFiles\village_2.xml" / >
	<popui ui = "gameloading" / >
	<pushui ui = "level_edit_hud" / >
	< / actionspressup>

	<actionspressdown>
	<playsoundeffect sound = "UI\\click.wav" / >
	< / actionspressdown>
	< / button>

	<button name = "37_level_test"
	process_input = "true"
	posX = "-800" posY = "-500" width = "240" height = "135"
	image = "Media\UI\level_edit\level_5_blur.png"
	image_focus = "Media\UI\level_edit\level_5.png"
	image_click = "Media\UI\level_edit\level_5.png"
	hflip = "false" vflip = "false"
	alpha = "1.0"
	use_standard_effect = "true">
	<actionspressup>
	<fadeout time = "150" / >
	<popui ui = "level_select_edit" / >
	<pushui ui = "gameloading" / >
	<fadein time = "150" / >
	<stopallsounds / >
	<leveledit level = "XmlFiles\levels\test_level.xml" / >
	<popui ui = "gameloading" / >
	<pushui ui = "level_edit_hud" / >
	< / actionspressup>

	<actionspressdown>
	<playsoundeffect sound = "UI\\click.wav" / >
	< / actionspressdown>
	< / button>



	*/

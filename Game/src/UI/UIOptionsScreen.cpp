#include "precompiled.h"
#include "UIOptionsScreen.h"
#include "UICheckbox.h"
#include "Settings.h"

UIOptionsScreen::UIOptionsScreen(string name) :
	UIScreen(name)
{
}

UIOptionsScreen::~UIOptionsScreen(void)
{
}

void UIOptionsScreen::Initialise()
{
	UIScreen::Initialise();

	// loop through the widgets and get the meter
	for (const auto & kvp : m_widgetMap)
	{
		UICheckbox * checkbox = dynamic_cast<UICheckbox*>(kvp.second);

		if (checkbox)
		{
			if (checkbox->Name().find("2_music_checkbox") != std::string::npos)
			{
				mMusicCheckbox = checkbox;

				if (mMusicCheckbox != nullptr)
				{
					continue;
				}
			}

			// s1.find(s2) != std::string::npos
			if (checkbox->Name().find("3_sound_checkbox") != std::string::npos)
			{
				mSfxCheckbox = checkbox;

				if (mSfxCheckbox != nullptr)
				{
					continue;
				}
			}
		}
	}

	auto settings = Settings::GetInstance();

	if (mSfxCheckbox)
	{
		mSfxCheckbox->SetChecked(settings->IsSFXEnabled());
	}

	if (mMusicCheckbox)
	{
		mMusicCheckbox->SetChecked(settings->IsMusicEnabled());
	}
}

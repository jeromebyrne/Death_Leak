#include "precompiled.h"
#include "UIGameHudScreen.h"
#include "UIMeter.h"
#include "GameObjectManager.h"
#include "SaveManager.h"
#include "PlayerLevelManager.h"

UIGameHudScreen::UIGameHudScreen(string name) : 
	UIScreen(name),
	mPlayerHealthMeter(nullptr),
	mPlayerXPMeter(nullptr)
{
}

UIGameHudScreen::~UIGameHudScreen(void)
{
}

void UIGameHudScreen::Update()
{
	UIScreen::Update();

	const Player * player = GameObjectManager::Instance()->GetPlayer();

	if (player && mPlayerHealthMeter)
	{
		float player_health = player->GetHealth();
		float player_max_health = player->GetMaxHealth();

		if (player_health > 0 && player_max_health > 0)
		{
			mPlayerHealthMeter->SetProgress(player_health / player_max_health);
		}
		else
		{
			mPlayerHealthMeter->SetProgress(0);
		}
	}

	if (player && mPlayerXPMeter)
	{
		// Player XP is repurposed into FOCUS
		float focusPercent = player->GetCurrentFocusAmount() / player->GetMaxFocusAmount();

		mPlayerXPMeter->SetProgress(focusPercent);
	}
}

void UIGameHudScreen::Initialise()
{
	UIScreen::Initialise();

	// loop through the widgets and get the meter
	for (const auto & kvp : m_widgetMap)
	{
		UIMeter * meter = dynamic_cast<UIMeter*>(kvp.second);

		if (meter)
		{
			// s1.find(s2) != std::string::npos
			if ( meter->Name().find("health_meter") != std::string::npos)
			{
				mPlayerHealthMeter = meter;

				if (mPlayerXPMeter != nullptr)
				{
					break;
				}
			}

			if (meter->Name().find("xp_meter") != std::string::npos)
			{
				mPlayerXPMeter = meter;

				if (mPlayerHealthMeter != nullptr)
				{
					break;
				}
			}
		}
		
	}
}

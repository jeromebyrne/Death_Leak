#include "precompiled.h"
#include "UIGameHudScreen.h"
#include "UIMeter.h"
#include "GameObjectManager.h"

UIGameHudScreen::UIGameHudScreen(string name) : 
UIScreen(name),
mPlayerHealthMeter(0)
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
}

void UIGameHudScreen::Initialise()
{
	UIScreen::Initialise();

	// loop through the widgets and get the meter
	map<string, UIWidget*>::const_iterator iter = m_widgetMap.begin();
	for (; iter != m_widgetMap.end(); ++iter)
	{
		UIMeter * meter = dynamic_cast<UIMeter*>((*iter).second);

		if (meter)
		{
			mPlayerHealthMeter = meter;
			break;
		}
	}
}

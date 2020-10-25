#include "precompiled.h"
#include "UIGameHudScreen.h"
#include "UIMeter.h"
#include "GameObjectManager.h"
#include "SaveManager.h"
#include "PlayerLevelManager.h"
#include "AudioManager.h"
#include "Game.h"

static const float kHealthMeterDimensionsMultiplier = 3.0f;
static const float kFocusMeterDimensionsMultiplier = 2.0f;
static const D3DXCOLOR kOrbTextColor = D3DXCOLOR(0.7f, 0.7f, 0.7f, 0.8f);

UIGameHudScreen::UIGameHudScreen(string name) : 
	UIScreen(name),
	mPlayerHealthMeter(nullptr),
	mPlayerXPMeter(nullptr)
{
}

UIGameHudScreen::~UIGameHudScreen(void)
{
	if (mMeterUpgradeSound != nullptr)
	{
		mMeterUpgradeSound->drop();
		mMeterUpgradeSound = nullptr;
	}

	if (mOrbCountText)
	{
		mOrbCountText->Release();
		mOrbCountText = nullptr;
	}
}

void UIGameHudScreen::Update()
{
	UIScreen::Update();

	Player * player = GameObjectManager::Instance()->GetPlayer();

	if (player == nullptr || mPlayerHealthMeter == nullptr)
	{
		return;
	}

	UpdatePlayerHealthMeter(player);

	UpdatePlayerFocusMeter(player);
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

	InitialiseText();

	mSaveManagerCached = SaveManager::GetInstance();
}

void UIGameHudScreen::UpdatePlayerHealthMeter(Player * player)
{
	float player_health = player->GetHealth();
	float player_max_health = player->GetMaxHealth();

	if (mDoingHealthMeterUpgrade)
	{
		DoHealthMeterUpgrade(player);
		// return;
	}

	if (player_max_health != mLastPlayerMaxHealth)
	{
		mLastPlayerMaxHealth = player_max_health;

		mDoingHealthMeterUpgrade = true;
		if (mMeterUpgradeSound == nullptr)
		{
			mMeterUpgradeSound = AudioManager::Instance()->PlaySoundEffect("character/meter_upgrade.wav", true, true);
		}
		else
		{
			mMeterUpgradeSound->setPlayPosition(0);
			mMeterUpgradeSound->setIsPaused(false);
		}
		return;
	}

	if (player_health > 0 && player_max_health > 0)
	{
		mPlayerHealthMeter->SetProgress(player_health / player_max_health);
	}
	else
	{
		mPlayerHealthMeter->SetProgress(0);
	}
}

void UIGameHudScreen::UpdatePlayerFocusMeter(Player * player)
{
	float player_max_focus = player->GetMaxFocusAmount();

	if (player_max_focus <= 0.0f)
	{
		mPlayerXPMeter->SetDrawMeter(false);
		mPlayerXPMeter->SetMeterLength(0.0f);
		return;
	}
	else
	{
		mPlayerXPMeter->SetDrawMeter(true);
	}

	if (mDoingFocusMeterUpgrade)
	{
		DoFocusMeterUpgrade(player);
		// return;
	}

	if (player_max_focus != mLastPlayerMaxFocus)
	{
		mLastPlayerMaxFocus = player_max_focus;

		mDoingFocusMeterUpgrade = true;
		if (mMeterUpgradeSound == nullptr)
		{
			mMeterUpgradeSound = AudioManager::Instance()->PlaySoundEffect("character/meter_upgrade.wav", true, true);
		}
		else
		{
			mMeterUpgradeSound->setPlayPosition(0);
			mMeterUpgradeSound->setIsPaused(false);
		}
		return;
	}

	if (player && mPlayerXPMeter)
	{
		// Player XP is repurposed into FOCUS
		float focusPercent = player->GetCurrentFocusAmount() / player->GetMaxFocusAmount();

		mPlayerXPMeter->SetProgress(focusPercent);
	}
}

void UIGameHudScreen::DoHealthMeterUpgrade(Player * player)
{
	float player_max_health = player->GetMaxHealth();

	float currentDimensions = mPlayerHealthMeter->GetMeterLength();
	float finalDimensions = player_max_health * kHealthMeterDimensionsMultiplier;
	if (currentDimensions < finalDimensions)
	{
		mPlayerHealthMeter->SetMeterLength(currentDimensions + 0.20f);
		Camera2D::GetInstance()->DoSmallShake();
		Game::GetInstance()->Vibrate(0.15f, 0.1f, 0.1f);
	}
	else if (currentDimensions >= finalDimensions)
	{
		mPlayerHealthMeter->SetMeterLength(finalDimensions);
		player->IncreaseHealth(9999999); // max out health
		mDoingHealthMeterUpgrade = false;

		if (mMeterUpgradeSound != nullptr)
		{
			mMeterUpgradeSound->setIsPaused(true);
		}
	}
}

void UIGameHudScreen::DoFocusMeterUpgrade(Player * player)
{
	float player_max_focus = player->GetMaxFocusAmount();

	float currentDimensions = mPlayerXPMeter->GetMeterLength();
	float finalDimensions = player_max_focus * kFocusMeterDimensionsMultiplier;
	if (currentDimensions < finalDimensions)
	{
		mPlayerXPMeter->SetMeterLength(currentDimensions + (player_max_focus <= 100.0f ? 0.5f : 0.20f));
		Camera2D::GetInstance()->DoSmallShake();
		Game::GetInstance()->Vibrate(0.15f, 0.1f, 0.1f);
	}
	else if (currentDimensions >= finalDimensions)
	{
		mPlayerXPMeter->SetMeterLength(finalDimensions);
		// player->IncreaseHealth(9999999); // max out health
		mDoingFocusMeterUpgrade = false;

		if (mMeterUpgradeSound != nullptr)
		{
			mMeterUpgradeSound->setIsPaused(true);
		}
	}
}

void UIGameHudScreen::Draw(ID3D10Device * device)
{
	UIScreen::Draw(device);

	if (mOrbCountText)
	{
		int newOrbs = mSaveManagerCached->GetNumCurrencyOrbsCollected();

		if (mLastOrbCount != newOrbs)
		{
			mOrbtext = std::to_string(newOrbs);
		}

		RECT rect = { 105, 135, 0, 0 };
		mOrbCountText->DrawTextA(0,  mOrbtext.c_str() , -1, &rect, DT_NOCLIP, kOrbTextColor);
	}
}

void UIGameHudScreen::InitialiseText()
{
	// orb count text
	{
		D3DX10_FONT_DESC fd;
		fd.Height = 32;
		fd.Width = 0;
		fd.Weight = 0;
		fd.MipLevels = 1;
		fd.Italic = false;
		fd.CharSet = OUT_DEFAULT_PRECIS;
		fd.Quality = DEFAULT_QUALITY;
		fd.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		wcscpy(fd.FaceName, L"Impact");

		D3DX10CreateFontIndirect(Graphics::GetInstance()->Device(), &fd, &mOrbCountText);
	}
}

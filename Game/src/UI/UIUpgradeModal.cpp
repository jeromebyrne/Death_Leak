#include "precompiled.h"
#include "UIUpgradeModal.h"
#include "GamePad.h"
#include "Game.h"
#include "UISprite.h"
#include "SaveManager.h"
#include "AudioManager.h"
#include "ParticleEmitterManager.h"

static const float kTitleTextDimensionsX = 1600.0f;
static const float kTitleTextDimensionsY = 100.0f;

static const float kDescriptionTextDimensionsX = 1600.0f;
static const float kDescriptionTextDimensionsY = 500.0f;
static const float kDismissWarmup = 1.0f;

static const D3DXCOLOR kTitleColor = D3DXCOLOR(0.6f, 0.6f, 0.6f, 1.0f);
static const D3DXCOLOR kDescColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
static const D3DXCOLOR kInsuffientOrbsColor = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);


UIUpgradeModal::UIUpgradeModal(string name) :
	UIScreen(name)
{
}

UIUpgradeModal::~UIUpgradeModal(void)
{
	if (mDescriptionFont)
	{
		mDescriptionFont->Release();
		mDescriptionFont = nullptr;
	}

	if (mTitleFont)
	{
		mTitleFont->Release();
		mTitleFont = nullptr;
	}
}

void UIUpgradeModal::Update()
{
	UIScreen::Update();

	// TODO: need to defer the popping of the UI to avoid a crash
	GamePad* gamepad1 = GamePad::GetPad1();
	auto padState = gamepad1->GetState();

	if (mDismissButtonCurrentWarmup < kDismissWarmup)
	{
		mDismissButtonCurrentWarmup += Timing::Instance()->GetLastUpdateDelta();
	}
	else
	{
		if (gamepad1 && gamepad1->IsConnected())
		{
			mBackButtonSprite_gamepad->SetAlpha(1.0f);

			if (padState.Gamepad.wButtons & XINPUT_GAMEPAD_B)
			{
				mDismissButtonCurrentWarmup = 0.0f;
				Game::GetInstance()->DismissUpgradeModal();
			}
		}
		else
		{
			mBackButtonSprite_keyboard->SetAlpha(1.0f);

			if (GetAsyncKeyState(VK_ESCAPE) < 0)
			{
				mDismissButtonCurrentWarmup = 0.0f;
				Game::GetInstance()->DismissUpgradeModal();
			}
		}
	}

	if (gamepad1 && gamepad1->IsConnected())
	{
		if (padState.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
		{
			if (CanPurchase())
			{
				DoPurchase();
				mDismissButtonCurrentWarmup = 1.0f;
				Game::GetInstance()->DismissUpgradeModal();
			}
			else
			{
				// play some failure SFX
				// AudioManager::Instance()->PlaySoundEffect("stonehit.wav");
			}
		}
	}
}

void UIUpgradeModal::Initialise()
{
	UIScreen::Initialise();

	// create title font
	{
		D3DX10_FONT_DESC fd;
		fd.Height = 50;
		fd.Width = 0;
		fd.Weight = 0;
		fd.MipLevels = 1;
		fd.Italic = false;
		fd.CharSet = OUT_DEFAULT_PRECIS;
		fd.Quality = DEFAULT_QUALITY;
		fd.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		wcscpy(fd.FaceName, Utilities::ConvertCharStringToWcharString("Impact"));

		D3DX10CreateFontIndirect(Graphics::GetInstance()->Device(), &fd, &mTitleFont);
	}

	// create description font
	{
		D3DX10_FONT_DESC fd;
		fd.Height = 75;
		fd.Width = 0;
		fd.Weight = 0;
		fd.MipLevels = 1;
		fd.Italic = false;
		fd.CharSet = OUT_DEFAULT_PRECIS;
		fd.Quality = DEFAULT_QUALITY;
		fd.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		wcscpy(fd.FaceName, Utilities::ConvertCharStringToWcharString("Impact"));

		D3DX10CreateFontIndirect(Graphics::GetInstance()->Device(), &fd, &mDescriptionFont);
	}

	for (const auto & kvp : m_widgetMap)
	{
		UISprite * s = dynamic_cast<UISprite*>(kvp.second);

		if (s == nullptr)
		{
			continue;
		}

		if (s->Name().find("2_back_button_image_gamepad") != std::string::npos)
		{
			mBackButtonSprite_gamepad = s;
			mBackButtonSprite_gamepad->SetAlpha(0.0f);
		}
		else if (s->Name().find("3_back_button_image_keyboard") != std::string::npos)
		{
			mBackButtonSprite_keyboard = s;
			mBackButtonSprite_keyboard->SetAlpha(0.0f);
		}
		else if (s->Name().find("4_upgrade_button_image_gamepad") != std::string::npos)
		{
			mUpgradeButtonSprite_gamepad = s;
			// mUpgradeButtonSprite_gamepad->SetAlpha(0.0f);
		}
	}
}

void UIUpgradeModal::LoadContent(ID3D10Device * graphicsdevice)
{
	UIScreen::LoadContent(graphicsdevice);
}

void UIUpgradeModal::Draw(ID3D10Device * device)
{
	UIScreen::Draw(device);

	// Title
	{
		if (!mLocalizedTitle.empty())
		{
			float halfDimensionsX = kTitleTextDimensionsX * 0.5f;
			float halfDimensionsY = kTitleTextDimensionsY * 0.5f;

			RECT bounds = { 960.0f - halfDimensionsX, 100, 960.0f + halfDimensionsX, 200 };
			mTitleFont->DrawTextA(0, mLocalizedTitle.c_str(), -1, &bounds, DT_CENTER | DT_TOP, kTitleColor);
		}
	}

	// description
	{
		if (!mLocalizedDescription.empty())
		{
			float halfDimensionsX = kDescriptionTextDimensionsX * 0.5f;
			float halfDimensionsY = kDescriptionTextDimensionsY * 0.5f;

			RECT bounds = { 960.0f - halfDimensionsX + 100.0f,
							540.0f - halfDimensionsY - 75.0f, 
							960.0f + halfDimensionsX - 100.0f, 
							540.0f + halfDimensionsY - 75.0f };
			mDescriptionFont->DrawTextA(0, mLocalizedDescription.c_str(), -1, &bounds, DT_WORDBREAK | DT_CENTER | DT_VCENTER, kDescColor);
		}
	}

	// upgrade cost
	{
		if (!mUpgradeCostAsString.empty())
		{
			float halfDimensionsX = kTitleTextDimensionsX * 0.5f;
			float halfDimensionsY = kTitleTextDimensionsY * 0.5f;

			RECT bounds = { 960.0f - halfDimensionsX - 50.0, 735, 960.0f + halfDimensionsX, 835 };
			mTitleFont->DrawTextA(0, mUpgradeCostAsString.c_str(), -1, &bounds, DT_CENTER | DT_BOTTOM | DT_LEFT, CanPurchase() ? kTitleColor : kInsuffientOrbsColor);
		}
	}

	// insufficient orbs text
	if (!CanPurchase())
	{
		float halfDimensionsX = kTitleTextDimensionsX * 0.5f;
		float halfDimensionsY = kTitleTextDimensionsY * 0.5f;

		RECT bounds = { 960.0f - halfDimensionsX, 850, 960.0f + halfDimensionsX, 950 };
		mTitleFont->DrawTextA(0, "NOT ENOUGH ORBS", -1, &bounds, DT_CENTER | DT_BOTTOM | DT_LEFT, kInsuffientOrbsColor);
	}
}

void UIUpgradeModal::SetLocalizedDescription(const string & desc)
{
	mLocalizedDescription = desc;
}

void UIUpgradeModal::SetLocalizedTitle(const string & title)
{
	mLocalizedTitle = title;
}

void UIUpgradeModal::SetUpgradeType(FeatureUnlockManager::FeatureType featureUnlock)
{
	mFeatureType = featureUnlock;
}

void UIUpgradeModal::SetUpgradeCost(int cost)
{
	mUpgradeCost = cost;
	mUpgradeCostAsString = Utilities::ConvertDoubleToString((double)mUpgradeCost);
}

bool UIUpgradeModal::CanPurchase()
{
#if _DEBUG
	// testing
	//return true;
#endif

	int currentOrbs = SaveManager::GetInstance()->GetNumCurrencyOrbsCollected();

	if (currentOrbs < mUpgradeCost)
	{
		return false;
	}
}

void UIUpgradeModal::DoPurchase()
{
	int currentOrbCount = SaveManager::GetInstance()->GetNumCurrencyOrbsCollected();

	SaveManager::GetInstance()->SetNumCurrencyOrbsCollected(currentOrbCount - mUpgradeCost);

	FeatureUnlockManager::GetInstance()->SetFeatureUnlocked(mFeatureType);

	// TODO: do some VFX and SFX
	Game::GetInstance()->Vibrate(1.0f, 1.0f, 1.0f);

	// Do effects
	// AudioManager::Instance()->PlaySoundEffect("gong.wav", false, false, false);
	AudioManager::Instance()->PlaySoundEffect("music\\japanese1.wav", false, false, false);
	Camera2D::GetInstance()->DoBigShake();

	auto player = GameObjectManager::Instance()->GetPlayer();

	if (player == nullptr)
	{
		return;
	}

	ParticleEmitterManager::Instance()->CreateDirectedSpray(20,
															player->Position(),
															GameObject::kGroundFront,
															Vector2(0.0f, 0.0f),
															0.1f,
															Vector2(3200.0f, 1200.0f),
															"Media\\explosion_lines.png",
															1.5f,
															4.0f,
															0.5f,
															0.9f,
															128.0f,
															256.0f,
															0.0f,
															false,
															1.0f,
															1.0f,
															0.0f,
															true,
															12.0f,
															0.0f,
															0.0f,
															0.05f,
															0.1f,
															true);
}


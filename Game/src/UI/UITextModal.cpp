#include "precompiled.h"
#include "UITextModal.h"
#include "GamePad.h"
#include "Game.h"
#include "UISprite.h"

static const float kTitleTextDimensionsX = 1600.0f;
static const float kTitleTextDimensionsY = 100.0f;

static const float kDescriptionTextDimensionsX = 1600.0f;
static const float kDescriptionTextDimensionsY = 500.0f;
static const float kDismissWarmup = 1.0f;

static const D3DXCOLOR kTitleColor = D3DXCOLOR(0.6f, 0.6f, 0.6f, 1.0f);
static const D3DXCOLOR kDescColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

UITextModal::UITextModal(string name) :
	UIScreen(name)
{
}

UITextModal::~UITextModal(void)
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

void UITextModal::Update()
{
	UIScreen::Update();

	if (mDismissButtonCurrentWarmup < kDismissWarmup)
	{
		mDismissButtonCurrentWarmup += Timing::Instance()->GetLastUpdateDelta();
	}
	else
	{
		// TODO: need to defer the popping of the UI to avoid a crash
		GamePad * gamepad1 = GamePad::GetPad1();

		if (gamepad1 && gamepad1->IsConnected())
		{
			mBackButtonSprite_gamepad->SetAlpha(1.0f);

			auto padState = gamepad1->GetState();
			if (padState.Gamepad.wButtons & XINPUT_GAMEPAD_B)
			{
				mDismissButtonCurrentWarmup = 0.0f;
				Game::GetInstance()->DismissTextModal();
			}
		}
		else
		{
			mBackButtonSprite_keyboard->SetAlpha(1.0f);

			if (GetAsyncKeyState(VK_ESCAPE) < 0)
			{
				mDismissButtonCurrentWarmup = 0.0f;
				Game::GetInstance()->DismissTextModal();
			}
		}
	}
}

void UITextModal::Initialise()
{
	UIScreen::Initialise();

	// create title font
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

		D3DX10CreateFontIndirect(Graphics::GetInstance()->Device(), &fd, &mTitleFont);
	}

	// create description font
	{
		D3DX10_FONT_DESC fd;
		fd.Height = 95;
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
	}
}

void UITextModal::LoadContent(ID3D10Device * graphicsdevice)
{
	UIScreen::LoadContent(graphicsdevice);
}

void UITextModal::Draw(ID3D10Device * device)
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

			RECT bounds = { 960.0f - halfDimensionsX, 540.0f - halfDimensionsY, 960.0f + halfDimensionsX, 540.0f + halfDimensionsY };
			mDescriptionFont->DrawTextA(0, mLocalizedDescription.c_str(), -1, &bounds, DT_WORDBREAK | DT_CENTER | DT_VCENTER, kDescColor);
		}
	}
}

void UITextModal::SetLocalizedDescription(const string & desc)
{
	mLocalizedDescription = desc;
}

void UITextModal::SetLocalizedTitle(const string & title)
{
	mLocalizedTitle = title;
}


#include "precompiled.h"
#include "UITextModal.h"
#include "GamePad.h"
#include "Game.h"

static const float kTextDimensionsX = 1400.0f;
static const float kTextDimensionsY = 900.0f;
static const float kDismissWarmup = 0.5f;

UITextModal::UITextModal(string name) :
	UIScreen(name)
{
}

UITextModal::~UITextModal(void)
{
	if (mDescriptionText)
	{
		mDescriptionText->Release();
		mDescriptionText = nullptr;
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
			auto padState = gamepad1->GetState();
			if (padState.Gamepad.wButtons & XINPUT_GAMEPAD_B)
			{
				Game::GetInstance()->DismissTextModal();
			}
		}
		else
		{
			if (GetAsyncKeyState(VK_ESCAPE) < 0)
			{
				Game::GetInstance()->DismissTextModal();
			}
		}
	}
}

void UITextModal::Initialise()
{
	UIScreen::Initialise();

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

		D3DX10CreateFontIndirect(Graphics::GetInstance()->Device(), &fd, &mDescriptionText);
	}
}

void UITextModal::LoadContent(ID3D10Device * graphicsdevice)
{
	UIScreen::LoadContent(graphicsdevice);
}

void UITextModal::Draw(ID3D10Device * device)
{
	UIScreen::Draw(device);

	float halfDimensionsX = kTextDimensionsX * 0.5f;
	float halfDimensionsY = kTextDimensionsY * 0.5f;

	RECT bounds = { 960.0f - halfDimensionsX, 540.0f - halfDimensionsY, 960.0f + halfDimensionsX, 540.0f + halfDimensionsY };
	mDescriptionText->DrawTextA(0, mLocalizedDescription.c_str(), -1, &bounds, DT_WORDBREAK | DT_CENTER | DT_VCENTER, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
}

void UITextModal::SetLocalizedDescription(const string & desc)
{
	mLocalizedDescription = desc;
}


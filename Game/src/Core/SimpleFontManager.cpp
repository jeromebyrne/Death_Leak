#include "precompiled.h"
#include "SimpleFontManager.h"

SimpleFontManager::SimpleFontManager(void):
	mDebugFont(nullptr)
{
}

SimpleFontManager::~SimpleFontManager(void)
{
	Release();
}

void SimpleFontManager::Init(Graphics * graphics)
{
	// DEBUG font
	{
		D3DX10_FONT_DESC fd;
		fd.Height = 30;
		fd.Width = 0;
		fd.Weight = 0;
		fd.MipLevels = 1;
		fd.Italic = false;
		fd.CharSet = OUT_DEFAULT_PRECIS;
		fd.Quality = DEFAULT_QUALITY;
		fd.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		wcscpy(fd.FaceName, L"Impact");

		D3DX10CreateFontIndirect(graphics->Device(), &fd, &mDebugFont);
		mDebugFontColor = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	}
}

void SimpleFontManager::Release()
{
	if (mDebugFont)
	{
		mDebugFont->Release();
		mDebugFont = 0;
	}
}

void SimpleFontManager::DrawDebugText(const char * text, float top, float left)
{
	RECT rectangle = {top, left, 0, 0};

	// TODO: optimise, cache the wchar_t rather than constant conversion
	wchar_t * w_text =  Utilities::ConvertCharStringToWcharString(text);
	mDebugFont->DrawText(0,w_text, -1, &rectangle, DT_NOCLIP, mDebugFontColor);
	delete w_text;
}

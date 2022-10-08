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
		fd.Height = 16;
		fd.Width = 0;
		fd.Weight = 0;
		fd.MipLevels = 1;
		fd.Italic = false;
		fd.CharSet = OUT_DEFAULT_PRECIS;
		fd.Quality = DEFAULT_QUALITY;
		fd.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		wcscpy(fd.FaceName, L"Arial");

		D3DX10CreateFontIndirect(graphics->Device(), &fd, &mDebugFont);
		mDebugFontColor = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	}
}

void SimpleFontManager::Release()
{
	if (mDebugFont)
	{
		mDebugFont->Release();
		mDebugFont = nullptr;
	}
}

void SimpleFontManager::DrawDebugText(const char * text, float top, float left)
{
	RECT rectangle = {(LONG)top, (LONG)left, 0l, 0l};

	if (strcmp(text, "")==0)
	{
		return;
	}

	mDebugFont->DrawTextA(0, text, -1, &rectangle, DT_NOCLIP, mDebugFontColor);
}

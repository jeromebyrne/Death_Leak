#ifndef SIMPLEFONTMANAGER_H
#define SIMPLEFONTMANAGER_H

class SimpleFontManager
{
public:
	SimpleFontManager(void);
	virtual ~SimpleFontManager(void);

	void Init(Graphics * graphics);

	void Release();

	void DrawDebugText(const char * text, float top, float left);

private:

	ID3DX10Font* mDebugFont;
	D3DXCOLOR mDebugFontColor;
};

#endif


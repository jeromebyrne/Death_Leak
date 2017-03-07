#ifndef UIBUTTON_H
#define UIBUTTON_H

#include "uisprite.h"

class UIButton : public UISprite
{
public:

	UIButton(void);
	virtual ~UIButton(void);
	virtual void Release();

	virtual void OnFocus() override; // what do we do when we come into focus
	virtual void OnLoseFocus() override; // what do we do when we lose focus

	virtual void LoadContent(ID3D10Device * graphicsdevice) override;
	virtual void XmlRead(TiXmlElement * element) override;

	virtual void Draw(ID3D10Device * graphicsdevice) override;

	void SetButtonAssets(const string & focusFilename,
						const string & noFocusFilename,
						const string & clickFilename);

	void SetButtonText(const string & text, const string & fontName, unsigned int fontSize,  const Vector3 & fontColor);

protected:

	virtual void OnPressDown();
	virtual void OnPressUp();

private:

	// 3 textures for 3 button states
	ID3D10ShaderResourceView* mNoFocusTexture;
	ID3D10ShaderResourceView* mFocusTexture;
	ID3D10ShaderResourceView* mClickTexture;

	string mNoFocusTextureFileName;
	string mFocusTextureFileName;
	string mClickTextureFileName;

	ID3DX10Font* mFont = nullptr;
	wchar_t * mCachedWideString = nullptr;
	D3DXCOLOR mFontColor;
};

#endif

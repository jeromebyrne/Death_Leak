#ifndef UIBUTTON_H
#define UIBUTTON_H

#include "uisprite.h"

class UIButton : public UISprite
{
private:
	// 3 textures for 3 button states
	ID3D10ShaderResourceView* mNoFocusTexture;
	ID3D10ShaderResourceView* mFocusTexture;
	ID3D10ShaderResourceView* mClickTexture;

	string mNoFocusTextureFileName;
	string mFocusTextureFileName;
	string mClickTextureFileName;
protected:

	virtual void OnPressDown();
	virtual void OnPressUp();

public:
	UIButton(void);
	virtual ~UIButton(void);
	virtual void Release();

	virtual void OnFocus(); // what do we do when we come into focus
	virtual void OnLoseFocus(); // what do we do when we lose focus

	virtual void LoadContent(ID3D10Device * graphicsdevice);
	virtual void XmlRead(TiXmlElement * element);
};

#endif

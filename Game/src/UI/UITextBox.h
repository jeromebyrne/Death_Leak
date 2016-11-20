#ifndef UITEXTBOX_H
#define UITEXTBOX_H

#include "uiwidget.h"

class UISprite;

class UITextBox : public UIWidget
{
public:

	UITextBox(void);
	virtual ~UITextBox(void);
	virtual void Release();
	virtual void LoadContent(ID3D10Device * graphicsdevice);
	virtual void XmlRead(TiXmlElement * element);
	virtual void Draw(ID3D10Device * graphicsdevice);
	virtual void Initialise();
	virtual void Reset();

	virtual void OnFocus() override;
	virtual void OnLoseFocus() override;

protected:

	virtual void OnPressDown() override;
	virtual void OnPressUp() override;

	UISprite * mBackgroundImage;

	Vector2 mFocusDimensions;
	Vector2 mNormalDimensions;
};

#endif

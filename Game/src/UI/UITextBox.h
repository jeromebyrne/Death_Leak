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
	virtual void Update() override;

	virtual void OnFocus() override;
	virtual void OnLoseFocus() override;

	std::string GetText() const { return mText; }

	void SetText(const std::string text);

	void SetIsEditable(bool editable);

protected:

	virtual void OnPressDown() override;
	virtual void OnPressUp() override;

	void ProcessEditInput();

	UISprite * mBackgroundImage;

	Vector2 mFocusDimensions;
	Vector2 mNormalDimensions;

	bool mInEditMode = false;

	bool mEditFadeUp = false;

	std::string mText;
	std::string mDisplayText;

	bool mIsEditable = true;
};

#endif

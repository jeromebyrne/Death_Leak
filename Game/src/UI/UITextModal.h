#ifndef UITEXTMODAL_H
#define UITEXTMODAL_H

#include "uiscreen.h"

class UISprite;

class UITextModal : public UIScreen
{
public:

	UITextModal(string name);
	virtual ~UITextModal(void);

	virtual void Update() override;

	virtual void Initialise() override;

	virtual void LoadContent(ID3D10Device * graphicsdevice) override;

	virtual void Draw(ID3D10Device * device) override;

	void SetLocalizedDescription(const string & desc);

	void SetLocalizedTitle(const string & title);

private:

	// string 
	string mLocalizedTitle;
	string mLocalizedDescription;
	ID3DX10Font* mDescriptionFont = nullptr;
	ID3DX10Font* mTitleFont = nullptr;

	float mDismissButtonCurrentWarmup = 0.0f;

	UISprite * mBackButtonSprite_gamepad = nullptr;
	UISprite * mBackButtonSprite_keyboard = nullptr;
};

#endif

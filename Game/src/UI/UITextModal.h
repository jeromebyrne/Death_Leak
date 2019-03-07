#ifndef UITEXTMODAL_H
#define UITEXTMODAL_H

#include "uiscreen.h"

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

private:

	string mLocalizedDescription;
	ID3DX10Font* mDescriptionText = nullptr;

	float mDismissButtonCurrentWarmup = 0.0f;
};

#endif

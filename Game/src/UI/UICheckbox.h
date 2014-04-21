#ifndef UICHECKBOX_H
#define UICHECKBOX_H

#include "uisprite.h"

class UICheckbox : public UISprite
{
private:
	ID3D10ShaderResourceView* mCheckTexture;
	ID3D10ShaderResourceView* mUnCheckTexture;
	ID3D10ShaderResourceView* mUnCheckFocusTexture;
	ID3D10ShaderResourceView* mCheckFocusTexture;

	string mCheckTextureFilename;
	string mUnCheckTextureFilename;
	string mUnCheckFocusTextureFilename;
	string mCheckFocusTextureFilename;
	bool mChecked;

protected:
	list<EventStruct> mCheckActions;
	list<EventStruct> mUnCheckActions;
	virtual void OnPressUp();
public:
	UICheckbox(void);
	virtual ~UICheckbox(void);

	virtual void Release();
	virtual void LoadContent(ID3D10Device * graphicsdevice);
	virtual void XmlRead(TiXmlElement * element);
	virtual void AssignEventAction(string eventType, EventStruct eventStruct);

	virtual void OnFocus() override;
	virtual void OnLoseFocus() override;

	bool IsChecked()
	{
		return mChecked;
	}
};

#endif

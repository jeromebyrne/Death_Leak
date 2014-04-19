#ifndef UIANIMATEDSPRITE_H
#define UIANIMATEDSPRITE_H

#include "uisprite.h"

class UIAnimatedSprite : public UISprite
{
protected:
	vector<ID3D10ShaderResourceView*> mTextures;
	vector<string> mTextureFileNames;
	float mFramesPerSecond;
	unsigned long mLastFrameSwitch;
	int mCurrentFrame;
public:
	UIAnimatedSprite(void);
	virtual ~UIAnimatedSprite(void);

	virtual void Release();
	virtual void LoadContent(ID3D10Device * graphicsdevice);
	virtual void XmlRead(TiXmlElement * element);
	virtual void Update();
};

#endif

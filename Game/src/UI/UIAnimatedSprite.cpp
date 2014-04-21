#include "precompiled.h"
#include "UIAnimatedSprite.h"

UIAnimatedSprite::UIAnimatedSprite(void) : UISprite(),
mLastFrameSwitch(0),
mCurrentFrame(0)
{
}

UIAnimatedSprite::~UIAnimatedSprite(void)
{
}


void UIAnimatedSprite::Release()
{
	UISprite::Release();
}

void UIAnimatedSprite::LoadContent(ID3D10Device * graphicsdevice)
{
	UISprite::LoadContent(graphicsdevice);

	// loop through the texture file paths and create/get the textures
	vector<string>::iterator iter = mTextureFileNames.begin();

	for (; iter != mTextureFileNames.end(); iter++)
	{
		ID3D10ShaderResourceView* texture = TextureManager::Instance()->LoadTexture_ui((char*)(*iter).c_str());
		mTextures.push_back(texture);
	}
	
	// set the current texture to the first frame
	if (mTextureFileNames.size() > 0)
	{
		m_texture = mTextures[0];
		mCurrentFrame = 0;
	}
}

void UIAnimatedSprite::XmlRead(TiXmlElement * element)
{
	UISprite::XmlRead(element);

	mFramesPerSecond = XmlUtilities::ReadAttributeAsFloat(element, "", "fps");

	// get the frames child
	TiXmlElement * framesElement = element->FirstChildElement();

	// loop the attributes (frames image paths) and add to the vector
	TiXmlAttribute * frame = framesElement->FirstAttribute();

	while (frame)
	{
		string filename = frame->Value();
		mTextureFileNames.push_back(filename);
		frame = frame->Next();
	}
}

void UIAnimatedSprite::Update()
{
	// check the last time we animated 
	unsigned long currentTime = timeGetTime();
	if ((mLastFrameSwitch + (1000/mFramesPerSecond)) < currentTime )
	{
		mCurrentFrame++;

		if (mCurrentFrame >= mTextures.size())
		{
			mCurrentFrame = 0;
		}

		m_texture = mTextures[mCurrentFrame];

		mLastFrameSwitch = currentTime;
	}
}
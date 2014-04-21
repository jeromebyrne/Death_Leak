 #include "precompiled.h"
#include "UICheckbox.h"
#include "UIManager.h"

UICheckbox::UICheckbox(void) : UISprite(),
	mChecked(true),
	mUnCheckFocusTexture(nullptr),
	mCheckFocusTexture(nullptr),
	mCheckTexture(nullptr),
	mUnCheckTexture(nullptr)
{

}

UICheckbox::~UICheckbox(void)
{
}

void UICheckbox::OnPressUp()
{
	UISprite::OnPressUp();

	if (mChecked)
	{
		mChecked = false;
		m_texture = mUnCheckFocusTexture;
		
		list<EventStruct>::iterator iter = mUnCheckActions.begin(); 
		for (; iter!= mUnCheckActions.end(); iter++)
		{
			UIManager::Instance()->PushBackEvent(iter->EventName, iter->EventParams);
		}
	}
	else
	{
		mChecked = true;
		m_texture = mCheckFocusTexture;

		list<EventStruct>::iterator iter = mCheckActions.begin(); 
		for (; iter!= mCheckActions.end(); iter++)
		{
			UIManager::Instance()->PushBackEvent(iter->EventName, iter->EventParams);
		}
	}
}

void UICheckbox::Release()
{
	UISprite::Release();
}

void UICheckbox::LoadContent(ID3D10Device * graphicsdevice)
{
	UISprite::LoadContent(graphicsdevice);

	mCheckTexture = TextureManager::Instance()->LoadTexture_ui((char*)mCheckTextureFilename.c_str());
	mUnCheckTexture = TextureManager::Instance()->LoadTexture_ui((char*)mUnCheckTextureFilename.c_str());
	mCheckFocusTexture = TextureManager::Instance()->LoadTexture_ui((char*)mCheckFocusTextureFilename.c_str());
	mUnCheckFocusTexture = TextureManager::Instance()->LoadTexture_ui((char*)mUnCheckFocusTextureFilename.c_str());

	if (mChecked)
	{
		m_texture = mCheckTexture;
	}
	else
	{
		m_texture = mUnCheckTexture;
	}
}

void UICheckbox::XmlRead(TiXmlElement * element)
{
	UISprite::XmlRead(element);

	mCheckTextureFilename = XmlUtilities::ReadAttributeAsString(element, "", "tick_image");
	mUnCheckTextureFilename = XmlUtilities::ReadAttributeAsString(element, "", "image");
	mUnCheckFocusTextureFilename = XmlUtilities::ReadAttributeAsString(element, "", "image_focus");
	mCheckFocusTextureFilename = XmlUtilities::ReadAttributeAsString(element, "", "tick_image_focus");

	mChecked = XmlUtilities::ReadAttributeAsBool(element, "", "checked");
}

void UICheckbox::AssignEventAction(string eventType, EventStruct eventStruct)
{
	UISprite::AssignEventAction(eventType, eventStruct);

	if (eventType == "actions_check")
	{
		mCheckActions.push_back(eventStruct);
	}
	else if(eventType == "actions_uncheck")
	{
		mUnCheckActions.push_back(eventStruct);
	}
}

void UICheckbox::OnFocus()
{
	UISprite::OnFocus();

	if (mChecked)
	{
		m_texture = mCheckFocusTexture;
	}
	else
	{
		m_texture = mUnCheckFocusTexture;
	}
}

void UICheckbox::OnLoseFocus()
{
	if (mChecked)
	{
		m_texture = mCheckTexture;
	}
	else
	{
		m_texture = mUnCheckTexture;
	}
}
	
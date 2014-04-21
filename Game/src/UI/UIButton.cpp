#include "precompiled.h"
#include "UIButton.h"

UIButton::UIButton(void) : UISprite()
{
}

UIButton::~UIButton(void)
{
}

void UIButton::Release()
{
	UIWidget::Release();
}

void UIButton::OnFocus()
{
	UISprite::OnFocus();

	m_texture = mFocusTexture;
}

void UIButton::OnLoseFocus()
{
	m_texture = mNoFocusTexture;
}

void UIButton::OnPressDown()
{
	UISprite::OnPressDown();

	m_texture = mClickTexture;
}

void UIButton::OnPressUp()
{
	UISprite::OnPressUp();

	m_texture = mFocusTexture;
}

void UIButton::LoadContent(ID3D10Device * graphicsdevice)
{
	UISprite::LoadContent(graphicsdevice);

	mNoFocusTexture = TextureManager::Instance()->LoadTexture_ui((char*)mNoFocusTextureFileName.c_str());
	mFocusTexture = TextureManager::Instance()->LoadTexture_ui((char*)mFocusTextureFileName.c_str());
	mClickTexture = TextureManager::Instance()->LoadTexture_ui((char*)mClickTextureFileName.c_str());
	
	// set the current texture
	m_texture = mNoFocusTexture;
}

void UIButton::XmlRead(TiXmlElement * element)
{
	UISprite::XmlRead(element);

	mNoFocusTextureFileName = XmlUtilities::ReadAttributeAsString(element, "", "image");
	mFocusTextureFileName = XmlUtilities::ReadAttributeAsString(element, "", "image_focus");
	mClickTextureFileName = XmlUtilities::ReadAttributeAsString(element, "", "image_click");
}
#include "precompiled.h"
#include "UIButton.h"
#include "UIManager.h"

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

void UIButton::SetButtonAssets(const string & focusFilename,
	const string & noFocusFilename,
	const string & clickFilename)
{
	m_textureFilename = noFocusFilename;
	mNoFocusTextureFileName = noFocusFilename;
	mFocusTextureFileName = focusFilename;
	mClickTextureFileName = clickFilename;
}

void UIButton::Draw(ID3D10Device * graphicsdevice)
{
	UISprite::Draw(graphicsdevice);

	if (mCachedWideString != nullptr && mFont != nullptr)
	{
		mFontColor.a = Alpha();

		auto uiManager = UIManager::Instance();

		RECT bounds;
		
		if (m_bottomLeft.X < 0.0f)
		{
			bounds.left = m_bottomLeft.X + uiManager->GetBaseWidth() * 0.5f;
		}
		else
		{
			bounds.left = std::abs(m_bottomLeft.X) + uiManager->GetBaseWidth() * 0.5f;
		}
		bounds.right = bounds.left + m_dimensions.X;
		
		if (m_bottomLeft.Y < 0.0f)
		{
			bounds.bottom = std::abs(m_bottomLeft.Y) + uiManager->GetBaseHeight() * 0.5f;
		}
		else
		{
			bounds.bottom = uiManager->GetBaseHeight() * 0.5f - m_bottomLeft.Y;
		}
		bounds.top = bounds.bottom - m_dimensions.Y;
		
		bounds.left += 10.0f; // x padding
		bounds.right -= 10.0f; // x padding

		mFont->DrawText(0, mCachedWideString, -1, &bounds, DT_VCENTER, mFontColor);
	}
}

void UIButton::SetButtonText(const string & text, const string & fontName, unsigned int fontSize, const Vector3 & fontColor)
{
	mCachedWideString = Utilities::ConvertCharStringToWcharString(text.c_str());

	if (mFont)
	{
		mFont->Release();
		mFont = nullptr;
	}

	// font setup
	{
		D3DX10_FONT_DESC fd;
		fd.Height = fontSize;
		fd.Width = 0;
		fd.Weight = 0;
		fd.MipLevels = 1;
		fd.Italic = false;
		fd.CharSet = OUT_DEFAULT_PRECIS;
		fd.Quality = DEFAULT_QUALITY;
		fd.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		wcscpy(fd.FaceName, Utilities::ConvertCharStringToWcharString(fontName.c_str()));

		D3DX10CreateFontIndirect(Graphics::GetInstance()->Device(), &fd, &mFont);
	}

	mFontColor.r = fontColor.X;
	mFontColor.g = fontColor.Y;
	mFontColor.b = fontColor.Z;
	mFontColor.a = 1.0f;
}


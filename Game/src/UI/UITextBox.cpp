#include "precompiled.h"
#include "UITextBox.h"
#include "UIManager.h"
#include "Graphics.h"
#include "DrawUtilities.h"
#include "UISprite.h"

UITextBox::UITextBox(void) : UIWidget()
{
}

UITextBox::~UITextBox(void)
{
}

void UITextBox::XmlRead(TiXmlElement * element)
{
	UIWidget::XmlRead(element);
}

void UITextBox::Release()
{
	UIWidget::Release();
}

void UITextBox::Reset()
{
}

void UITextBox::LoadContent(ID3D10Device * graphicsdevice)
{
	UIWidget::LoadContent(graphicsdevice);
}

void UITextBox::Initialise()
{
	mBackgroundImage = new UISprite();
	mBackgroundImage->SetIsProcessInput(false);
	mBackgroundImage->SetBottomLeft(m_bottomLeft);
	mBackgroundImage->SetDimensions(m_dimensions);
	mBackgroundImage->SetImage("Media\\editor\\selection.png");
	mBackgroundImage->SetAlpha(m_alpha);
	mBackgroundImage->SetUseStandardEffect(true);
	
	mBackgroundImage->LoadContent(Graphics::GetInstance()->Device());
	mBackgroundImage->Initialise();

	mNormalDimensions = m_dimensions;
	mFocusDimensions = m_dimensions; 
	mFocusDimensions.Y *= 1.2f;
}

void UITextBox::Draw(ID3D10Device * graphicsdevice)
{
	auto camPos = Camera2D::GetInstance()->Position();

	if (mBackgroundImage)
	{
		mBackgroundImage->Draw(graphicsdevice);
	}

	auto uiManager = UIManager::Instance();

	unsigned xPadding = 5;
	Graphics::GetInstance()->DrawDebugText("testing UITextBox", xPadding + m_bottomLeft.X + uiManager->GetBaseWidth() * 0.5f,
																- m_bottomLeft.Y + (uiManager->GetBaseHeight() * 0.5f) - m_dimensions.Y);
}

void UITextBox::OnFocus()
{
	UIWidget::OnFocus();

	m_dimensions = mFocusDimensions;

	Reset();

	if (mBackgroundImage)
	{
		mBackgroundImage->SetDimensions(mFocusDimensions);
		mBackgroundImage->Reset();
	}
}

void UITextBox::OnLoseFocus()
{
	UIWidget::OnLoseFocus();

	m_dimensions = mNormalDimensions;

	Reset();

	if (mBackgroundImage)
	{
		mBackgroundImage->SetDimensions(mNormalDimensions);
		mBackgroundImage->Reset();
	}
}

void UITextBox::OnPressDown()
{
	LOG_INFO("UITextBox press down");
}

void UITextBox::OnPressUp()
{
	LOG_INFO("UITextBox press up");
}

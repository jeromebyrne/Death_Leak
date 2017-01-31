#include "precompiled.h"
#include "UITextBox.h"
#include "UIManager.h"
#include "Graphics.h"
#include "DrawUtilities.h"
#include "UISprite.h"
#include <conio.h>  
#include <ctype.h> 

static const float kEditFadeAmount = 0.05f;

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
	mBackgroundImage->SetImage(mIsEditable ? "Media\\editor\\text_box_backing.png" : "Media\\editor\\label_backing.png");
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
	float textHeight = 8.0f; // TODO: don't hardcode this
	Graphics::GetInstance()->DrawDebugText(mDisplayText.c_str(), xPadding + m_bottomLeft.X + uiManager->GetBaseWidth() * 0.5f,
																(- m_bottomLeft.Y + (uiManager->GetBaseHeight() * 0.5f) - m_dimensions.Y) - textHeight * 0.25f);
}

void UITextBox::OnFocus()
{
	if (!mIsEditable)
	{
		return;
	}

	if (mInEditMode)
	{
		return;
	}

	if (UIManager::Instance()->IsInKeyboardInputMode())
	{
		return;
	}

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
	if (!mIsEditable)
	{
		return;
	}

	if (mInEditMode)
	{
		return;
	}

	if (UIManager::Instance()->IsInKeyboardInputMode())
	{
		return;
	}

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
	if (!mIsEditable)
	{
		return;
	}

	if (UIManager::Instance()->IsInKeyboardInputMode())
	{
		return;
	}

	LOG_INFO("UITextBox press down");
}

void UITextBox::OnPressUp()
{
	if (!mIsEditable)
	{
		return;
	}

	if (UIManager::Instance()->IsInKeyboardInputMode())
	{
		return;
	}

	LOG_INFO("UITextBox press up");

	mInEditMode = true;

	UIManager::Instance()->SetIsInKeyboardInputMode(true);

	UIManager::Instance()->SetStartingInputInKeyboardInputMode(mText);
}

void UITextBox::Update()
{
	UIWidget::Update();

	if (mInEditMode)
	{
		ProcessEditInput();

		if (mBackgroundImage)
		{
			if (mEditFadeUp)
			{
				mBackgroundImage->SetAlpha(mBackgroundImage->Alpha() + kEditFadeAmount);

				if (mBackgroundImage->Alpha() >= 1.0f)
				{
					mEditFadeUp = false;
				}
			}
			else
			{
				mBackgroundImage->SetAlpha(mBackgroundImage->Alpha() - kEditFadeAmount);

				if (mBackgroundImage->Alpha() <= 0.0f)
				{
					mEditFadeUp = true;
				}
			}
		}
	}
	else
	{
		if (mBackgroundImage)
		{
			mBackgroundImage->SetAlpha(1.0f);
		}
	}
}

void UITextBox::ProcessEditInput()
{
	if (!mIsEditable)
	{
		return;
	}

	if (GetAsyncKeyState(VK_RETURN) < 0)
	{
		// commit the change to text
		mText = mDisplayText;
		mInEditMode = false;
		UIManager::Instance()->SetIsInKeyboardInputMode(false);
		return;
	}
	else if (GetAsyncKeyState(VK_ESCAPE) < 0)
	{
		// cancel the change to text
		mDisplayText = mText;
		mInEditMode = false;
		UIManager::Instance()->SetIsInKeyboardInputMode(false);
		return;
	}

	mDisplayText = UIManager::Instance()->GetKeyboardInputInKeyboardInputMode();
}

void UITextBox::SetText(const std::string text)
{
	mText = text;
	mDisplayText = text;
}

void UITextBox::SetIsEditable(bool editable)
{
	mIsEditable = editable;
}

void UITextBox::UpdateMetadata(const std::string & key, const std::string & value)
{
	mMetadata[key] = value;
}

std::string UITextBox::GetMetadataValue(const std::string & key)
{
	return mMetadata[key];
}
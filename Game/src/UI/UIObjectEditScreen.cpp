#include "precompiled.h"
#include "UIObjectEditScreen.h"
#include "UITextBox.h"
#include "UIManager.h"

UIObjectEditScreen::UIObjectEditScreen(string name) :
	UIScreen(name)
{
}

UIObjectEditScreen::~UIObjectEditScreen(void)
{
}

void UIObjectEditScreen::Update()
{
	UIScreen::Update();
}

void UIObjectEditScreen::Initialise()
{
	UIScreen::Initialise();
}

void UIObjectEditScreen::SetObjectToEdit(GameObject * object)
{
	GAME_ASSERT(object != 0);

	mCurrentObject = object;

	removeTexBoxes();

	addTextBoxesForCurrentObject();
}

void UIObjectEditScreen::addTextBoxesForCurrentObject()
{
	if (!mCurrentObject)
	{
		return;
	}

	// TODO: we only want to save what has changed
	// cloned XML does not get changed when we use regular editor to modify objects
	auto xmlNode = mCurrentObject->GetClonedXml();

	float xPadding = 20.0f;
	float yPadding = 50.0f;
	float heightPadding = 25;
	float startX = -(UIManager::Instance()->GetBaseWidth() * 0.5f);
	float startY = (UIManager::Instance()->GetBaseHeight() * 0.5f);

	int textBoxCount = 8000;
	for (int i = 0; i < 40; ++i)
	{
		UITextBox * textBox = new UITextBox();
		textBox->SetBottomLeft(Vector2(xPadding + startX, (startY - yPadding) - (heightPadding * i)));
		textBox->SetDimensions(Vector2(300, 15));
		textBox->SetIsProcessInput(true);

		textBox->LoadContent(Graphics::GetInstance()->Device());
		textBox->Initialise();

		std::string mapKeyPrefix = Utilities::ConvertDoubleToString(textBoxCount);
		m_widgetMap[mapKeyPrefix + "_textbox"] = textBox;

		++textBoxCount;
	}
}

void UIObjectEditScreen::removeTexBoxes()
{
	for (const auto & kvp : mTextBoxMap)
	{
		m_widgetMap.erase(kvp.first);
	}

	for (const auto & kvp : mTextBoxMap)
	{
		delete kvp.second;
	}

	mTextBoxMap.clear();
}

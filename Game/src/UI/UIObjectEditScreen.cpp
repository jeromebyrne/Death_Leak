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

	float startX = -(UIManager::Instance()->GetBaseWidth() * 0.5f);
	float startY = (UIManager::Instance()->GetBaseHeight() * 0.5f);

	unsigned int textBoxCount = 8000;

	int rowCount = 0;

	addRow(xmlNode->ToElement(), startX, startY, textBoxCount, rowCount);

	++rowCount;

	auto childElement = xmlNode->FirstChildElement();

	while (childElement != nullptr)
	{
		addRow(childElement, startX, startY, textBoxCount, rowCount);

		childElement = childElement->NextSiblingElement();

		++rowCount;
	}
}

void UIObjectEditScreen::addRow(TiXmlElement * xmlElement, float startX, float startY, unsigned int & textBoxCountOut, unsigned int rowCount)
{
	float heightPadding = 25;
	float xPadding = 8.0f;
	float yPadding = 30.0f;

	float nextXOffset = startX + xPadding;

	// element
	std::string elementName = xmlElement->Value();
	{
		float textBoxWidth = strlen(elementName.c_str()) * 7.5;

		if (textBoxWidth < 15.0f) { textBoxWidth = 15.0f; }

		UITextBox * textBox = new UITextBox();
		float posX = nextXOffset;
		textBox->SetBottomLeft(Vector2(nextXOffset, (startY - yPadding) - (heightPadding * rowCount)));
		textBox->SetDimensions(Vector2(textBoxWidth, 15));
		textBox->SetIsProcessInput(false);
		textBox->SetIsEditable(false);
		textBox->SetText(elementName);

		nextXOffset += (5 + textBoxWidth);

		textBox->LoadContent(Graphics::GetInstance()->Device());
		textBox->Initialise();

		std::string mapKeyPrefix = Utilities::ConvertDoubleToString(textBoxCountOut);
		m_widgetMap[mapKeyPrefix + "_textbox"] = textBox;

		++textBoxCountOut;
	}

	for (auto attr = xmlElement->FirstAttribute(); attr != nullptr; attr = attr->Next())
	{
		// property key
		{
			float textBoxWidth = strlen(attr->Name()) * 8;

			if (textBoxWidth < 15.0f) { textBoxWidth = 15.0f; }

			// MASSIVE HACK:
			if (elementName == "foliage_sway_props")
			{
				// prevent going off screen
				textBoxWidth = 110.0f;
			}

			UITextBox * textBox = new UITextBox();
			float posX = nextXOffset;
			textBox->SetBottomLeft(Vector2(nextXOffset, (startY - yPadding) - (heightPadding * rowCount)));
			textBox->SetDimensions(Vector2(textBoxWidth, 15));
			textBox->SetIsProcessInput(false);
			textBox->SetIsEditable(false);
			textBox->SetText(attr->Name());

			nextXOffset += (5 + textBoxWidth);

			textBox->LoadContent(Graphics::GetInstance()->Device());
			textBox->Initialise();

			std::string mapKeyPrefix = Utilities::ConvertDoubleToString(textBoxCountOut);
			m_widgetMap[mapKeyPrefix + "_textbox"] = textBox;

			++textBoxCountOut;
		}

		// property value
		{
			float textBoxWidth = strlen(attr->Value()) * 8;

			if (textBoxWidth < 20.0f) { textBoxWidth = 20.0f; }

			UITextBox * textBox = new UITextBox();
			float posX = nextXOffset;
			textBox->SetBottomLeft(Vector2(nextXOffset, (startY - yPadding) - (heightPadding * rowCount)));
			textBox->SetDimensions(Vector2(textBoxWidth, 15));
			textBox->SetIsProcessInput(true);
			textBox->SetText(attr->Value());

			nextXOffset += (xPadding + textBoxWidth);

			textBox->LoadContent(Graphics::GetInstance()->Device());
			textBox->Initialise();

			std::string mapKeyPrefix = Utilities::ConvertDoubleToString(textBoxCountOut);
			m_widgetMap[mapKeyPrefix + "_textbox"] = textBox;

			++textBoxCountOut;
		}
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

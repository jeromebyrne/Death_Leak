#include "precompiled.h"
#include "UIObjectEditScreen.h"
#include "UITextBox.h"
#include "UIManager.h"
#include "GameObjectManager.h"
#include "LevelEditor.h"
#include "Game.h"
#include "UIWidget.h"

static const unsigned int kTextBoxStartIndex = 8000;

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

	if (GetAsyncKeyState('O') < 0)
	{
		// just testing
		ApplyChanges();
	}
}

void UIObjectEditScreen::Initialise()
{
	UIScreen::Initialise();
}

void UIObjectEditScreen::SetObjectToEdit(GameObject * object)
{
	GAME_ASSERT(object != 0);

	clearData();

	mCurrentObject = object;

	AddTextBoxesForObject(mCurrentObject);
}

void UIObjectEditScreen::AddTextBoxesForObject(GameObject * object)
{
	if (!object)
	{
		return;
	}

	auto xmlElement = GameObjectManager::Instance()->ConvertObjectToXmlElement(object);

	float startX = -(UIManager::Instance()->GetBaseWidth() * 0.5f);
	float startY = (UIManager::Instance()->GetBaseHeight() * 0.5f);

	unsigned int textBoxCount = kTextBoxStartIndex;

	int rowCount = 0;

	AddRow(xmlElement, startX, startY, textBoxCount, rowCount);

	++rowCount;

	auto childElement = xmlElement->FirstChildElement();

	while (childElement != nullptr)
	{
		AddRow(childElement, startX, startY, textBoxCount, rowCount);

		childElement = childElement->NextSiblingElement();

		++rowCount;
	}
}

void UIObjectEditScreen::AddRow(TiXmlElement * xmlElement, float startX, float startY, unsigned int & textBoxCountOut, unsigned int rowCount)
{
	float heightPadding = 25;
	float xPadding = 8.0f;
	float yPadding = 30.0f;

	float nextXOffset = startX + xPadding;

	std::map<std::string, std::string> propertyMap;

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
		std::string key = attr->Name();
		std::string value = attr->Value();

		// property key
		{
			float textBoxWidth = strlen(key.c_str()) * 8;

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
			textBox->SetText(key);

			nextXOffset += (5 + textBoxWidth);

			textBox->LoadContent(Graphics::GetInstance()->Device());
			textBox->Initialise();

			std::string mapKeyPrefix = Utilities::ConvertDoubleToString(textBoxCountOut);
			m_widgetMap[mapKeyPrefix + "_textbox"] = textBox;

			++textBoxCountOut;
		}

		// property value
		{
			float textBoxWidth = strlen(value.c_str()) * 8;

			if (textBoxWidth < 20.0f) { textBoxWidth = 20.0f; }

			UITextBox * textBox = new UITextBox();
			float posX = nextXOffset;
			textBox->SetBottomLeft(Vector2(nextXOffset, (startY - yPadding) - (heightPadding * rowCount)));
			textBox->SetDimensions(Vector2(textBoxWidth, 15));
			textBox->SetIsProcessInput(true);
			textBox->SetText(value);

			nextXOffset += (xPadding + textBoxWidth);

			textBox->LoadContent(Graphics::GetInstance()->Device());
			textBox->Initialise();

			std::string mapKeyPrefix = Utilities::ConvertDoubleToString(textBoxCountOut);
			m_widgetMap[mapKeyPrefix + "_textbox"] = textBox;

			propertyMap[key] = value;

			textBox->UpdateMetadata("element_key", elementName);
			textBox->UpdateMetadata("attribute_key", key);

			++textBoxCountOut;
		}
	}

	mObjectProperties.push_back(std::pair<std::string, std::map<std::string, std::string>>(elementName, propertyMap));
}

TiXmlElement * UIObjectEditScreen::ConvertPropertiesToXmlElement()
{
	TiXmlElement * parentElement = nullptr;
	
	unsigned count = 0;
	for (const auto & kvp : mObjectProperties)
	{
		if (count == 0)
		{
			parentElement = new TiXmlElement(kvp.first.c_str());

			for (const auto & attribute : kvp.second)
			{
				parentElement->SetAttribute(attribute.first.c_str(), attribute.second.c_str());
			}
		}
		else
		{
			TiXmlElement * childElement = new TiXmlElement(kvp.first.c_str());

			for (const auto & attribute : kvp.second)
			{
				childElement->SetAttribute(attribute.first.c_str(), attribute.second.c_str());
			}

			parentElement->LinkEndChild(childElement);
		}

		++count;
	}

	return parentElement;
}

void UIObjectEditScreen::ApplyChanges()
{
	if (mCurrentObject == nullptr)
	{
		return;
	}

	UpdateObjectProperties();

	auto xmlElement = ConvertPropertiesToXmlElement();

	auto gameObjectManager = GameObjectManager::Instance();

	int objectId = mCurrentObject->ID();

	gameObjectManager->RemoveGameObject(mCurrentObject, false);
	mCurrentObject = nullptr;

	auto editedObject = gameObjectManager->CreateObject(xmlElement, std::vector<unsigned int>());

	GAME_ASSERT(editedObject != 0);

	if (editedObject)
	{
		gameObjectManager->AddGameObjectViaLevelEditor(editedObject);
		editedObject->SetID(objectId);
		editedObject->Update(0);
		SetObjectToEdit(editedObject);
		//UIManager::Instance()->DismissObjectEditor();
	}

	ResetFocusedWidget();
}

void UIObjectEditScreen::clearData()
{
	std::list<std::string> textBoxKeys;

	for (const auto & kvp : m_widgetMap)
	{
		UITextBox * textBox = dynamic_cast<UITextBox*>(kvp.second);

		if (textBox)
		{
			textBoxKeys.push_back(kvp.first);
		}
	}

	for (const auto & key : textBoxKeys)
	{
		m_widgetMap[key]->Release();
		delete m_widgetMap[key];
		m_widgetMap.erase(key);
	}
}

void UIObjectEditScreen::UpdateObjectProperties()
{
	for (const auto & kvp : m_widgetMap)
	{
		UITextBox * textBox = dynamic_cast<UITextBox*>(kvp.second);

		if (textBox == nullptr)
		{
			continue;
		}

		std::string elementKey = textBox->GetMetadataValue("element_key");
		std::string attributeKey = textBox->GetMetadataValue("attribute_key");
		if (attributeKey.empty() || elementKey.empty())
		{
			continue;
		}

		for (auto & kvp : mObjectProperties)
		{
			if (kvp.first == elementKey)
			{
				kvp.second[attributeKey] = textBox->GetText();
			}
		}
	}
}
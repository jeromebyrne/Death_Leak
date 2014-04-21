#include "precompiled.h"
#include "UIWidget.h"
#include "UIManager.h"
#include "AudioManager.h"

UIWidget::UIWidget(void):
m_processInput(true),
m_visible(true),
m_bottomLeft(0,0), 
m_dimensions(200,200),
m_alpha(1.0f),
mCurrentlyInFocus(false)
{
}

UIWidget::~UIWidget(void)
{
}

void UIWidget::OnFocus()
{
	if (m_processInput)
	{
		AudioManager::Instance()->PlaySoundEffect("UI\\default_onfocus.wav", false);
		mCurrentlyInFocus = true;
	}
}

void UIWidget::OnLoseFocus()
{
	mCurrentlyInFocus = false;
}

void UIWidget::XmlRead(TiXmlElement * element)
{
	m_processInput = XmlUtilities::ReadAttributeAsBool(element, "", "process_input");
	m_bottomLeft.X = XmlUtilities::ReadAttributeAsFloat(element, "", "posX");
	m_bottomLeft.Y = XmlUtilities::ReadAttributeAsFloat(element, "", "posY");
	m_dimensions.X = XmlUtilities::ReadAttributeAsFloat(element, "", "width");
	m_dimensions.Y = XmlUtilities::ReadAttributeAsFloat(element, "", "height");
	
	TiXmlElement * action_group = element->FirstChildElement();

	while (action_group)
	{
		string action_group_name = action_group->Value();

		// loop through the action elements 
			TiXmlElement * action_element = action_group->FirstChildElement();

			while (action_element)
			{
				// we have an action
				string name = action_element->Value();

				// get the list of parameters
				TiXmlAttribute * attribute = action_element->FirstAttribute();
				list<string> params;

				while (attribute)
				{
					params.push_back(attribute->Value());
					attribute = attribute->Next();
				}

				EventStruct eventStruct;
				eventStruct.EventName = name;
				eventStruct.EventParams = params;
				
				AssignEventAction(action_group_name, eventStruct);
				
				action_element = action_element->NextSiblingElement();
			}
		action_group = action_group->NextSiblingElement();
	}
}

void UIWidget::AssignEventAction(string eventType, EventStruct eventStruct)
{
	if (eventType == "actionspressup")
	{
		m_pressUpActions.push_back(eventStruct);
	}
	else if(eventType == "actionspressdown")
	{
		m_pressDownActions.push_back(eventStruct);
	}
}

void UIWidget::Initialise()
{
}

void UIWidget::LoadContent(ID3D10Device * graphicsdevice)
{
}	

void UIWidget::Update()
{

}

void UIWidget::Draw(ID3D10Device * device)
{

}

void UIWidget::Release()
{

}

void UIWidget::OnPressDown()
{
	list<EventStruct>::iterator iter = m_pressDownActions.begin();

	for (; iter != m_pressDownActions.end(); iter++)
	{
		UIManager::Instance()->PushBackEvent(iter->EventName, iter->EventParams);
	}
}

void UIWidget::OnPressUp()
{
	list<EventStruct>::iterator iter = m_pressUpActions.begin();

	for (; iter != m_pressUpActions.end(); iter++)
	{
		UIManager::Instance()->PushBackEvent(iter->EventName, iter->EventParams);
	}
}

void UIWidget::HandleEvent(enum UIEventTypesEnum eventType)
{
	switch(eventType)
	{
		case UI_PRESS_DOWN:
			{
				OnPressDown();
				break;
			}
		case UI_PRESS_UP:
			{
				OnPressUp();
				break;
			}
		default: break;
	};
}

void UIWidget::Scale(float x, float y)
{
	m_bottomLeft.X *= x;
	m_bottomLeft.Y *= y;

	m_dimensions.X *= x;
	m_dimensions.Y *= y;

	Reset();
}


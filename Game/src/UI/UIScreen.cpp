#include "precompiled.h"
#include "UIScreen.h"
#include "UIButton.h"
#include "UISprite.h"
#include "UIManager.h"
#include "DXWindow.h"
#include "UIAnimatedSprite.h"
#include "UICheckbox.h"
#include "UIMeter.h"
#include "UITextBox.h"
#include "gamepad.h"
#include "Game.h"

UIScreen::UIScreen(string name) : 
	m_currentWidgetInFocus(0),
	m_lastMousePos(0,0),
	m_name(name),
	mShowCursor(true)
{
}

UIScreen::~UIScreen(void)
{
}

void UIScreen::ApplyAlpha(float newAlpha)
{
	map<string, UIWidget*>::iterator iter = m_widgetMap.begin();

	for(; iter != m_widgetMap.end(); iter++)
	{
		(*iter).second->SetAlpha(newAlpha);
	}
}

void UIScreen::ScaleScreen(float scaleFactorX, float scaleFactorY)
{
	map<string, UIWidget*>::iterator iter = m_widgetMap.begin();

	for(; iter != m_widgetMap.end(); iter++)
	{
		(*iter).second->Scale(scaleFactorX, scaleFactorY);
	}
}

void UIScreen::XmlRead(TiXmlElement * root)
{
	bool attribExists = XmlUtilities::AttributeExists(root, "", "show_cursor");

	if (attribExists)
	{
		mShowCursor = XmlUtilities::ReadAttributeAsBool(root, "", "show_cursor");
	}

	// loop through all of the widgets and add them to the widget map
	TiXmlElement * child = root->FirstChildElement();

	while (child)
	{
		// get the widget type
		const char * type = child->Value();
		Utilities::ToLower((char*)type);
		
		// create the widget 
		UIWidget * widget = CreateWidget(type);

		// get the name of the widget
		string name = XmlUtilities::ReadAttributeAsString(child, "", "name");

		// set the widget name
		widget->SetName(name);
		
		// add to the widget map
		m_widgetMap[name] = widget;

		// widget reads in its own content
		widget->XmlRead(child);
			
		// loop to the next widget
		child = child->NextSiblingElement();
	}
}

UIWidget * UIScreen::CreateWidget(const char * type)
{
	if (strcmp(type, "sprite") == 0)
	{
		UISprite * sprite = new UISprite();
		return sprite;
	}
	else if (strcmp(type, "button") == 0)
	{
		UIButton * button = new UIButton();
		return button;
	}
	else if (strcmp(type, "textbox") == 0)
	{
		UITextBox * textBox = new UITextBox();
		return textBox;
	}
	else if (strcmp(type, "animatedsprite") == 0)
	{
		UIAnimatedSprite * animation = new UIAnimatedSprite();
		return animation;
	}
	else if (strcmp(type, "checkbox") == 0)
	{
		UICheckbox * checkbox = new UICheckbox();
		return checkbox;
	}
	else if (strcmp(type, "meter") == 0)
	{
		UIMeter * meter = new UIMeter();
		return meter;
	}

	return 0;
}

void UIScreen::Release()
{
	// loop through our widgets and release
	map<string, UIWidget*>::const_iterator current = m_widgetMap.begin();
	for(;current != m_widgetMap.end(); current++)
	{
		UIWidget * widget = (*current).second;
		if(widget)
		{
			widget->Release();
			widget = 0;
		}
	}
}

void UIScreen::Update()
{
	// loop through our widgets and update
	map<string, UIWidget*>::const_iterator current = m_widgetMap.begin();
	for(;current != m_widgetMap.end(); current++)
	{
		UIWidget * widget = (*current).second;
		widget->Update();
	}

	// process input on the screen
	if (GetForegroundWindow() == DXWindow::GetInstance()->Hwnd())
	{
		if (m_name == "level_select_edit" || 
			m_name == "level_select")
		{
			ProcessCursorInput();
		}
		else
		{
			if (GamePad::GetPad1() && GamePad::GetPad1()->IsConnected() &&
				!UIManager::Instance()->IsObjectEditorDisplaying())
			{
				ProcessGamePad();
			}
			else
			{
				ProcessCursorInput();
			}
		}
	}

	// HACK: here to enable disable gamepad warning
	if (m_name == "mainmenu")
	{
		if (!GamePad::GetPad1() || !GamePad::GetPad1()->IsConnected())
		{
			// enable gamepad warning
			// get the gamepad warning widget
			UIWidget * widget = m_widgetMap["8_gamepad_warning"]; // warning, crash potential

			widget->SetVisible(true);
		}
		else
		{
			// disable gamepad warning
			UIWidget * widget = m_widgetMap["8_gamepad_warning"]; // warning, crash potential

			widget->SetVisible(false);
		}
	}
}

void UIScreen::Draw(ID3D10Device * device)
{
	// loop through our widgets and draw
	map<string, UIWidget*>::const_iterator current = m_widgetMap.begin();
	for(;current != m_widgetMap.end(); current++)
	{
		UIWidget * widget = (*current).second;
		if(widget->IsVisible())
		{
			widget->Draw(device);
		}
	}
}

void UIScreen::Initialise()
{
	// loop through our widgets and draw
	for (const auto & w : m_widgetMap)
	{
		w.second->Initialise();
	}
}

void UIScreen::LoadContent(ID3D10Device * graphicsdevice)
{
	for (const auto & w : m_widgetMap)
	{
		w.second->LoadContent(graphicsdevice);
	}
}

void UIScreen::ProcessKeyStrokes()
{
}

void UIScreen::ProcessCursorInput()
{
	// get the mouse position
	POINT currentMouse;
	GetCursorPos(&currentMouse);
	ScreenToClient(DXWindow::GetInstance()->Hwnd(), &currentMouse);

	// translate mouse coords to UI coords
	Vector2 mouseUICoords = UIManager::Instance()->GetPointInUICoords(currentMouse.x, currentMouse.y);

	// only process for mouse if mouse has changed
	if(mouseUICoords.X != m_lastMousePos.X && mouseUICoords.Y != m_lastMousePos.Y)
	{		
		bool focusedOnSomething = false;
		bool widgetWasDifferentOnLastFrame = false;
	
		for(const auto & kvp : m_widgetMap)
		{
			UIWidget * widget = kvp.second;

			Vector2 widget_top_left = widget->BottomLeft();
			widget_top_left.Y += widget->Dimensions().Y;

			bool mouseCollision = IsPointWithinBounds(mouseUICoords.X, mouseUICoords.Y, widget_top_left, widget->Dimensions());
			if(mouseCollision)
			{
				focusedOnSomething = true;
				if(widget != m_currentWidgetInFocus) 
				{
					if (widget->IsProcessInput())
					{
						widgetWasDifferentOnLastFrame = true;

						if(m_currentWidgetInFocus != 0)
						{
							// lost focus on the old widget
							m_currentWidgetInFocus->OnLoseFocus();
						}

						// just focused on this
						widget->OnFocus(); 
						m_currentWidgetInFocus = widget;

						break; // we found a valid widget so break
					}
					else
					{
						// its a valid widget but it doesnt accept input so keep going through the widget loop
						focusedOnSomething = false;
					}
				}
				
				// break; // only one widget should be in focus so let's break
			} // end if
		}// end for

		if (!focusedOnSomething)
		{
			if(m_currentWidgetInFocus != nullptr)
			{
				// lost focus on the old widget
				m_currentWidgetInFocus->OnLoseFocus();
			}
			m_currentWidgetInFocus = nullptr;
		}

		// now check if we are left clicking
		else
		{
			static bool leftMouseDown = false;
			static UIWidget * widget_pressed_on = 0; // the widget we initially pressed down on

			if(GetAsyncKeyState(VK_LBUTTON) < 0 && !leftMouseDown && !widgetWasDifferentOnLastFrame)
			{
				m_currentWidgetInFocus->HandleEvent(UI_PRESS_DOWN);
				widget_pressed_on = m_currentWidgetInFocus;
				leftMouseDown = true;
			}
			else if(leftMouseDown && !(GetAsyncKeyState(VK_LBUTTON) < 0))
			{
				if ((widget_pressed_on != 0 && widget_pressed_on == m_currentWidgetInFocus))
				{
					// we just released the button
					m_currentWidgetInFocus->HandleEvent(UI_PRESS_UP);
				}
				widget_pressed_on = 0;
				leftMouseDown = false;
			}
		}

	}
}

void UIScreen::ProcessGamePad()
{
	GamePad * gamepad = GamePad::GetPad1();

	if (gamepad && gamepad->IsConnected())
	{
		/// loop through our widgets and check if we are mousing over anything
		map<string, UIWidget*>::const_iterator current = m_widgetMap.begin();
	
		// something must always be in focus when using the gamepad
		// so make sure something is in focus
		for(;current != m_widgetMap.end(); current++)
		{
			UIWidget * widget = (*current).second;

			if (widget->IsProcessInput())
			{
				if(m_currentWidgetInFocus == 0)
				{
					m_currentWidgetInFocus = widget;
					m_currentWidgetInFocus->OnFocus();
				}
				
			}
		}// end for
		
		
		if (m_currentWidgetInFocus)
		{
			// check if pressing confirm button
			XINPUT_STATE pad_state = gamepad->GetState();
			static bool pressing_confirm = false;

			if (pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_A)
			{
				// TODO: don't constantly call this
				//m_currentWidgetInFocus->HandleEvent(UI_PRESS_DOWN);
				pressing_confirm = true;
			}
			else if (pressing_confirm)
			{
				// released
				m_currentWidgetInFocus->HandleEvent(UI_PRESS_DOWN);
				m_currentWidgetInFocus->HandleEvent(UI_PRESS_UP);
				pressing_confirm = false;
			}

			static float last_time_tilting_lthumb = 0;
			float current_time = Timing::Instance()->GetTotalTimeSeconds();
			float delay = 0.3f;

			if (pad_state.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				if (last_time_tilting_lthumb + delay < current_time)
				{
					// go down the widget list
					map<string, UIWidget*>::iterator current = m_widgetMap.find(m_currentWidgetInFocus->Name());
					for(;current != m_widgetMap.end(); ++current)
					{
						UIWidget * widget = (*current).second;
						if (widget == m_currentWidgetInFocus)
						{
							continue;
						}
						if (widget->IsProcessInput())
						{
							m_currentWidgetInFocus->OnLoseFocus(); // lose focus on the previous widget
							m_currentWidgetInFocus = widget;
							m_currentWidgetInFocus->OnFocus();
							last_time_tilting_lthumb = current_time;
							break;
						}
					}// end for
				}
			}
			else if (pad_state.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				if (last_time_tilting_lthumb + delay < current_time)
				{
					// go up the widget list
					map<string, UIWidget*>::iterator current = m_widgetMap.find(m_currentWidgetInFocus->Name());
					for(;current != m_widgetMap.begin(); --current)
					{
						UIWidget * widget = (*current).second;
						if (widget == m_currentWidgetInFocus)
						{
							continue;
						}
						if (widget->IsProcessInput())
						{
							m_currentWidgetInFocus->OnLoseFocus(); // lose focus on the previous widget
							m_currentWidgetInFocus = widget;
							m_currentWidgetInFocus->OnFocus();
							last_time_tilting_lthumb = current_time;
							break;
						}
					}// end for
				}
			}
			else
			{
				last_time_tilting_lthumb = 0;
			}
		}
	}
}

bool UIScreen::IsPointWithinBounds(float pointx, float pointy, Vector2 topLeftBounds, Vector2 boundsDimensions)
{
	float left = topLeftBounds.X;
	float right = topLeftBounds.X + boundsDimensions.X;
	float bottom = topLeftBounds.Y - boundsDimensions.Y;
	float top = topLeftBounds.Y;

	if(pointx > left && pointx < right && pointy > bottom && pointy < top)
	{
		return true;
	}

	return false;
}
	

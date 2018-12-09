#include "precompiled.h"
#include "UIManager.h"
#include "Graphics.h"
#include "AudioManager.h"
#include "Game.h"
#include "dxwindow.h"
#include "uigamehudscreen.h"
#include "EffectLightTextureVertexWobble.h"
#include "Settings.h"
#include "StringManager.h"
#include "UISprite.h"
#include "gamepad.h"
#include "UIObjectEditScreen.h"
#include "UILevelSelectScreen.h"
#include "UIQuickPlayScreen.h"
#include "SaveManager.h"

static const int kMaxInteractablesToDraw = 3;
static Vector2 kInteractSpriteDimensions = Vector2(50.0f, 50.0f);

extern void PostDestroyMessage();

UIManager * UIManager::m_instance = 0;

UIManager::UIManager(void):
	m_defaultEffect(nullptr),
	mBaseHeight(1920),
	mBaseWidth(1080),
	mStandardEffect(nullptr),
	mCursorSprite(nullptr)
{
}

UIManager::~UIManager(void)
{
}

void UIManager::Release()
{
	for (const auto & screen : m_allScreens)
	{
		screen.second->Release();
	}

	m_allScreens.clear();

	m_currentScreens.clear();
}

UIManager* UIManager::Instance()
{
	if(m_instance == nullptr)
	{
		m_instance = new UIManager();
	}
	return m_instance;
}

void UIManager::Initialise()
{
	InitActionStringToEnumMap();

	for (const auto & screen : m_allScreens)
	{
		screen.second->Initialise();
	}

	mCursorSprite = CreateCursorSprite();

	CreateInteractableSprites();
}

void UIManager::Update()
{	
	m_defaultEffect->SetTimeVariable(Timing::Instance()->GetTotalTimeSeconds()); 

	bool updateCursor = false;
	for(const auto & screen : m_currentScreens)
	{
		screen->Update();
		updateCursor = screen->GetShowCursor();
	}

	if (updateCursor && mCursorSprite)
	{
		if (!(GamePad::GetPad1() && GamePad::GetPad1()->IsConnected()) ||
			Game::GetIsLevelEditMode())
		{
			POINT currentMouse;
			GetCursorPos(&currentMouse);
			ScreenToClient(DXWindow::GetInstance()->Hwnd(), &currentMouse);

			// translate mouse coords to UI coords
			Vector2 mouseUICoords = GetPointInUICoords(currentMouse.x, currentMouse.y);
		
			mouseUICoords.Y -= mCursorSprite->Dimensions().Y;

			mCursorSprite->SetBottomLeft(mouseUICoords);

			mCursorSprite->RebuildBuffers();
		}
	}
}

void UIManager::HandleEvents()
{
	// handle any pending UI events
	for (const auto & event : mCurrentEventList)
	{
		HandleEvent(event.EventName, event.EventParams);
	}
	mCurrentEventList.clear();
}

void UIManager::LoadContent(Graphics * graphicsSystem)
{
	// create the default UI shader effect
	m_defaultEffect = new EffectLightTextureVertexWobble();
	m_defaultEffect->Load(graphicsSystem->Device());
	m_defaultEffect->SetInputLayout(m_defaultEffect->CurrentTechnique, graphicsSystem->Device(), graphicsSystem->InputDescriptions.POS_TEXCOORD_NORM, 3);

	mStandardEffect = new EffectLightTexture();
	mStandardEffect->Load(graphicsSystem->Device());
	mStandardEffect->SetInputLayout(mStandardEffect->CurrentTechnique, graphicsSystem->Device(), graphicsSystem->InputDescriptions.POS_TEXCOORD_NORM, 3);

	// set the world view projection matrix for the default shader here, it shouldnt change for UI items
	D3DXMATRIX world, view, projection;

	D3DXMatrixIdentity( &world );
	D3DXMatrixIdentity( &view );
	// create orthographic projection
	D3DXMatrixOrthoLH(&projection, graphicsSystem->BackBufferWidth(), graphicsSystem->BackBufferHeight(), 0.1f, 100000.0f);
	
	m_defaultEffect->SetWorldViewProjection(world, view, projection);
	m_defaultEffect->SetWobbleIntensity(4.0f);

	mStandardEffect->SetWorldViewProjection(world, view, projection);

	// load UI
	map<string, UIScreen*>::iterator current = m_allScreens.begin();
	for(; current != m_allScreens.end(); current++)
	{
		current->second->LoadContent(graphicsSystem->Device());
	}
}	

void UIManager::Draw(ID3D10Device * device)
{
	bool drawCursor = false;
	for(const auto & screen : m_currentScreens)
	{
		screen->Draw(device);
		drawCursor = screen->GetShowCursor();
	}

	if (drawCursor && mCursorSprite )
	{
		if (!(GamePad::GetPad1() && GamePad::GetPad1()->IsConnected()) ||
			Game::GetIsLevelEditMode())
		{
			mCursorSprite->Draw(device);
		}
	}

	GameObjectManager * gameObjectManager = GameObjectManager::Instance();

	if (gameObjectManager)
	{
		float worldScale = gameObjectManager->GetCurrentLevelProperties().GetZoomInPercent();
		worldScale = (1.0f + (1.0f - worldScale));

		int iDrawnCount = 0;
		for (const auto & i : mInteractableIconsToDraw)
		{
			if (iDrawnCount >= kMaxInteractablesToDraw)
			{
				break;
			}

			auto iSprite = mInteractableSprites[iDrawnCount];

			Vector2 uiCoords = GetPointInUICoords(i.CurrentScreenPos.X - iSprite->Dimensions().X * 0.5f,
													i.CurrentScreenPos.Y - iSprite->Dimensions().Y * 0.5f);

			uiCoords = uiCoords * worldScale;

			// mInteractableProperties.CurrentScale
			iSprite->SetDimensions(kInteractSpriteDimensions * i.CurrentScale);
			iSprite->SetBottomLeft(uiCoords);
			iSprite->SetAlpha(i.CurrentAlpha);
			iSprite->RebuildBuffers();
			iSprite->Draw(device);

			++iDrawnCount;
		}
	}

	// clear the interactables every frame
	mInteractableIconsToDraw.clear();
}

void UIManager::AddInteractableToDraw(GameObject::InteractableProperties iProp)
{
	mInteractableIconsToDraw.push_back(iProp);
}

UIScreen * UIManager::PushUI(string uiName)
{
	// check that it's not already in the currentUI screen list
	for(const auto & screen : m_currentScreens)
	{
		if (screen->Name() == uiName)
		{
			// already in the current screens so leave
			return screen;
		}
	}

	// it's not in the current screens so let's look in all screens
	for(const auto & kvp : m_allScreens)
	{
		if(kvp.first == uiName)
		{
			m_currentScreens.push_back(kvp.second);
			// found it an pushed onto current list
			return kvp.second;
		}
	}

	// never found it, wont be pushed onto current list
	return nullptr;
}

void UIManager::PopUI(string uiName)
{
	// look for it on the current screens list
	UIScreen * screen_found = nullptr;

	for(const auto & screen : m_currentScreens)
	{
		if (screen->Name() == uiName)
		{
			screen_found = screen;
			break;
		}
	}

	if (screen_found)
	{
		m_currentScreens.remove(screen_found);
	}
}

void UIManager::XmlRead(const char * uiRootPath)
{
	// this root xml just contains the names of the individial xml UI screens
	XmlDocument root_doc;
	root_doc.Load(uiRootPath);

	// get the base width and height of the UI
	mBaseWidth = XmlUtilities::ReadAttributeAsFloat(root_doc.GetRoot(), "", "base_width");
	mBaseHeight = XmlUtilities::ReadAttributeAsFloat(root_doc.GetRoot(), "", "base_height");

	float scaledWidth = (float)Graphics::GetInstance()->BackBufferWidth() / (float)mBaseWidth;
	float scaledHeight = (float)Graphics::GetInstance()->BackBufferHeight() / (float)mBaseHeight;

	TiXmlElement * ui_screen_xml = root_doc.GetRoot()->FirstChildElement();

	// each child is a ui screen which points to the screens own xml file
	while(ui_screen_xml)
	{
		// get the name and path of the ui screen
		string name		 =	XmlUtilities::ReadAttributeAsString(ui_screen_xml, "", "name");
		string file_path =	XmlUtilities::ReadAttributeAsString(ui_screen_xml, "", "filepath");	

		// create a new uiscreen object
		UIScreen * ui_screen;

		if (name == "game_hud")
		{
			ui_screen= new UIGameHudScreen(name);
		}
		else if (name == "object_editor")
		{
			ui_screen = new UIObjectEditScreen(name);
		}
		else if (name == "level_select")
		{
			ui_screen = new UIQuickPlayScreen(name);
		}
		else if (name == "level_select_edit")
		{
			ui_screen = new UILevelSelectScreen(name);
		}
		else
		{
			ui_screen = new UIScreen(name);
		}

		// load the xml file for this screen and pass the data to the ui screen object
		XmlDocument ui_screen_doc;
		ui_screen_doc.Load(file_path.c_str());
		ui_screen->XmlRead(ui_screen_doc.GetRoot()); // let the ui screen read its own data

		// scale accordingly
		ui_screen->ScaleScreen(scaledWidth, scaledHeight);

		//add to the ui screen map
		m_allScreens[name] = ui_screen;

		// move to the next ui screen
		ui_screen_xml = ui_screen_xml->NextSiblingElement();
	}

	DisplayLaunchUI();
}

void UIManager::DisplayLaunchUI()
{
	list<string> params;

	// just set the current ui screen here - TEMP
	std::string languageSet = SaveManager::GetInstance()->GetLanguageSet();
	if (languageSet.empty())
	{
		params.push_back("language_select");
	}
	else
	{
		// the language was set so tell the strings
		StringManager::GetInstance()->SetLocale(languageSet);
		params.push_back("mainmenu");
	}

	PushBackEvent("pushui", params);
	// AudioManager::Instance()->PlayMusic("weather\\2minutestorm.mp3");
	AudioManager::Instance()->PlayMusic("music\\Ripples.mp3");
}

void UIManager::PushBackEvent(string eventName, list<string> eventParams)
{
	EventStruct eventStruct;
	eventStruct.EventName = eventName;
	eventStruct.EventParams = eventParams;
	mCurrentEventList.push_back(eventStruct);
}

Vector2 UIManager::GetPointInUICoords(float x, float y)
{
	Vector2 ui_coords;

	auto window = DXWindow::GetInstance();
	auto graphics = Graphics::GetInstance();

	float xScale = 1.0f;
	float yScale = 1.0f;

	float windowWidth = window->GetWindowDimensions().X;
	float windowHeight = window->GetWindowDimensions().Y;

	float backBufferWidth = graphics->BackBufferWidth();
	float backBufferHeight = graphics->BackBufferHeight();

	if (windowWidth != backBufferWidth)
	{
		xScale = backBufferWidth / windowWidth;
	}

	if (windowHeight != backBufferHeight)
	{
		yScale = backBufferHeight / windowHeight;
	}

	int halfWidth = DXWindow::GetInstance()->GetWindowDimensions().X * 0.5f;
	int halfHeight = DXWindow::GetInstance()->GetWindowDimensions().Y * 0.5f;

	ui_coords.X = (x - halfWidth) * xScale;
	ui_coords.Y = (halfHeight - y) * yScale;

	return ui_coords;
}

void UIManager::RefreshUI()
{
	Graphics * graphics = Graphics::GetInstance();

	Update();

	// draw our UI
	Draw(graphics->Device());
	
	// swap out the backbuffer
	graphics->SwapBuffers();
}

void UIManager::HandleEvent(string eventName, list<string> params)
{
	int action = m_ActionStringToEnumMap[eventName];

	switch (action)
	{
	case PUSH_UI:
		{
			string ui_name = *(params.begin()); // always only 1 parameter
			PushUI(ui_name); 
			if (ui_name == "mainmenu")
			{
				GameObjectManager::Instance()->SetLevelFreshLaunch(true);
			}
			break;
		}
	case POP_UI:
		{
			string ui_name = *(params.begin()); // always only 1 parameter
			PopUI(ui_name); 
			break;
		}
	case LOAD_LEVEL:
		{
			string level_file = *(params.begin()); // always only 1 parameter

			string savedlevel = SaveManager::GetInstance()->GetLevelLastSavedAt();
			if (!savedlevel.empty())
			{
				// This is not a new game
				level_file = savedlevel;
			}

			Game::SetIsLevelEditMode(false);
			auto inputManager = Game::GetInstance()->GetInputManager();
			inputManager.EnableDebugInfo(false);
			inputManager.EnablePostProcessing(false);
			GameObjectManager::Instance()->LoadObjectsFromFile(level_file.c_str());
			break;
		}
	case PLAY_SOUND_EFFECT:
		{
			list<string>::iterator iter = params.begin();
			string sound_file = (*iter); // first param is the audio file

			// second param is optional and is the loop bool
			bool loop = false;
			iter++;
			if (iter != params.end())
			{
				string boolean = (*iter);
				if ((*iter) == "true") loop = true; 
			}

			AudioManager::Instance()->PlaySoundEffect(sound_file, loop);
			break;
		}
	case PLAY_MUSIC:
		{
			list<string>::iterator iter = params.begin();
			string sound_file = (*iter); // first param is the audio file

			// second param is optional and is the loop bool
			bool loop = false;
			iter++;
			if (iter != params.end())
			{
				string boolean = (*iter);
				if ((*iter) == "true") loop = true; 
			}

			AudioManager::Instance()->PlayMusic(sound_file, loop);
			break;
		}
	case REFRESH_UI:
		{
			RefreshUI();
			break;
		}
	case STOP_ALL_SOUNDS:
		{
			AudioManager::Instance()->StopAllSounds();
			break;
		}
	case PAUSE_GAME:
		{
			Game::PauseGame();
			break;
		}
	case UNPAUSE_GAME:
		{
			Game::UnPauseGame();
			break;
		}
	case DESTROY_LEVEL:
		{
			Game::SetIsLevelEditMode(false);
			Game::GetInstance()->ResetLevelEditor();
			GameObjectManager::Instance()->QuitLevel();
			break;
		}
	case SLEEP:
		{
			list<string>::iterator iter = params.begin();
			int milliseconds = atoi((*iter).c_str());
			Sleep(milliseconds);
		}
	case APPLY_ALPHA:
		{
			list<string>::iterator iter = params.begin();
			string ui = (*iter); 
			iter++;
			float alpha = atof((*iter).c_str());
			m_allScreens[ui]->ApplyAlpha(alpha);
			break;
		}
	case FADE_OUT:
		{
			/*
			list<string>::iterator iter = params.begin();
			int milliseconds = atoi((*iter).c_str());

			// start fading out - we do 10 passes 
			UIScreen * screen = PushUI("fullscreen_fade");
			screen->ApplyAlpha(0.0);
			float count = 0.0f;

			while (count < 1.0f)
			{
				screen->ApplyAlpha(count);
				RefreshUI();
				Sleep(milliseconds * 0.05f); // TODO: this is terrible bleugh, get rid of this crap
				count+= 0.05;
			}

			PopUI("fullscreen_fade");
			*/
			RefreshUI();
			break;
		}
	case FADE_IN:
		{
		/*
			list<string>::iterator iter = params.begin();
			int milliseconds = atoi((*iter).c_str());

			// start fading out - we do 10 passes 
			UIScreen * screen = PushUI("fullscreen_fade");
			screen->ApplyAlpha(1.0);
			float count = 1.0;

			while (count > 0)
			{
				screen->ApplyAlpha(count);
				RefreshUI();
				Sleep(milliseconds/10);
				count-= 0.05;
			}

			PopUI("fullscreen_fade");
			*/
			RefreshUI();
			break;
		}
	case MUTE_SOUND_EFFECTS:
		{
			AudioManager::Instance()->SetSfxEnabled(false);
			Settings::GetInstance()->SetSfxEnabled(false, true);
			break;
		}
	case MUTE_MUSIC:
		{
			AudioManager::Instance()->SetMusicEnabled(false);
			Settings::GetInstance()->SetMusicEnabled(false, true);
			break;
		}
	case UNMUTE_MUSIC:
		{
			AudioManager::Instance()->SetMusicEnabled(false);
			Settings::GetInstance()->SetMusicEnabled(true, true);
			break;
		}
	case UNMUTE_SOUND_EFFECTS:
		{
			AudioManager::Instance()->SetSfxEnabled(true);
			Settings::GetInstance()->SetSfxEnabled(true, true);
			break;
		}
	case QUIT_TO_DESKTOP:
		{
			PostDestroyMessage(); // evil global from program.cpp
			break;
		}
	case LEVEL_EDIT:
		{
			Game::GetInstance()->ResetLevelEditor();
			// only allow level editing if the back buffer is 1920x1080
			// this needs to be fixed at a later time
			if (Graphics::GetInstance()->BackBufferWidth() == 1920 &&
				Graphics::GetInstance()->BackBufferHeight() == 1080)
			{
				list<string>::iterator iter = params.begin();
				const char * level = (*iter).c_str();
				Game::SetIsLevelEditMode(true);
				GameObjectManager::Instance()->LoadObjectsFromFile(level);
			}
			auto inputManager = Game::GetInstance()->GetInputManager();
			inputManager.EnableDebugInfo(true);
			inputManager.EnablePostProcessing(true);
			break;			
		}
	case SET_LANGUAGE:
		{
			list<string>::iterator iter = params.begin();
			std::string lang = (*iter).c_str();
			StringManager::GetInstance()->SetLocale(lang);
			SaveManager::GetInstance()->SetLanguage(lang);
			SaveManager::GetInstance()->WriteSaveFile();
			break;
		}
	case APPLY_OBJECT_EDIT_CHANGES:
	{
		ApplyObjectEditChanges();
		DismissObjectEditor();
		break;
	}

	default:
		break;
	};
}

void UIManager::InitActionStringToEnumMap()
{
	m_ActionStringToEnumMap["pushui"] = PUSH_UI;
	m_ActionStringToEnumMap["popui"] = POP_UI;
	m_ActionStringToEnumMap["loadlevel"] = LOAD_LEVEL;
	m_ActionStringToEnumMap["playsoundeffect"] = PLAY_SOUND_EFFECT;
	m_ActionStringToEnumMap["playmusic"] = PLAY_MUSIC;
	m_ActionStringToEnumMap["refreshui"] = REFRESH_UI;
	m_ActionStringToEnumMap["stopallsounds"] = STOP_ALL_SOUNDS;
	m_ActionStringToEnumMap["pausegame"] = PAUSE_GAME;
	m_ActionStringToEnumMap["unpausegame"] = UNPAUSE_GAME;
	m_ActionStringToEnumMap["destroylevel"] = DESTROY_LEVEL;
	m_ActionStringToEnumMap["sleep"] = SLEEP;
	m_ActionStringToEnumMap["applyalpha"] = APPLY_ALPHA;
	m_ActionStringToEnumMap["fadeout"] = FADE_OUT;
	m_ActionStringToEnumMap["fadein"] = FADE_IN;
	m_ActionStringToEnumMap["mutesoundeffects"] = MUTE_SOUND_EFFECTS;
	m_ActionStringToEnumMap["mutemusic"] = MUTE_MUSIC;
	m_ActionStringToEnumMap["unmutesoundeffects"] = UNMUTE_SOUND_EFFECTS;
	m_ActionStringToEnumMap["unmutemusic"] = UNMUTE_MUSIC;
	m_ActionStringToEnumMap["quittodesktop"] = QUIT_TO_DESKTOP; 
	m_ActionStringToEnumMap["leveledit"] = LEVEL_EDIT; 
	m_ActionStringToEnumMap["set_language"] = SET_LANGUAGE;
	m_ActionStringToEnumMap["apply_object_edit_changes"] = APPLY_OBJECT_EDIT_CHANGES;
}

UISprite * UIManager::CreateCursorSprite()
{
	UISprite * cursorSprite = new UISprite();

	cursorSprite->SetImage("Media\\UI\\cursor.png");

	cursorSprite->SetDimensions(Vector2(28, 42));

	cursorSprite->SetUseStandardEffect(true);

	cursorSprite->Initialise();

	cursorSprite->LoadContent(Graphics::GetInstance()->Device());

	return cursorSprite;
}

void UIManager::CreateInteractableSprites()
{
	mInteractableSprites.reserve(kMaxInteractablesToDraw);

	for (int i = 0; i < kMaxInteractablesToDraw; ++i)
	{
		UISprite * sprite = new UISprite();

		sprite->SetImage("Media\\UI\\gamepad_icons\\x.png");
		sprite->SetDimensions(kInteractSpriteDimensions);
		sprite->SetUseStandardEffect(true);
		sprite->Initialise();
		sprite->LoadContent(Graphics::GetInstance()->Device());

		mInteractableSprites.push_back(sprite);
	}
}

void UIManager::HandleKeyPressInKeyboardInputMode(char character)
{
	if (!mIsInKeyboardInputMode)
	{
		return;
	}

	mKeyboardInput += character;
}

void UIManager::HandleBackspaceInKeyboardInputMode()
{
	if (!mIsInKeyboardInputMode)
	{
		return;
	}

	if (!mKeyboardInput.empty())
	{
		mKeyboardInput.resize(mKeyboardInput.size() - 1);
	}
}

void UIManager::SetIsInKeyboardInputMode(bool value)
{
	mIsInKeyboardInputMode = value;

	if (value == false)
	{
		// clear the string for next time
		mKeyboardInput = "";
	}
}

void UIManager::DisplayObjectEditor(GameObject * gameObject)
{
	mObjectEditorDisplaying = true;

	UIObjectEditScreen * screen = dynamic_cast<UIObjectEditScreen*>(PushUI("object_editor"));

	GAME_ASSERT(screen != 0);

	if (screen)
	{
		screen->SetObjectToEdit(gameObject);
	}
}

void UIManager::DismissObjectEditor()
{
	PopUI("object_editor");

	mObjectEditorDisplaying = false;
}

void UIManager::ApplyObjectEditChanges()
{
	for (const auto & screen : m_currentScreens)
	{
		UIObjectEditScreen * correctScreen = dynamic_cast<UIObjectEditScreen*>(screen);

		if (correctScreen)
		{
			correctScreen->ApplyChanges();
		}
	}
}
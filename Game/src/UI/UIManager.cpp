#include "precompiled.h"
#include "UIManager.h"
#include "Graphics.h"
#include "AudioManager.h"
#include "Game.h"
#include "dxwindow.h"
#include "uigamehudscreen.h"
#include "EffectLightTextureVertexWobble.h"

extern void PostDestroyMessage();

UIManager * UIManager::m_instance = 0;

UIManager::UIManager(void):
	m_defaultEffect(nullptr),
	mBaseHeight(600),
	mBaseWidth(800),
	mStandardEffect(nullptr)
{
}

UIManager::~UIManager(void)
{
}

void UIManager::Release()
{
	list<UIScreen*>::iterator current = m_currentScreens.begin();

	for(;current != m_currentScreens.end(); current++)
	{
		(*current)->Release();
		(*current) = 0;
	}

	// delete the default shader effect here
	if (m_defaultEffect)
	{
		//delete m_defaultEffect;
		//m_defaultEffect = 0;
	}
}

UIManager* UIManager::Instance()
{
	if(m_instance == 0)
	{
		m_instance = new UIManager();
	}
	return m_instance;
}

void UIManager::Initialise()
{
	InitActionStringToEnumMap();

	map<string, UIScreen*>::iterator current = m_allScreens.begin();
	for(; current != m_allScreens.end(); current++)
	{
		current->second->Initialise();
	}
}

void UIManager::Update()
{	
	m_defaultEffect->SetTimeVariable(Timing::Instance()->GetTotalTimeSeconds()); 

	// update the screens
	list<UIScreen*>::iterator current = m_currentScreens.begin();
	for(;current != m_currentScreens.end(); current++)
	{
		(*current)->Update();
	}
}

void UIManager::HandleEvents()
{
	// handle any pending UI events
	list<EventStruct>::iterator eventIter = mCurrentEventList.begin();
	for (; eventIter != mCurrentEventList.end(); eventIter++)
	{
		HandleEvent((*eventIter).EventName, (*eventIter).EventParams);
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
	list<UIScreen*>::iterator current = m_currentScreens.begin();

	for(;current != m_currentScreens.end(); current++)
	{
		(*current)->Draw(device);
	}
}

UIScreen * UIManager::PushUI(string uiName)
{
	// check that it's not already in the currentUI screen list
	list<UIScreen*>::iterator current = m_currentScreens.begin();

	for(;current != m_currentScreens.end(); current++)
	{
		if ((*current)->Name() == uiName)
		{
			// already in the current screens so leave
			return (*current);
		}
	}

	// it's not in the current screens so let's look in all screens
	map<string, UIScreen*>::iterator iter = m_allScreens.begin();
	for(;iter != m_allScreens.end(); iter++)
	{
		if(iter->first == uiName)
		{
			m_currentScreens.push_back(iter->second);
			// found it an pushed onto current list
			return iter->second;
		}
	}

	// never found it, wont be pushed onto current list
	return 0;
}

void UIManager::PopUI(string uiName)
{
	// look for it on the current screens list
	list<UIScreen*>::iterator current = m_currentScreens.begin();
	
	UIScreen * screen_found = 0;

	for(;current != m_currentScreens.end(); current++)
	{
		if ((*current)->Name() == uiName)
		{
			screen_found = (*current);
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

	// just set the current ui screen here - TEMP
	list<string> params;
	params.push_back("mainmenu");
	PushBackEvent("pushui", params);
	// AudioManager::Instance()->PlayMusic("weather\\2minutestorm.mp3");
	AudioManager::Instance()->PlayMusic("mainmenu_theme.mp3");
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

	int halfWidth =  DXWindow::GetInstance()->GetWindowDimensions().X * 0.5f; // Graphics::GetInstance()->BackBufferWidth()/2;
	int halfHeight = DXWindow::GetInstance()->GetWindowDimensions().Y * 0.5f; // Graphics::GetInstance()->BackBufferHeight()/2;

	ui_coords.X = x - halfWidth;
	ui_coords.Y = halfHeight - y;

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
			Game::SetIsLevelEditMode(false);
			GameObjectManager::Instance()->SetShowDebugInfo(false);
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
			GameObjectManager::Instance()->DeleteGameObjects();
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
			list<string>::iterator iter = params.begin();
			int milliseconds = atoi((*iter).c_str());

			// start fading out - we do 10 passes 
			UIScreen * screen = PushUI("fullscreen_fade");
			screen->ApplyAlpha(0.0);
			float count = 0.0;

			while (count < 1)
			{
				screen->ApplyAlpha(count);
				RefreshUI();
				Sleep(milliseconds/10);
				count+= 0.05;
			}

			PopUI("fullscreen_fade");
			break;
		}
	case FADE_IN:
		{
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
			break;
		}
	case MUTE_SOUND_EFFECTS:
		{
			AudioManager::Instance()->MuteSoundEffects();
			break;
		}
	case MUTE_MUSIC:
		{
			AudioManager::Instance()->MuteMusic();
			break;
		}
	case UNMUTE_MUSIC:
		{
			AudioManager::Instance()->UnMuteMusic();
			break;
		}
	case UNMUTE_SOUND_EFFECTS:
		{
			AudioManager::Instance()->UnMuteSoundEffects();
			break;
		}
	case QUIT_TO_DESKTOP:
		{
			PostDestroyMessage(); // evil global from program.cpp
			break;
		}
	case LEVEL_EDIT:
		{
			// only allow level editing if the back buffer is 1920x1080
			// this needs to be fixed at a later time
			if (Graphics::GetInstance()->BackBufferWidth() == 1920 &&
				Graphics::GetInstance()->BackBufferHeight() == 1080)
			{
				list<string>::iterator iter = params.begin();
				const char * level = (*iter).c_str();
				Game::SetIsLevelEditMode(true);
				GameObjectManager::Instance()->SetShowDebugInfo(true);
				GameObjectManager::Instance()->LoadObjectsFromFile(level);
			}
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
}
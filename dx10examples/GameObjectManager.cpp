#include "precompiled.h"
#include "GameObjectManager.h"
#include "sprite.h"
#include "movingsprite.h"
#include "player.h"
#include "parallaxlayer.h"
#include "ParticleSpray.h"
#include "collisionmanager.h"
#include "Projectile.h"
#include "AudioManager.h"
#include "dxwindow.h"
#include "ParticleEmitterManager.h"
#include "UIManager.h"
#include "npc.h"
#include "platform.h"
#include "pathingplatform.h"
#include "fallingplatform.h"
#include "gamepad.h"
#include "NinjaSpawner.h"
#include "NPCManager.h"
#include "WeatherManager.h"
#include "AudioObject.h"
#include "Rabbit.h"
#include "Game.h"
#include "Orb.h"
#include "LevelTrigger.h"
#include "WaterBlock.h"

GameObjectManager* GameObjectManager::m_instance = 0;

GameObjectManager::GameObjectManager(): 
m_camera(),
m_player(0),
m_levelLoaded(false),
mShowDebugInfo(false),
mCamYShouldOffset(false),
mCamYOffset(0),
mSwitchToLevel(false),
mSlowMotionLayer(nullptr)
{
}

GameObjectManager::~GameObjectManager(void)
{
}

void GameObjectManager::OrderDrawable_pushBack(DrawableObject* object)
{
	m_drawableObjects.remove(object); // remove it first, we don't want to add it twice

	float objZ = object->Z();
	list<DrawableObject*>::iterator current = m_drawableObjects.begin();
	bool found = false;
	for (DrawableObject * d : m_drawableObjects)
	{
		float currentObjZ = d->Z();

		if(currentObjZ <= objZ)
		{
			m_drawableObjects.insert(current, object);
			found = true;
			break;
		}

		current++;
	}

	if (!found)
	{
		m_drawableObjects.push_back(object);
	}
}

void GameObjectManager::RemoveGameObject_RunTime(GameObject * object, bool defer)
{
	if (defer)
	{
		m_killList.push_back(object); // add to the kill list
	}
	else
	{
		m_gameObjects.remove(object);

		// try and cast to a solid object
		SolidMovingSprite * sms = dynamic_cast<SolidMovingSprite*>(object);
		if(sms != 0)
		{
			CollisionManager::Instance()->RemoveObject(sms); // remove from list of collidables
			m_drawableObjects.remove(static_cast<DrawableObject*>(sms));
		}
		else
		{
			// try and cast to a drawable
			DrawableObject* d = dynamic_cast<DrawableObject*>(object);
			if(d != 0)
			{
				m_drawableObjects.remove(d);
			}
		}

		if (object->IsUpdateable())
		{
			m_updateableObjects.remove(object);
		}

		delete object;
		object = nullptr;
	}
}

GameObject * GameObjectManager::GetObjectByID(int id)
{
	list<GameObject*>::iterator current = m_gameObjects.begin();

	for(; current!= m_gameObjects.end(); current++)
	{
		if((*current)->ID() == id)
		{
			return (*current);
		}
	}

	return nullptr;
}

// this is to be called before initialise
void GameObjectManager::LoadContent(ID3D10Device * device)
{
	list<DrawableObject*>::iterator current = m_drawableObjects.begin();

	for(; current!= m_drawableObjects.end(); current++)
	{
		(*current)->LoadContent(device);

		// refresh the ui 
		UIManager::Instance()->RefreshUI();
	}
}

void GameObjectManager::Initialise()
{
	list<GameObject*>::iterator current = m_gameObjects.begin();

	for(; current!= m_gameObjects.end(); current++)
	{
		(*current)->Initialise();

		// refresh the UI
		UIManager::Instance()->RefreshUI();
	}

	if (m_player)
	{
		// sync with the players position
		m_camera->FollowObjectsOrigin(m_player);
	}
}

void GameObjectManager::Update(bool paused, float delta)
{
	if (!paused)
	{
		float camX = m_camera->X();
		float camY = m_camera->Y();

		NPCManager::Instance()->Update();

		m_camera->FollowObjectWithOffset(m_player, 0, 150);

		// update the weather
		WeatherManager::GetInstance()->Update(delta);

		list<GameObject*>::iterator current = m_updateableObjects.begin();
		list<GameObject*>::iterator end = m_updateableObjects.end();

		for(; current!= end; current++) 
		{
			// if the object is in the update zone
			if(Utilities::IsObjectInRectangle((*current), camX, camY, m_updateZoneDimensions.X, m_updateZoneDimensions.Y))
			{
				(*current)->Update(delta);
			}
			else if(dynamic_cast<ParallaxLayer *>((*current)) || dynamic_cast<AudioObject *>((*current)) || dynamic_cast<Orb *>((*current))) // OPTIMISE
			{
				(*current)->Update(delta); // always update parralax layers
			}
			else // objects outside the update area
			{
				// if a projectile has gone outsid ethe bounds then just remove it
				if(dynamic_cast<Projectile*>((*current))) // add checks for other projectile class names as needed
				{
					RemoveGameObject_RunTime((*current));
				}
			}
		}
	} // end of if paused

	// kill any objects in the kill list
	for (GameObject * g : m_killList)
	{
		RemoveGameObject_RunTime(g, false);
	}

	m_killList.clear(); // remove our objects

	if (mSwitchToLevel)
	{
		SwitchToLevel(mLevelToSwitch.c_str(), false);
		mLevelToSwitch = "";
		mSwitchToLevel = false;
	}
}

void GameObjectManager::ScaleObjects(float xScale, float yScale)
{
	list<GameObject*>::iterator iter = m_gameObjects.begin();

	for (; iter != m_gameObjects.end(); iter++)
	{
		(*iter)->Scale(xScale, yScale);
	}
}

void GameObjectManager::Draw(ID3D10Device *  device)
{
	for (DrawableObject * d : m_drawableObjects)
	{
		// only draw if object is in view
		if ( d && d->Alpha() > 0 && m_camera->IsObjectInView(d))
		{
			// apply any changes needed
			if(d->IsChangeRequired())
			{
				d->ApplyChange(device);
			}
			d->Draw(device, m_camera);
		}
	}

	if (Timing::Instance()->GetTimeModifier() < 1.0f && mSlowMotionLayer)
	{
		float alpha = mSlowMotionLayer->Alpha();
		if (alpha < 1.0f)
		{
			alpha += 0.1f;
		}
		mSlowMotionLayer->SetAlpha(alpha);
	}
	else if (mSlowMotionLayer)
	{
		float alpha = mSlowMotionLayer->Alpha();
		if (alpha > 0.0f)
		{
			alpha -= 0.1f;
		}
		mSlowMotionLayer->SetAlpha(alpha);
	}

#ifdef DEBUG

	if (mShowDebugInfo)
	{
		for (GameObject * g : m_gameObjects)
		{
			// only draw if we are in view
			if(m_camera->IsObjectInView(g))
			{
				g->DebugDraw(device);
			}
		}
	}
#endif
}

// load game objects via xml file
void GameObjectManager::LoadObjectsFromFile(const char* filename)
{
	Graphics * graphics = Graphics::GetInstance();
	Camera2D * camera = Camera2D::GetInstance();

	// space in which include objects for Updates() and collision detection
	m_updateZoneDimensions.X = 4000;
	m_updateZoneDimensions.Y = 4000;

	CollisionManager::Instance()->SetCollisionArea(m_updateZoneDimensions.X, m_updateZoneDimensions.Y);

	m_camera = camera;

	XmlDocument doc;
	doc.Load(filename);

	TiXmlHandle * hdoc = doc.Handle();
	TiXmlElement * root = hdoc->FirstChildElement().Element();

	// get the audio track for this level
	char * audio_track = XmlUtilities::ReadAttributeAsString(root, "", "audio");
	
	// start playing level music as we load the objects
	AudioManager::Instance()->PlayMusic(audio_track, true); // always loop level music

	TiXmlElement * child = root->FirstChildElement();

	// loop through our game objects
	while(child)
	{
		// create our game object
		CreateObject(child);

		// move to the next game object
		child = child->NextSiblingElement();

		// refresh the UI
		UIManager::Instance()->RefreshUI();
	}

	// initialise all objects
	LoadContent(graphics->Device());
	Initialise();

	// now order the objects by z value
	OrderDrawablesByDepth();

	// get the backbuffer width and height and determine how much we need to scale by
	// base width and height are 1920x1080 (need to not hard code this)
	float bbWidth = Graphics::GetInstance()->BackBufferWidth();
	float bbHeight = Graphics::GetInstance()->BackBufferHeight();

	float scaleX = bbWidth / 1920;
	float scaleY = bbHeight / 1080;

	m_updateZoneDimensions.X * scaleX;
	m_updateZoneDimensions.Y * scaleY;

	ScaleObjects(scaleX, scaleY);

	// update all the objects at least once at the start
	for (auto obj : m_updateableObjects)
	{
		obj->Update(0);
	}

	WeatherManager::GetInstance()->RefreshAssets();

	AddSlowMotionLayer();

	// loaded a level
	m_levelLoaded = true;
}

void GameObjectManager::SwitchToLevel(const char * level, bool defer)
{
	if (defer)
	{
		mSwitchToLevel = true;
		mLevelToSwitch = level;
		return;
	}

	UIManager::Instance()->PopUI("game_hud");
	UIManager::Instance()->PushUI("gameloading");
	UIManager::Instance()->RefreshUI();

	DeleteGameObjects();

	LoadObjectsFromFile(level);

	UIManager::Instance()->PopUI("gameloading");
	UIManager::Instance()->PushUI("game_hud");
}

void GameObjectManager::SaveObjectsToFile(const char* filename)
{
	// loop through the game objects and make sur ethere are no duplicates
	list<GameObject *> checkedList;

	for (auto obj : m_gameObjects)
	{
		for (auto checked : checkedList)
		{
			if (checked == obj)
			{
				// duplicate
				return;
			}
		}

		checkedList.push_back(obj);
	}

	XmlDocument doc;
	TiXmlElement * root = new TiXmlElement( "level" );
	root->SetAttribute("audio", ""); 

	for (list<GameObject*>::iterator iter = m_gameObjects.begin();
		iter != m_gameObjects.end();
		++iter)
	{
		TiXmlElement * objElem = SaveObject((*iter));

		if (objElem)
		{
			root->LinkEndChild(objElem);
		}
	}

	doc.Save(filename, root);
}

TiXmlElement * GameObjectManager::SaveObject(GameObject * object)
{
	string objectType = object->GetTypeName();
	TiXmlElement * element = new TiXmlElement(objectType.c_str()/*object->GetObjectType()*/); // TODO: delete
	object->XmlWrite(element);
	return element;
}

GameObject * GameObjectManager::CreateObject(TiXmlElement * objectElement)
{
	// what type of object is this
	char* gameObjectTypeName = (char*)objectElement->Value();
	Utilities::ToLower(gameObjectTypeName);
	
	// start looking at what object we need to make
	if(strcmp(gameObjectTypeName, "player") == 0)
	{
		if (!m_player) // only create 1 player
		{
			Player * p = new Player();
			p->XmlRead(objectElement);
			m_player = p;
			return p;
		}
	}
	else if (strcmp(gameObjectTypeName, "character") == 0)
	{
		Character * c = new Character();
		c->XmlRead(objectElement);
		return c;
	}
	else if (strcmp(gameObjectTypeName, "npc") == 0)
	{
		NPC * npc = new NPC();
		npc->XmlRead(objectElement);
		return npc;
	}
	else if (strcmp(gameObjectTypeName, "solidmovingsprite") == 0)
	{
		SolidMovingSprite * s = new SolidMovingSprite();
		s->XmlRead(objectElement);
		return s;
	}
	else if (strcmp(gameObjectTypeName, "sprite") == 0)
	{
		Sprite * s = new Sprite();
		s->XmlRead(objectElement);
		return s;
	}
	else if (strcmp(gameObjectTypeName, "movingsprite") == 0)
	{
		MovingSprite * ms = new MovingSprite();
		ms->XmlRead(objectElement);
		return ms;
	}
	else if (strcmp(gameObjectTypeName, "parallaxlayer") == 0)
	{
		ParallaxLayer * pl = new ParallaxLayer(m_camera);
		pl->XmlRead(objectElement);
		return pl;
	}
	else if (strcmp(gameObjectTypeName, "platform") == 0)
	{
		Platform * p = new Platform();
		p->XmlRead(objectElement);
		return p;
	}
	else if (strcmp(gameObjectTypeName, "pathingplatform") == 0)
	{
		PathingPlatform * p = new PathingPlatform();
		p->XmlRead(objectElement);
		return p;
	}
	else if (strcmp(gameObjectTypeName, "fallingplatform") == 0)
	{
		FallingPlatform * fp = new FallingPlatform();
		fp->XmlRead(objectElement);
		return fp;
	}
	else if(strcmp(gameObjectTypeName, "particlespray") == 0)
	{
		return ReadParticleSpray(objectElement);
	}
	else if (strcmp(gameObjectTypeName, "audioobject") == 0)
	{
		AudioObject * audioObject = new AudioObject();
		audioObject->XmlRead(objectElement);
		return audioObject;
	}
	else if (strcmp(gameObjectTypeName, "rabbit") == 0)
	{
		Rabbit * rabbit = new Rabbit();
		rabbit->XmlRead(objectElement);
		return rabbit;
	}
	else if (strcmp(gameObjectTypeName, "leveltrigger") == 0)
	{
		LevelTrigger * trigger = new LevelTrigger();
		trigger->XmlRead(objectElement);
		return trigger;
	}
	else if (strcmp(gameObjectTypeName, "waterblock") == 0)
	{
		WaterBlock * waterBlock = new WaterBlock();
		waterBlock->XmlRead(objectElement);
		return waterBlock;
	}

	return nullptr;
}

ParticleSpray * GameObjectManager::ReadParticleSpray(TiXmlElement * element)
{
	string texFileName = XmlUtilities::ReadAttributeAsString(element, "texture", "filename");

	float numParticles = XmlUtilities::ReadAttributeAsFloat(element, "numparticles", "value");

	float posX = XmlUtilities::ReadAttributeAsFloat(element, "position", "x");
	float posY = XmlUtilities::ReadAttributeAsFloat(element, "position", "y");
	float posZ = XmlUtilities::ReadAttributeAsFloat(element, "position", "z");

	//dimensions 
	float dimX = XmlUtilities::ReadAttributeAsFloat(element, "dimensions", "width");
	float dimY = XmlUtilities::ReadAttributeAsFloat(element, "dimensions", "height");
	float dimZ = XmlUtilities::ReadAttributeAsFloat(element, "dimensions", "breadth");

	float dirX = XmlUtilities::ReadAttributeAsFloat(element, "direction", "x");
	float dirY = XmlUtilities::ReadAttributeAsFloat(element, "direction", "y");

	float spread = XmlUtilities::ReadAttributeAsFloat(element, "spread", "value");

	float minSpeed = XmlUtilities::ReadAttributeAsFloat(element, "speed", "min");
	float maxSpeed = XmlUtilities::ReadAttributeAsFloat(element, "speed", "max");

	float minLive = XmlUtilities::ReadAttributeAsFloat(element, "livetime", "min");
	float maxLive = XmlUtilities::ReadAttributeAsFloat(element, "livetime", "max");

	float minSize = XmlUtilities::ReadAttributeAsFloat(element, "size", "min");
	float maxSize = XmlUtilities::ReadAttributeAsFloat(element, "size", "max");
	bool scaleto = XmlUtilities::ReadAttributeAsBool(element, "size", "scaleto");
	float scaletoValue = XmlUtilities::ReadAttributeAsFloat(element, "size", "scaletovalue");

	float gravity = XmlUtilities::ReadAttributeAsFloat(element, "gravity", "value");

	bool loop = XmlUtilities::ReadAttributeAsBool(element, "loop", "value");
	float loopTime = XmlUtilities::ReadAttributeAsFloat(element, "loop", "looptime");

	float minBrightness = XmlUtilities::ReadAttributeAsFloat(element, "brightness", "min");
	float maxBrightness = XmlUtilities::ReadAttributeAsFloat(element, "brightness", "max"); 

	Vector3 pos = Vector3(0, -23, 16);

	return ParticleEmitterManager::Instance()->CreateDirectedSprayLoadTime(numParticles,
																			Vector3(posX, posY, posZ),
																			Vector3(dirX, dirY, 0),
																			spread,
																			Vector3(dimX, dimY, dimZ),
																			texFileName.c_str(),
																			minSpeed,
																			maxSpeed,
																			minLive,
																			maxLive,
																			minSize,
																			maxSize,
																			gravity,
																			loop,
																			minBrightness,
																			maxBrightness,
																			loopTime,
																			scaleto,
																			scaletoValue);
}

void GameObjectManager::DeleteGameObjects()
{
	// traverse through the object list and delete
	list<GameObject*>::iterator current = m_gameObjects.begin();

	for(;current != m_gameObjects.end(); current++)
	{
		if ((*current))
		{
#ifdef _DEBUG
			string typeName = typeid((**current)).name();
#endif
			delete (*current);
			(*current)  = nullptr;
		}
	}

	m_player = 0;

	m_gameObjects.clear();
	m_drawableObjects.clear();
	m_updateableObjects.clear();
	m_drawableObjects.clear();
	CollisionManager::Instance()->ClearObjects();
	ParticleEmitterManager::Instance()->ClearParticles();
	
	mSlowMotionLayer = nullptr;

	m_levelLoaded = false;
}

void GameObjectManager::AddGameObject(GameObject * object)
{
	m_gameObjects.push_back(object);
}

void GameObjectManager::AddDrawableObject(DrawableObject *object)
{
	m_drawableObjects.push_back(object);
}

void GameObjectManager::AddUpdateableObject(GameObject * object)
{
	m_updateableObjects.push_back(object);
}

void GameObjectManager::AddDrawableObject_RunTime(DrawableObject * object, bool editModeAdd)
{
#if _DEBUG
	// don't let any new objects be created unless we purposely added them
	if (Game::GetIsLevelEditMode() && !editModeAdd)
	{
		return;
	}
#endif
	// no need to manually add it to gameObject list as this is done automatically in the constructor
	object->LoadContent(Graphics::GetInstance()->Device());
	object->Initialise();
	
	// SCALE
	// get the backbuffer width and height and determine how much we need to scale by
	// base width and height are 1920x1080 (need to not hard code this)
	float bbWidth = Graphics::GetInstance()->BackBufferWidth();
	float bbHeight = Graphics::GetInstance()->BackBufferHeight();

	float scaleX = bbWidth / 1920;
	float scaleY = bbHeight / 1080;

	// never scale position during the game, only before
	// it is always assumed you are giving a valid position during the game
	object->Scale(scaleX, scaleY, false);

	// order this object by depth in an efficient manner - TODO may need further optimising
	OrderDrawable_pushBack(object);
	// OrderDrawablesByDepth();
}

void GameObjectManager::AddAudioObject_RunTime(AudioObject * audioObject, bool editModeAdd)
{
#if _DEBUG
	// don't let any new objects be created unless we purposely added them
	if (Game::GetIsLevelEditMode() && !editModeAdd)
	{
		return;
	}
#endif
	audioObject->Initialise();

	float bbWidth = Graphics::GetInstance()->BackBufferWidth();
	float bbHeight = Graphics::GetInstance()->BackBufferHeight();

	float scaleX = bbWidth / 1920;
	float scaleY = bbHeight / 1080;

	// never scale position during the game, only before
	// it is always assumed you are giving a valid position during the game
	audioObject->Scale(scaleX, scaleY, false);
}

// order our drawable list by depth - this a once off that should be done at initialise stage
void GameObjectManager::OrderDrawablesByDepth()
{ 
	list<DrawableObject*>::iterator current = m_drawableObjects.begin();
	list<DrawableObject*>::iterator oneBehind = m_drawableObjects.begin();
	
	for(;current != m_drawableObjects.end(); current++)
	{
		list<DrawableObject*>::iterator other = current;
		for(; other != m_drawableObjects.end(); other++)
		{
			// dont check against the same object
			if(other!=current)
			{
				float otherZ = (*other)->Position().Z;
				float currentZ = (*current)->Position().Z;
				if(otherZ > currentZ)
				{
					// swap them
					DrawableObject * currentTemp = (*current);
					DrawableObject * otherTemp = (*other);

					(*current) = otherTemp;
					(*other) = currentTemp;
				}
			}
		}// end of inner for
	}// end of outer for
}

void GameObjectManager::CheckPlayerInput()
{
#ifdef _DEBUG

	// turn debug info on or off here
	static bool pressingDebugInfo = false;
	if (GetForegroundWindow() == DXWindow::GetInstance()->Hwnd() && GetAsyncKeyState('I'))
	{
		pressingDebugInfo = true;
	}
	else
	{
		if (pressingDebugInfo) // just released
		{
			if (mShowDebugInfo)
			{
				mShowDebugInfo = false;
			}
			else
			{
				mShowDebugInfo = true;
			}
		}
		pressingDebugInfo = false;
	}

#endif
	
	if (m_player && GetForegroundWindow() == DXWindow::GetInstance()->Hwnd())
	{
		GamePad * gamepad1 = GamePad::GetPad1();

		if (gamepad1 && gamepad1->IsConnected())
		{
			ProcessGamePad();
		}
		else
		{
			ProcessKeyboardMouse();
		}
	}
}

void GameObjectManager::ProcessKeyboardMouse()
{
	if (!m_player->GetIsCollidingAtObjectSide() || m_player->GetVelocity().Y <= 0.1)
	{
		if(GetAsyncKeyState(VK_LEFT))
		{
			m_player->AccelerateX(-1);
		}
		else if(GetAsyncKeyState(VK_RIGHT))
		{
			m_player->AccelerateX(1);
		}
		else
		{
			// not pressing anything
			m_player->StopXAccelerating();
		}
	}

	static bool pressingJump = false;
	static int jumpPower = 1;
	int defaultJumpPower = 50; // N percent of our max jump power
	int jumpPowerIncrement = 3;

	if(GetAsyncKeyState(VK_UP))
	{
		// build up our jump power
		jumpPower+= jumpPowerIncrement;

		pressingJump = true;
	}
	else
	{
		if(pressingJump) // we just released the jump key
		{
			if(jumpPower < defaultJumpPower)
			{
				jumpPower = defaultJumpPower;
			}
			m_player->Jump(jumpPower); // jump
		}
		pressingJump = false;
		jumpPower = 1;
	}
}

void GameObjectManager::ProcessGamePad()
{
	GamePad * gamepad1 = GamePad::GetPad1();

	XINPUT_STATE pad_state = gamepad1->GetState();

	if (pad_state.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE )
	{
		mCamYOffset = -500;
		mCamYShouldOffset = true;
	}
	else
	{
		mCamYShouldOffset = false;
	}
	
	// LEFT / RIGHT ======================
	
	// can't move left or right whilst moving up the side of an object
	// this helps the player latch on better
	if (!m_player->GetIsCollidingAtObjectSide() || m_player->GetVelocity().Y <= 0.1)
	{
		if (pad_state.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE )
		{
			// move left
			m_player->AccelerateX(-100);
		}
		else if (pad_state.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		{
			// move right
			m_player->AccelerateX(100);
		}
		else
		{
			// not pressing anything
			m_player->StopXAccelerating();
		}
	}
	// ===================================
	
	// JUMP ==============================
	static bool pressingJump = false;
	static int jumpPower = 1;
	int defaultJumpPower = 40; // N percent of our max jump power
	int jumpPowerIncrement = 3;

	static bool startedPressingJump = false;
	static float maxJumpTime = 0.2f;
	static float startedPressing = 0;

	if (pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_A)
	{
		float currentTime = Timing::Instance()->GetTotalTimeSeconds();
		if (!startedPressingJump)
		{
			startedPressing = currentTime;
			startedPressingJump = true;

			if (m_player->IsOnGround() || m_player->GetIsCollidingOnTopOfObject())
			{
				m_player->Jump(50); // 50 % jump to begin with
			}
		}
		else
		{
			float diff = currentTime - startedPressing;

			if (diff > 0 && diff < maxJumpTime)
			{
				float val = (float)diff/(float)maxJumpTime;
				m_player->Jump(50 + (val * 50));
			}
		}
	}
	else
	{
		if (m_player->GetIsCollidingOnTopOfObject() || m_player->IsOnGround())
		{
			startedPressingJump = false;
			startedPressing = 0;
		}
	}
	// ===================================

	// weapon ============================

	static bool pressing_weapon = false;
	if (pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_X)
	{
		pressing_weapon = true;
	}
	else
	{
		if(pressing_weapon)
		{
			// get aim direction 
			Vector2 dir = Vector2(m_player->DirectionX(), 0.1);

			if (pad_state.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
				pad_state.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
				pad_state.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
				pad_state.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				dir = Vector2(pad_state.Gamepad.sThumbLX, pad_state.Gamepad.sThumbLY);
				dir.Normalise();
			}
			
			// let the player fire and return a projectile object which is added to the world
			Projectile * p = m_player->FireWeapon(dir);
			
			if (p)
			{
				GameObjectManager::Instance()->AddDrawableObject_RunTime(p);
			}
		}
		pressing_weapon = false;
	}
	// ==========================

	// Bomb ============================

	static bool pressing_bomb = false;
	if (pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_B)
	{
		pressing_bomb = true;
	}
	else
	{
		if(pressing_bomb)
		{
			// get aim direction 
			Vector2 dir = Vector2(m_player->DirectionX(), 0);

			if (pad_state.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
				pad_state.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
				pad_state.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
				pad_state.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			{
				dir = Vector2(pad_state.Gamepad.sThumbLX, pad_state.Gamepad.sThumbLY);
				dir.Normalise();
			}
			
			// let the player fire and return a projectile object which is added to the world
			Projectile * p = m_player->FireBomb(dir);
			
			if (p)
			{
				GameObjectManager::Instance()->AddDrawableObject_RunTime(p);
			}
		}
		pressing_bomb = false;
	}
	// ==========================

	// Sprint ====================
	
	// only sprint if on solid ground or wall running
	if ((m_player->GetAccelY() > -0.1 && m_player->GetAccelY() < 0.1) || m_player->GetIsCollidingAtObjectSide())
	{
		if (pad_state.Gamepad.bLeftTrigger > 75)
		{
			m_player->SetSprintActive(true);
		}
		else
		{
			m_player->SetSprintActive(false);
		}
	}
	else
	{
		m_player->SetSprintActive(false);
	}

	// ===========================

	static bool pressingLeftShoulder = false;
	if (pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
	{
		pressingLeftShoulder = true;
	}
	else
	{
		if (pressingLeftShoulder)
		{
			// testing
			NinjaSpawner * spawner = new NinjaSpawner();
			spawner->SpawnMultiple(5, Vector2(m_player->X(), m_player->Y()), Vector2(1200, 1200)); 
		}

		pressingLeftShoulder = false;
	}

	// slow motion
	if (pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
	{
		Timing::Instance()->SetTimeModifier(0.1f);
	}
	else
	{
		Timing::Instance()->SetTimeModifier(1.0f);
	}

		
}

GameObject * GameObjectManager::CopyObject(GameObject * toCopy)
{
	TiXmlNode * xmlNode = toCopy->GetClonedXml();

	// clone again in case we do multiple copies
	TiXmlElement * xmlElement = dynamic_cast<TiXmlElement*>(xmlNode->Clone());

	if (!xmlElement)
	{
		return nullptr;
	}

	GameObject * returnObj = CreateObject(xmlElement);

	return returnObj;
}

void GameObjectManager::AddSlowMotionLayer()
{
	if (mSlowMotionLayer)
	{
		return;
	}

	float gameScale = Game::GetGameScale().X;
	mSlowMotionLayer = new ParallaxLayer(Camera2D::GetInstance());

	mSlowMotionLayer->m_textureFilename = "Media\\slow_motion_overlay.png";
	mSlowMotionLayer->m_drawAtNativeDimensions = true;
	mSlowMotionLayer->m_updateable = true;
	mSlowMotionLayer->m_position = Vector3(0, 0 * gameScale, 0);
	mSlowMotionLayer->m_dimensions = Vector3(2048, 1200, 0);
	mSlowMotionLayer->mRepeatTextureX = false;
	mSlowMotionLayer->mRepeatTextureY = false;
	mSlowMotionLayer->m_repeatWidth = 1920;
	mSlowMotionLayer->m_cameraParallaxMultiplierX = 0;
	mSlowMotionLayer->m_cameraParallaxMultiplierY = 0;
	mSlowMotionLayer->m_followCamXPos = true;
	mSlowMotionLayer->m_followCamYPos = true;
	mSlowMotionLayer->m_autoScrollY = false;
	mSlowMotionLayer->m_autoScrollX = false;
	mSlowMotionLayer->m_autoScrollXSpeed = 0;
	mSlowMotionLayer->m_autoScrollYSpeed = 0.0f;
	mSlowMotionLayer->EffectName = "effectlighttexture";
	mSlowMotionLayer->m_alpha = 0.0f;
	mSlowMotionLayer->mNoiseShaderIntensity = 0.01f;

	GameObjectManager::Instance()->AddDrawableObject_RunTime(mSlowMotionLayer);
}

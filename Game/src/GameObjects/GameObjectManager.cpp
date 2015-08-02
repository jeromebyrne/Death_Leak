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
#include "butterfly.h"
#include "scrollingsprite.h"
#include "SolidLineStrip.h"
#include "NPCTrigger.h"
#include "ForceBox.h"
#include "TextObject.h"
#include "AmbientBird.h"
#include "CurrencyOrb.h"

struct DepthSortPredicate
{
	bool operator() (const shared_ptr<GameObject> & lhs, const shared_ptr<GameObject> & rhs) 
	{
		return lhs->Z() > rhs->Z();
	}
};

GameObjectManager* GameObjectManager::m_instance = 0;

GameObjectManager::GameObjectManager(): 
	m_camera(),
	m_player(0),
	m_levelLoaded(false),
	mShowDebugInfo(false),
	mCamYShouldOffset(false),
	mCamYOffset(0),
	mSwitchToLevel(false),
	mSlowMotionLayer(nullptr),
	mFreshLevelLaunch(true)
{
}

GameObjectManager::~GameObjectManager(void)
{
}

/*
void GameObjectManager::OrderDrawable_pushBack(DrawableObject* object)
{
	LOG_INFO("Refactor GameObjectManager::OrderDrawable_pushBack");
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
*/

void GameObjectManager::RemoveGameObject(GameObject * object, bool defer)
{
	if (!object)
	{
		GAME_ASSERT(object);
		return;
	}
	
	if (defer)
	{
		m_killList.push_back(object);
	}
	else
	{
		auto iter = m_gameObjects.begin();
		for (auto & obj : m_gameObjects)
		{
			if (obj.get() == object)
			{
				break;
			}
			++iter;
		}

		if (iter != m_gameObjects.end())
		{
#if _DEBUG
			// the ref count should only be 1 when removing an object from the master list
			long refCount = (*iter).use_count();
			if (refCount > 1)
			{
				LOG_ERROR("POSSIBLE MEMORY LEAK!!! GameObject with ID: %u is being removed but still has a ref count greater than 1.", (*iter)->ID());
				// GAME_ASSERT(false);
			}
#endif
			m_gameObjects.remove(*iter);
		}
	}
}

shared_ptr<GameObject> & GameObjectManager::GetObjectByID(int id)
{
	for (auto & obj : m_gameObjects)
	{
		if(obj->ID() == id)
		{
			return obj;
		}
	}

	LOG_INFO("Did not find object with id: %i", id);
	GAME_ASSERT(false);
	return shared_ptr<GameObject>(nullptr);
}

// this is to be called before initialise
void GameObjectManager::LoadContent(ID3D10Device * device)
{
	for(auto & obj : m_gameObjects)
	{
		if (!obj)
		{
			GAME_ASSERT(obj);
			continue;
		}

		if (!obj->IsDrawable())
		{
			continue;
		}

		GAME_ASSERT(dynamic_cast<DrawableObject*>(obj.get()));
		DrawableObject * drawObj = static_cast<DrawableObject*>(obj.get());

		drawObj->LoadContent(device);
	}
}

void GameObjectManager::Initialise()
{
	for(auto &obj : m_gameObjects)
	{
		GAME_ASSERT(obj);
		if (obj)
		{
			obj->Initialise();
		}
	}

	GAME_ASSERT(m_player);
	if (m_player)
	{
		// set the camera to the players position initially
		m_camera->FollowObjectsOrigin(m_player);

		// now tell the camera that the player is the object we should follow
		m_camera->SetTargetObject(m_player);
	}
}

void GameObjectManager::Update(bool paused, float delta)
{
	mMusicManager.Update(delta); // TODO: delta should not have time modifier

	if (!paused)
	{
		float camX = m_camera->X();
		float camY = m_camera->Y();

		NPCManager::Instance()->Update();

		m_camera->FollowTargetObjectWithLag();

		// update the weather
		WeatherManager::GetInstance()->Update(delta);

		for(auto & obj : m_gameObjects) 
		{
			if (!obj)
			{
				GAME_ASSERT(obj);
				continue;
			}

			if (!obj->IsUpdateable())
			{
				continue;
			}

			// if the object is in the update zone
			if (Utilities::IsObjectInRectangle(obj.get(), camX, camY, m_updateZoneDimensions.X, m_updateZoneDimensions.Y))
			{
				obj->Update(delta); 
			}
			else if(obj->AlwaysUpdate())
			{
				obj->Update(delta); // always update parralax layers
			}
			else // objects outside the update area
			{
				// if a projectile has gone outside the bounds then just remove it
				if (obj->IsProjectile() || obj->IsOrb()) // add checks for other projectile class names as needed
				{
					RemoveGameObject(obj.get());
				}
			}
		}
	} // end of if paused

	// kill any objects in the kill list
	for (auto obj : m_killList)
	{
		RemoveGameObject(obj, false);
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
	for (auto &obj : m_gameObjects)
	{
		GAME_ASSERT(obj);
		if (obj)
		{
			obj->Scale(xScale, yScale);
		}
	}
}

void GameObjectManager::Draw(ID3D10Device *  device)
{
	for (auto & obj : m_gameObjects)
	{
		GAME_ASSERT(obj);
		if (!obj)
		{
			continue;
		}

		if (!obj->IsDrawable())
		{
			continue;
		}

		GAME_ASSERT(dynamic_cast<DrawableObject*>(obj.get()));
		DrawableObject * drawObj = static_cast<DrawableObject*>(obj.get());

		// only draw if object is in view
		if (drawObj->Alpha() > 0.0f && (m_camera->IsObjectInView(drawObj) || drawObj->GetParallaxMultiplierX() > 1.0f)) // Parallax multiplier X hack (keeps poping into view)
		{
			// apply any changes needed
			if (drawObj->IsChangeRequired())
			{
				drawObj->ApplyChange(device);
			}
			drawObj->Draw(device, m_camera);
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
}

void GameObjectManager::DebugDraw()
{
	if (mShowDebugInfo)
	{
		for (auto & obj : m_gameObjects)
		{
			GAME_ASSERT(obj);
			if (!obj)
			{
				continue;
			}

			// only draw if we are in view
			if (m_camera->IsObjectInView(obj.get()))
			{
				obj.get()->DebugDraw(Graphics::GetInstance()->Device());
			}
		}
	}
}

// load game objects via xml file
void GameObjectManager::LoadObjectsFromFile(const char* filename)
{
	GameObject::ResetGameIds();

#if _DEBUG
	if (Game::GetIsLevelEditMode())
	{
		Game::GetInstance()->SetLevelEditFilename(filename);
	}
#endif

	// play opening stinger
	AudioManager::Instance()->StopAllSounds();

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

	TiXmlElement * child = root->FirstChildElement();

	// loop through our game objects
	unsigned int objLoadCount = 0;
	while(child)
	{
		GameObject * object = CreateObject(child);
		if (object)
		{
			m_gameObjects.push_back(shared_ptr<GameObject>(object));
		}
		
		child = child->NextSiblingElement();

		++objLoadCount;

		if (objLoadCount > 5)
		{
			// I should really look into loading textures on a thread, but for now...
			UIManager::Instance()->RefreshUI();
			objLoadCount = 0;
		}
	}

	ParseLevelProperties(root);

	mMusicManager = MusicManager();
	mMusicManager.Initialise(mLevelProperties.GetLevelMusic(), 
							 mLevelProperties.GetMusicLength(),
							 mLevelProperties.GetMusicInitialDelay(), 
							 mLevelProperties.GetMusicTimeBetween());

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

	WeatherManager::GetInstance()->RefreshAssets();

	AddSlowMotionLayer();

	if (mFreshLevelLaunch)
	{
		mFreshLevelLaunch = false;
	}
	else
	{
		// this level was triggered by a LevelTrigger
		GAME_ASSERT(m_player);
		if (m_player)
		{
			m_player->SetX(mPlayerStartPosForLevel.X);
			m_player->SetY(mPlayerStartPosForLevel.Y);
			m_player->SetDirectionXYZ(mPlayerStartDirectionXForLevel, 1, 0);
			m_player->AccelerateX(mPlayerStartDirectionXForLevel);
			m_camera->SetPositionY(mPlayerStartPosForLevel.Y + 300);
			m_camera->SetPositionX(-mPlayerStartPosForLevel.X);
			m_camera->FollowTargetObjectWithLag(true, 1.0f, 2.0f);
		}
	}

	// update all the objects at least once at the start
	for (auto & obj : m_gameObjects)
	{
		obj->Update(0);
	}

	// loaded a level
	m_levelLoaded = true;
}

void GameObjectManager::ParseLevelProperties(TiXmlElement * element)
{
	TiXmlElement * child = element->FirstChildElement();

	bool foundLevelProperties = false;
	// loop through our game objects
	while (child)
	{
		// what type of object is this
		const char* objName = child->Value();
		Utilities::ToLower((char *)objName); // TODO: this is nasty, pass a const char * and return a new std::string

		// start looking at what object we need to make
		if (strcmp(objName, "levelproperties") == 0)
		{
			mLevelProperties.XmlRead(child);

			foundLevelProperties = true;

			return;
		}

		child = child->NextSiblingElement();
	}

	GAME_ASSERT(foundLevelProperties);
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
	// loop through the game objects and make sure there are no duplicates
	list<GameObject *> checkedList;

	for (auto & obj : m_gameObjects)
	{
		for (auto checked : checkedList)
		{
			if (checked == obj.get())
			{
				// duplicate
				GAME_ASSERT(false);
				return;
			}
		}

		checkedList.push_back(obj.get());
	}

	std::map<int, GameObject *> sortedObjects;

	list<int> addedIDs;

	for (auto & obj : m_gameObjects)
	{
		for (auto i : addedIDs)
		{
			if (i == obj->ID())
			{
				// make sure IDs are unique
				GAME_ASSERT(i != obj->ID());
				return;
			}
		}
		sortedObjects[obj->ID()] = obj.get();
		addedIDs.push_back(obj->ID());
	}

	XmlDocument doc;
	TiXmlElement * root = new TiXmlElement( "level" );
	root->SetAttribute("audio", ""); 

	TiXmlElement * levelPropsElement = new TiXmlElement("LevelProperties");

	mLevelProperties.XmlWrite(levelPropsElement);

	root->LinkEndChild(levelPropsElement);

	for (auto obj : sortedObjects)
	{
		TiXmlElement * objElem = SaveObject(obj.second);

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
	TiXmlElement * element = new TiXmlElement(objectType.c_str());
	object->XmlWrite(element);
	return element;
}

GameObject * GameObjectManager::CreateObject(TiXmlElement * objectElement)
{
	// what type of object is this
	const char* gameObjectTypeName = objectElement->Value();
	Utilities::ToLower((char *)gameObjectTypeName); // TODO: this is nasty, pass a const char * and return a new std::string
	
	GameObject * newGameObject = nullptr;

	// start looking at what object we need to make
	if(strcmp(gameObjectTypeName, "player") == 0)
	{
		if (!m_player) // only create 1 player
		{
			newGameObject = new Player();
			m_player = static_cast<Player*>(newGameObject);
		}
	}
	else if (strcmp(gameObjectTypeName, "npc") == 0)
	{
		newGameObject = new NPC();
	}
	else if (strcmp(gameObjectTypeName, "solidmovingsprite") == 0)
	{
		newGameObject = new SolidMovingSprite();
	}
	else if (strcmp(gameObjectTypeName, "sprite") == 0)
	{
		newGameObject = new Sprite();
	}
	else if (strcmp(gameObjectTypeName, "movingsprite") == 0)
	{
		newGameObject = new MovingSprite();
	}
	else if (strcmp(gameObjectTypeName, "parallaxlayer") == 0)
	{
		newGameObject = new ParallaxLayer(m_camera);
	}
	else if (strcmp(gameObjectTypeName, "platform") == 0)
	{
		newGameObject = new Platform();
	}
	else if (strcmp(gameObjectTypeName, "pathingplatform") == 0)
	{
		newGameObject = new PathingPlatform();
	}
	else if (strcmp(gameObjectTypeName, "fallingplatform") == 0)
	{
		newGameObject = new FallingPlatform();
	}
	else if(strcmp(gameObjectTypeName, "particlespray") == 0)
	{
		newGameObject = ReadParticleSpray(objectElement);
	}
	else if (strcmp(gameObjectTypeName, "audioobject") == 0)
	{
		newGameObject = new AudioObject();
	}
	else if (strcmp(gameObjectTypeName, "rabbit") == 0)
	{
		newGameObject = new Rabbit();
	}
	else if (strcmp(gameObjectTypeName, "leveltrigger") == 0)
	{
		newGameObject = new LevelTrigger();
	}
	else if (strcmp(gameObjectTypeName, "waterblock") == 0)
	{
		newGameObject = new WaterBlock();
	}
	else if (strcmp(gameObjectTypeName, "butterfly") == 0)
	{
		newGameObject = new Butterfly();
	}
	else if (strcmp(gameObjectTypeName, "scrollingsprite") == 0)
	{
		newGameObject = new ScrollingSprite();
	}
	else if (strcmp(gameObjectTypeName, "solidlinestrip") == 0)
	{
		newGameObject = new SolidLineStrip();
	}
	else if (strcmp(gameObjectTypeName, "npctrigger") == 0)
	{
		newGameObject = new NPCTrigger();
	}
	else if (strcmp(gameObjectTypeName, "forcebox") == 0)
	{
		newGameObject = new ForceBox();
	}
	else if (strcmp(gameObjectTypeName, "textobject") == 0)
	{
		newGameObject = new TextObject();
	}
	else if (strcmp(gameObjectTypeName, "ambientbird") == 0)
	{
		newGameObject = new AmbientBird();
	}
	else if (strcmp(gameObjectTypeName, "currencyorb") == 0)
	{
		newGameObject = new CurrencyOrb();
	}

	if (newGameObject && !dynamic_cast<ParticleSpray*>(newGameObject))
	{
		newGameObject->XmlRead(objectElement);
	}
	
	return newGameObject;
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

	float spawnSpreadX = XmlUtilities::ReadAttributeAsFloat(element, "position", "spawn_spread_x");
	float spawnSpreadY = XmlUtilities::ReadAttributeAsFloat(element, "position", "spawn_spread_y");

	float fadeInPercentTime = XmlUtilities::ReadAttributeAsFloat(element, "fade_time_percent", "in");
	float fadeOutPercentTime = XmlUtilities::ReadAttributeAsFloat(element, "fade_time_percent", "out");

	Vector3 pos = Vector3(0, -23, 16);

	ParticleSpray * p = ParticleEmitterManager::Instance()->CreateDirectedSprayLoadTime(numParticles,
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
																						scaletoValue,
																						spawnSpreadX,
																						spawnSpreadY,
																						fadeInPercentTime,
																						fadeOutPercentTime);
	p->SetXmlForCloning(element);

	return p;
}

void GameObjectManager::DeleteGameObjects()
{
	m_player = nullptr;

	m_gameObjects.clear();
	ParticleEmitterManager::Instance()->ClearParticles();
	
	mSlowMotionLayer = nullptr;

	m_levelLoaded = false;
}

void GameObjectManager::AddGameObject(GameObject * object, bool editModeAdd)
{
	GAME_ASSERT(object);

	if (!object)
	{
		return;
	}

#if _DEBUG
	// don't let any new objects be created unless we purposely added them
	if (Game::GetIsLevelEditMode() && !editModeAdd)
	{
		return;
	}
#endif

	m_gameObjects.push_back(shared_ptr<GameObject>(object));

	// no need to manually add it to gameObject list as this is done automatically in the constructor
	if (object->IsDrawable())
	{
		object->LoadContent(Graphics::GetInstance()->Device());
	}
	object->Initialise();
	
	// SCALE
	// get the backbuffer width and height and determine how much we need to scale by
	// base width and height are 1920x1080 (need to not hard code this)
	float bbWidth = Graphics::GetInstance()->BackBufferWidth();
	float bbHeight = Graphics::GetInstance()->BackBufferHeight();

	float scaleX = bbWidth / 1920.0f;
	float scaleY = bbHeight / 1080.0f;

	// never scale position during the game, only before
	// it is always assumed you are giving a valid position during the game
	object->Scale(scaleX, scaleY, false);

	// TODO: Optimise, huge bottleneck
	LOG_INFO("optimise AddGameObject");

	OrderDrawablesByDepth();
}

// order our drawable list by depth - this a once off that should be done at initialise stage
void GameObjectManager::OrderDrawablesByDepth()
{
	m_gameObjects.sort(DepthSortPredicate());
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
		if (!m_player->IsOnSolidSurface() && !(m_player->WasInWaterLastFrame() && m_player->GetWaterIsDeep()))
		{
			pressingJump = false;
		}
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
		if (!startedPressingJump && (m_player->IsOnSolidSurface() || (m_player->WasInWaterLastFrame() && m_player->GetWaterIsDeep())))
		{
			startedPressing = currentTime;
			startedPressingJump = true;

			m_player->Jump(50); // 50 % jump to begin with
		}
		else if (startedPressingJump)
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
		startedPressingJump = false;
		startedPressing = 0;
	}
	// ===================================

	// weapon ============================
	if (pad_state.Gamepad.sThumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
		pad_state.Gamepad.sThumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
		pad_state.Gamepad.sThumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
		pad_state.Gamepad.sThumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	{
		// get aim direction 
		Vector2 dir = Vector2(pad_state.Gamepad.sThumbRX, pad_state.Gamepad.sThumbRY);
		dir.Normalise();

		// let the player fire and return a projectile object which is added to the world
		Projectile * p = m_player->FireWeapon(dir);

		if (p)
		{
			GameObjectManager::Instance()->AddGameObject(p);
		}
	}
	else
	{
		m_player->ResetProjectileFireDelay();
	}
	
	// ==========================

	// melee ============================

	static bool pressing_melee = false;
	if (pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_X)
	{
		pressing_melee = true;
	}
	else
	{
		if (pressing_melee)
		{
			/*
			// get aim direction 
			Vector2 dir = Vector2(m_player->DirectionX(), 0.1f);

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
				GameObjectManager::Instance()->AddGameObject(p);
			}
			*/
		}
		pressing_melee = false;
	}
	// ==========================

	// Bomb ============================

	static bool pressing_bomb = false;
	if (pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
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
				GameObjectManager::Instance()->AddGameObject(p);
			}
		}
		pressing_bomb = false;
	}
	// ==========================

	// Sprint ====================
	
	// only sprint if on solid ground or wall running and not in water
	if (!m_player->WasInWaterLastFrame() && ((m_player->GetAccelY() > -0.1f && m_player->GetAccelY() < 0.1f) || m_player->GetIsCollidingAtObjectSide()))
	{
		if (pad_state.Gamepad.bLeftTrigger > 75.0f)
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

	// Ninja spawning
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
			Timing::Instance()->SetTimeModifierForNumSeconds(0.1f, 3.5f);
			NinjaSpawner spawner;
			spawner.SpawnMultiple(5, Vector2(m_player->X(), m_player->Y()), Vector2(1200, 1200));
		}

		pressingLeftShoulder = false;
	}

	// slow motion
	static bool pressing_slo_mo = false;
	if (pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
	{
		pressing_slo_mo = true;
	}
	else
	{
		if (pressing_slo_mo)
		{
			LOG_INFO("This is a really bad way to do this. Come back later.");
			if (Timing::Instance()->GetTimeModifier() == 1.0f)
			{
				Timing::Instance()->SetTimeModifier(0.1f);
			}
			else
			{
				Timing::Instance()->SetTimeModifier(1.0f);
			}
		}
		pressing_slo_mo = false;
	}		
}

GameObject * GameObjectManager::CopyObject(GameObject * toCopy)
{
	GAME_ASSERT(toCopy);
	if (!toCopy)
	{
		return nullptr;
	}

	TiXmlNode * xmlNode = toCopy->GetClonedXml();

	if (!xmlNode)
	{
		GAME_ASSERT(false);
		return nullptr;
	}

	// clone again in case we do multiple copies
	TiXmlElement * xmlElement = dynamic_cast<TiXmlElement*>(xmlNode->Clone());

	if (!xmlElement)
	{
		return nullptr;
	}

	return CreateObject(xmlElement);
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

	GameObjectManager::Instance()->AddGameObject(mSlowMotionLayer);
}

void GameObjectManager::GetSolidSpritesOnScreen(std::list<GameObject *> & toPopulate)
{
	toPopulate.clear();

	Camera2D * cam = Camera2D::GetInstance();

	for (auto & obj : m_gameObjects)
	{
		GameObject * objRawPtr = obj.get();
		GAME_ASSERT(objRawPtr);

		if (objRawPtr->IsSolidSprite() &&
			cam->IsObjectInView(objRawPtr))
		{
			toPopulate.push_back(objRawPtr);
		}
	}
}

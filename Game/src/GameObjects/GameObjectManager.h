#ifndef GAMEOBJECTMANAGER_H
#define GAMEOBJECTMANAGER_H

#include "gameObject.h"
#include "drawableobject.h"
#include "player.h"
#include "particleSpray.h"
#include "LevelProperties.h"
#include "MusicManager.h"

class Camera2D;
class AudioObject;
class ParallaxLayer;

class GameObjectManager
{
public:

	void CheckPlayerInput();
	void ProcessKeyboardMouse();
	void ProcessGamePad();

	bool IsLevelLoaded() { return m_levelLoaded; }

	inline static GameObjectManager* Instance()
	{
		if (m_instance == 0)
		{
			m_instance = new GameObjectManager();
		}
		return m_instance;
	}

	void Update(bool paused, float delta); // call update function on all game objects
	void Draw(ID3D10Device * device); // call Draw on all drawable objects
	void DebugDraw();

	// this function also acts as a public initialise
	void LoadObjectsFromFile(const char* filename);// load game objects via xml file
	void SaveObjectsToFile(const char* filename);
	void AddGameObject(GameObject * object, bool editModeAdd = false);
	void RemoveGameObject(GameObject * object, bool defer = true);

	shared_ptr<GameObject> & GetObjectByID(int id);

	void DeleteGameObjects();
	Player * GetPlayer() const { return m_player; }

	list<shared_ptr<GameObject> > & GetGameObjectList()
	{
		return m_gameObjects;
	}

	void OrderDrawablesByDepth(); // reorders the drawables list by depth

	void SetShowDebugInfo(bool value) { mShowDebugInfo = value; };

	void GetSolidSpritesOnScreen(std::list<GameObject *> & toPopulate);

	template <class T> void GetTypesOnScreen(std::list<GameObject *> & toPopulate)
	{
		toPopulate.clear();

		Camera2D * cam = Camera2D::GetInstance();

		for (auto & obj : m_gameObjects)
		{
			// GAME_ASSERT(obj);
			if (!obj)
			{
				continue;
			}

			GameObject * rawPointer = obj.get();
			if (cam->IsObjectInView(rawPointer))
			{
				T * t = dynamic_cast<T*>(rawPointer);
				if (t)
				{
					toPopulate.push_back(rawPointer);
				}
			}
		}
	}

	// only works in debug, designed for level editor
	GameObject * CopyObject(GameObject * toCopy);

	void SwitchToLevel(const char * level, bool defer = true);

	bool mSwitchToLevel;
	std::string mLevelToSwitch;

	Vector2 GetPlayerStartPos() const { return mPlayerStartPosForLevel; }

	void SetPlayerStartPos(Vector2 & position) { mPlayerStartPosForLevel = position; }

	float GetPlayerStartDirectionX() const { return mPlayerStartDirectionXForLevel; }

	void SetPlayerStartDirectionX(float value) { mPlayerStartDirectionXForLevel = value; }

	void SetLevelFreshLaunch(bool value) { mFreshLevelLaunch = value; }

private:

	Camera2D * m_camera;
	static GameObjectManager* m_instance;
	list<shared_ptr<GameObject> > m_gameObjects;
	list<GameObject *> m_killList; // a list of all objects which need to be removed
	GameObject * CreateObject(TiXmlElement * object);
	TiXmlElement * SaveObject(GameObject * object);
	void LoadContent(ID3D10Device * device); // load graphics content for drawable objects
	void Initialise(); // call initialise function on all game objects

	Graphics * m_graphicsSystem; // a pointer to the graphics system
	ID3D10Device * m_graphicsDevice; // a pointer to the graphics device
	Vector2 m_updateZoneDimensions; // screen space dimensions (+ more) in which we update objects, if outside then dont update

	ParticleSpray * ReadParticleSpray(TiXmlElement * element);

	void ParseLevelProperties(TiXmlElement * element);

	void AddSlowMotionLayer();

	GameObjectManager();
	~GameObjectManager(void);
	
	// the current player object
	Player * m_player;
	
	// is there a level loaded
	bool m_levelLoaded;

	void ScaleObjects(float xScale, float yScale);

	bool mShowDebugInfo;

	// should the camera be looking ip above the player or down below
	bool mCamYShouldOffset;
	float mCamYOffset; // if mCamYShouldOffset = true then this decides up or down 

	ParallaxLayer * mSlowMotionLayer;

	LevelProperties mLevelProperties;

	Vector2 mPlayerStartPosForLevel;
	float mPlayerStartDirectionXForLevel;

	bool mFreshLevelLaunch;

	MusicManager mMusicManager;
};

#endif

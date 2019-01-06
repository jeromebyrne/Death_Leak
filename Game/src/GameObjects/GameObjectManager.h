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

	bool IsLevelLoaded() { return m_levelLoaded; }

	inline static GameObjectManager* Instance()
	{
		if (m_instance == nullptr)
		{
			m_instance = new GameObjectManager();
		}
		return m_instance;
	}

	void Update(bool paused, float delta); // call update function on all game objects
	void PostUpdate(bool paused, float delta);
	void Draw(ID3D10Device * device); // call Draw on all drawable objects
	void DebugDraw();

	// this function also acts as a public initialise
	void LoadObjectsFromFile(const string & filename);// load game objects via xml file
	void SaveObjectsToFile(const string & filename);
	void AddGameObject(GameObject * object, bool editModeAdd = false);
	void RemoveGameObject(GameObject * object, bool defer = true);
	void AddGameObjectViaLevelEditor(GameObject * object);

	shared_ptr<GameObject> & GetObjectByID(int id);

	void DeleteGameObjects();
	Player * GetPlayer() const { return m_player; }

	list<shared_ptr<GameObject> > & GetGameObjectList()
	{
		return m_gameObjects;
	}

	void OrderDrawablesByDepth(); // reorders the drawables list by depth

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

	void SwitchToLevel(const string & level, const string & doorId, bool defer = true);

	bool mSwitchToLevel;
	std::string mLevelToSwitch;
	std::string mLastLevel;
	std::string mDoorIdCameFrom;

	void SetLevelFreshLaunch(bool value) { mFreshLevelLaunch = value; }

	void SetCurrencyOrbCollected(unsigned int orbId);

	void SetBreakableBroken(unsigned int breakableId);

	std::string GetCurrentLevelFile() const { return mCurrentLevelFile; }

	const LevelProperties & GetCurrentLevelProperties() const { return mLevelProperties; }

	TiXmlElement * ConvertObjectToXmlElement(GameObject * object);

	GameObject * CreateObject(TiXmlElement * object, const std::vector<unsigned int> & orbsCollected);

	void SaveGame();

	void QuitLevel();

	void SpawnHealthIncrease(const Vector2 & position);

private:

	Camera2D * m_camera;
	static GameObjectManager* m_instance;
	list<shared_ptr<GameObject> > m_gameObjects;
	list<GameObject *> m_killList; // a list of all objects which need to be removed
	void LoadContent(ID3D10Device * device); // load graphics content for drawable objects
	void Initialise(); // call initialise function on all game objects

	Graphics * m_graphicsSystem; // a pointer to the graphics system
	ID3D10Device * m_graphicsDevice; // a pointer to the graphics device
	Vector2 m_updateZoneDimensions; // screen space dimensions (+ more) in which we update objects, if outside then dont update

	ParticleSpray * ReadParticleSpray(TiXmlElement * element);

	void ParseLevelProperties(TiXmlElement * element);

	void AddSlowMotionLayer();

	void CacheSaveData();

	GameObjectManager();
	~GameObjectManager(void);
	
	// the current player object
	Player * m_player;
	
	// is there a level loaded
	bool m_levelLoaded;

	void ScaleObjects(float xScale, float yScale);

	ParallaxLayer * mSlowMotionLayer;

	LevelProperties mLevelProperties;

	bool mFreshLevelLaunch;

	MusicManager mMusicManager;

	std::vector<unsigned int> mCurrentCurrencyOrbIdsCollected;

	std::vector<unsigned int> mCurrentBreakablesBroken;

	std::string mCurrentLevelFile;

	list<GameObject *> mPostUpdateObjects;

	// save the health between levels
	float mCachedPlayerHealth = -1;
};

#endif

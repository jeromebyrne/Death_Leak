#ifndef GAMEOBJECTMANAGER_H
#define GAMEOBJECTMANAGER_H
#include "gameObject.h"
#include "drawableobject.h"
#include "player.h"
#include "particleSpray.h"

class Camera2D;
class AudioObject;
class ParallaxLayer;

class GameObjectManager
{
private:
	Camera2D * m_camera; //store teh camera
	static GameObjectManager* m_instance;
	list<unique_ptr<GameObject> > m_gameObjects;
	list<GameObject *> m_updateableObjects; // a list of all updateable objects
	list<DrawableObject *> m_drawableObjects; // a list of all the drawable objects
	list<GameObject*> m_killList; // a list of all objects which need to be removed
	GameObject * CreateObject(TiXmlElement * object);
	TiXmlElement * SaveObject(GameObject * object);
	void LoadContent(ID3D10Device * device); // load graphics content for drawable objects
	void Initialise(); // call initialise function on all game objects

	Graphics * m_graphicsSystem; // a pointer to the graphics system
	ID3D10Device * m_graphicsDevice; // a pointer to the graphics device
	Vector2 m_updateZoneDimensions; // screen space dimensions (+ more) in which we update objects, if outside then dont update

	void OrderDrawable_pushBack(DrawableObject* object); // call this in a drawables constructor

	ParticleSpray * ReadParticleSpray(TiXmlElement * element);

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

public:

	void CheckPlayerInput();
	void ProcessKeyboardMouse();
	void ProcessGamePad();

	bool IsLevelLoaded() { return m_levelLoaded; }
	
	inline static GameObjectManager* Instance()
	{
		if(m_instance == 0)
		{
			m_instance = new GameObjectManager();
		}
		return m_instance;
	}

	void Update(bool paused, float delta); // call update function on all game objects
	void Draw(ID3D10Device * device); // call Draw on all drawable objects

	// this function also acts as a public initialise
	void LoadObjectsFromFile(const char* filename);// load game objects via xml file
	void SaveObjectsToFile(const char* filename);
	// void AddGameObject(GameObject* object);
	// void AddDrawableObject(DrawableObject* object);
	// void AddUpdateableObject(GameObject* object);
	void AddDrawableObject_RunTime(DrawableObject * object, bool editModeAdd = false);
	void RemoveGameObject_RunTime(GameObject * object, bool defer = true); // remove aan object from it's appropriate lists and releases it's memory
	void AddAudioObject_RunTime(AudioObject * audioObject, bool editModeAdd = false);

	unique_ptr<GameObject> & GetObjectByID(int id);

	void DeleteGameObjects();

	Player * GetPlayer() const { return m_player; }

#if _DEBUG
	// for level editor
	list<unique_ptr<GameObject> > & GetGameObjectList() 
	{ 
		return m_gameObjects; 
	}
#endif

	void OrderDrawablesByDepth(); // reorders the drawables list by depth

	void SetShowDebugInfo(bool value) { mShowDebugInfo = value; };

	template <class T> void GetTypesOnScreen (std::list<T*> & toPopulate)
	{
		/*toPopulate.clear();

		Camera2D * cam = Camera2D::GetInstance();

		for (auto obj : m_gameObjects)
		{
			if (cam->IsObjectInView(obj))
			{
				T * t = dynamic_cast<T*>(obj);
				if (t)
				{
					toPopulate.push_back(t);
				}
			}
		}*/
	}

	// only works in debug, designed for level editor
	GameObject * CopyObject(GameObject * toCopy);

	void SwitchToLevel(const char * level, bool defer = true);

	bool mSwitchToLevel;
	std::string mLevelToSwitch;
};

#endif

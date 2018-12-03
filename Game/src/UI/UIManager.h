#ifndef UIMANAGER_H
#define UIMANAGER_H

#include "uiscreen.h"

class UISprite;

// all of the possible UI actions 
enum UIEventActions 
{	
	POP_UI,
	PUSH_UI,
	LOAD_LEVEL,
	PLAY_SOUND_EFFECT,
	PLAY_MUSIC,
	REFRESH_UI,
	STOP_ALL_SOUNDS,
	PAUSE_GAME,
	UNPAUSE_GAME,
	DESTROY_LEVEL,
	SLEEP,
	APPLY_ALPHA,
	FADE_OUT,
	FADE_IN,
	MUTE_SOUND_EFFECTS,
	UNMUTE_SOUND_EFFECTS,
	UNMUTE_MUSIC,
	MUTE_MUSIC,
	QUIT_TO_DESKTOP,
	LEVEL_EDIT,
	SET_LANGUAGE,
	APPLY_OBJECT_EDIT_CHANGES
};

class UIManager
{
public:

	static UIManager * Instance();

	void XmlRead(const char * uiRootPath);

	void Initialise();

	void LoadContent(Graphics * graphicsSystem);

	void Update();

	void Draw(ID3D10Device * device);

	void Release();

	// pushes a UI onto the current UI list
	UIScreen * PushUI(string uiName);

	// pops a UI off the current UI list
	void PopUI(string uiName);

	EffectLightTextureVertexWobble * GetDefaultEffect() { return m_defaultEffect; }

	EffectLightTexture * GetStandardEffect() { return mStandardEffect; }

	void PushBackEvent(string eventName, list<string> eventParams);

	Vector2 GetPointInUICoords(float x, float y);

	// draws the UI there and then 
	void RefreshUI();

	void HandleEvents();

	int GetBaseWidth() const { return mBaseWidth; }
	int GetBaseHeight() const { return mBaseHeight; }

	bool IsInKeyboardInputMode() const { return mIsInKeyboardInputMode; }

	void SetIsInKeyboardInputMode(bool value);

	void HandleKeyPressInKeyboardInputMode(char character);

	void HandleBackspaceInKeyboardInputMode();

	const std::string & GetKeyboardInputInKeyboardInputMode() const { return mKeyboardInput; }

	void SetStartingInputInKeyboardInputMode(const std::string & startingInput)  { mKeyboardInput = startingInput; }

	void DisplayObjectEditor(GameObject * gameObject);

	void DismissObjectEditor();

	bool IsObjectEditorDisplaying() const { return mObjectEditorDisplaying; }

	void AddInteractableToDraw(GameObject::InteractableProperties iProp);

private:

	void DisplayLaunchUI();

	void ApplyObjectEditChanges();

	static UIManager * m_instance;
	
	// a map of all the screens in the game
	map<string, UIScreen*> m_allScreens;

	// a list of all the screens being displayed currently
	list<UIScreen*> m_currentScreens;

	UIManager(void);
	~UIManager(void);

	// shader effect 
	EffectLightTextureVertexWobble * m_defaultEffect;
	EffectLightTexture * mStandardEffect;

	map<string, int> m_ActionStringToEnumMap;

	void InitActionStringToEnumMap();

	// this is where all UI events get handled (just a big switch statement :/ )
	void HandleEvent(string eventName, list<string> params);

	UISprite * CreateCursorSprite();

	void CreateInteractableSprites();
	
	// a list of events that need to be processed in the next update loop
	list<EventStruct> mCurrentEventList;

	int mBaseWidth;
	int mBaseHeight;

	UISprite * mCursorSprite;

	bool mIsInKeyboardInputMode = false;

	std::string mKeyboardInput;

	bool mObjectEditorDisplaying = false;

	list<GameObject::InteractableProperties> mInteractableIconsToDraw;

	vector<UISprite *> mInteractableSprites;
};

#endif

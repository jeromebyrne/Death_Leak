#ifndef GAME_H
#define GAME_H

#include "InputManager.h"
#include "steam_api.h"

class EffectLightTexture;
class EffectLightTextureVertexWobble;
class EffectParticleSpray;
class EffectBloodParticleSpray;
class EffectLightTextureBump;
class EffectBasic;
class LevelEditor;
class EffectNoise;
class EffectSepia;
class EffectMonochrome;
class EffectMonochromeRed;
class EffectLightTexturePixelWobble;
class EffectFoliageSway;
class ScreenAlignedTexture;
class LevelEditor;
class ScreenAlignedTexture;
class Camera2D;
class Graphics;
class UIManager;

class Game
{
public:

	Game(Graphics * pGraphics);
	~Game(void);
	void Initialise();
	void Update(float delta);
	void Draw();
	void PostDraw(); // post processsing effects here
	void Cleanup();

	static void PauseGame(bool pushPauseScreen = true);
	static void UnPauseGame(bool popPauseScreen = true);
	static void SetIsLevelEditMode(bool value) { mLevelEditMode = value; }

	static void DisplayTextModal(const string & localizedTitle, const string & localizeddesc);
	static void DismissTextModal();

	static bool GetIsLevelEditMode() { return mLevelEditMode; }

	static Game * GetInstance() { return mInstance; }
	static void Create();
	static void Destroy();

	bool IsLevelEditTerrainMode() const;

	void SetLevelEditFilename(const string & file);

	static Vector2 GetGameScale() { return mGameScale; }

	void ResetLevelEditor();

	void ResetLevelEditorSelectedObject();

	// This is an effect to add pauses for enemy damage etc...
	void DoDamagePauseEffect();

	void DoDamagePauseEffectLonger();
		
	const InputManager & GetInputManager() const { return mInputManager; }

	void Game::OnSteamGameOverlayActivated(GameOverlayActivated_t* pCallback);

private:

	void * m_steamcallback_OnSteamGameOverlayActivated = nullptr;

	// graphics manager
	Graphics * m_pGraphics;

	// 2D camera
	Camera2D * m_pCam2d;

	// Our effects
	EffectLightTexture * m_effectLightTexture;
	EffectLightTextureVertexWobble * m_effectLightTextureVertexWobble;
	EffectParticleSpray * m_effectParticleSpray;
	EffectBloodParticleSpray * m_effectBloodParticleSpray;
	EffectLightTextureBump * m_effectLightTextureBump;
	EffectBasic * m_effectBasic;
	LevelEditor * mlevelEditor;
	EffectSepia * m_effectSepia;
	EffectMonochrome * m_effectMonochrome;
	EffectMonochromeRed * m_effectMonochromeRed;
	EffectNoise * m_effectNoise;
	EffectLightTexturePixelWobble * m_effectPixelWobble;
	EffectFoliageSway * m_effectFoliageSway;
	static bool mPaused;
	static bool mLevelEditMode;
	static bool mIsDisplayingTextModal;
	ScreenAlignedTexture * m_screenAlignedPostProcTex1;
	static Vector2 mGameScale;
	static Game * mInstance;
	float mLastTimeDamagePauseEffect;
	InputManager mInputManager;
	bool mDamagePauseEnabled = true;
	GameObjectManager * mGOMInstance = nullptr;
	UIManager * mUIManagerInstance = nullptr;
	float mPauseEffectDelay = 0.0f;
};

#endif

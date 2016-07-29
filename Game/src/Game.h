#ifndef GAME_H
#define GAME_H

#include "InputManager.h"

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

	static void PauseGame();
	static void UnPauseGame();
	static void SetIsLevelEditMode(bool value) { mLevelEditMode = value; }

	static bool GetIsLevelEditMode() { return mLevelEditMode; }

	static Game * GetInstance() { return mInstance; }
	static void Create();
	static void Destroy();

	bool IsLevelEditTerrainMode() const;

	void SetLevelEditFilename(const char * file);

	static Vector2 GetGameScale() { return mGameScale; }

	void ResetLevelEditor();

	// This is an effect to add pauses for enemy damage etc...
	void DoDamagePauseEffect();

	void DisableDebugInfoEnabled();
		
	const InputManager & GetInputManager() const { return mInputManager; }

private:

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
	ScreenAlignedTexture * m_screenAlignedPostProcTex1;
	static Vector2 mGameScale;
	static Game * mInstance;
	float mLastTimeDamagePauseEffect;
	InputManager mInputManager;
};

#endif

#ifndef GAME_H
#define GAME_H

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
class ScreenAlignedTexture;
class LevelEditor;
class ScreenAlignedTexture;
class Camera2D;
class Graphics;

class Game
{
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
#if _DEBUG
	EffectBasic * m_effectBasic;
	LevelEditor * mlevelEditor;
#endif
	EffectSepia * m_effectSepia;
	EffectMonochrome * m_effectMonochrome;
	EffectMonochromeRed * m_effectMonochromeRed;
	EffectNoise * m_effectNoise;
	EffectLightTexturePixelWobble * m_effectPixelWobble;
	static bool mPaused;
	static bool mLevelEditMode;
	ScreenAlignedTexture * m_screenAlignedPostProcTex1;
	static Vector2 mGameScale;

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

	static Vector2 GetGameScale() { return mGameScale; }
};

#endif
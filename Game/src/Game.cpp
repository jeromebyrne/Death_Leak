#include "precompiled.h"
#include "Game.h"
#include "drawableobject.h"
#include "sprite.h"
#include "collisionManager.h"
#include "Environment.h"
#include "EffectParticleSpray.h"
#include "ParticleEmitterManager.h"
#include "Projectile.h"
#include "AudioManager.h"
#include "MaterialManager.h"
#include "DXWindow.h"
#include "UIManager.h"
#include "screenalignedtexture.h"
#include <time.h>
#include "Graphics.h"
#include "EffectBasic.h"
#include "EffectLightTextureVertexWobble.h"
#include "EffectReflection.h"
#include "player.h"
#include "particleSpray.h"
#include "effectbloodparticlespray.h"
#include "effectsepia.h"
#include "effectmonochrome.h"
#include "effectmonochromered.h"
#include "effectLightTextureBump.h"
#include "EffectNoise.h"
#include "LevelEditor.h"
#include "GamePad.h"
#include "WeatherManager.h"
#include "EffectLightTexturePixelWobble.h"
#include "EffectFoliageSway.h"
#include "Settings.h"
#include "SaveManager.h"
#include "DataValue.h"
#include "PlayerLevelManager.h"

Game * Game::mInstance = nullptr;

static const float kPixelWobbleReverseDelay = 40.0f;
static const float kPauseDamageEffectDelay = 0.05f;

bool Game::mPaused = false;
bool Game::mLevelEditMode = false;
Vector2 Game::mGameScale = Vector2(1.0f, 1.0f);

Game::Game(Graphics * pGraphics) : 
	m_pGraphics(pGraphics),
	m_pCam2d(nullptr),
	m_effectLightTexture(nullptr),
	m_effectLightTextureVertexWobble(nullptr),
	m_effectParticleSpray(nullptr),
	m_effectBloodParticleSpray(nullptr),
	m_effectLightTextureBump(nullptr),
	m_effectBasic(nullptr),
	mlevelEditor(nullptr),
	m_effectSepia(nullptr),
	m_effectMonochrome(nullptr),
	m_effectMonochromeRed(nullptr),
	m_screenAlignedPostProcTex1(nullptr),
	m_effectNoise(nullptr),
	m_effectPixelWobble(nullptr),
	m_effectFoliageSway(nullptr),
	mLastTimeDamagePauseEffect(0.0f)
{
}

Game::~Game(void)
{
}

void Game::Initialise()
{
	HRESULT hr = S_OK;

	// SaveManager::GetInstance()->ReadSaveFile();

	Settings::GetInstance()->ReadSettingsFile();

	// initialise audio
	AudioManager::Instance()->Initialise();

	m_pCam2d = new Camera2D(m_pGraphics->BackBufferWidth(), m_pGraphics->BackBufferHeight(), -8000, 0, 200);

	// initialise the texture manager
	TextureManager::Instance()->Initialise(m_pGraphics->Device());
	
	// initialise our shaders
	EffectManager::Instance()->Initialise(m_pGraphics);

	// update our effect pointers
	m_effectLightTexture = static_cast<EffectLightTexture*>(EffectManager::Instance()->GetEffect("effectlighttexture"));
	m_effectLightTextureVertexWobble = static_cast<EffectLightTextureVertexWobble*>(EffectManager::Instance()->GetEffect("effectlighttexturevertexwobble"));
	m_effectParticleSpray = static_cast<EffectParticleSpray*>(EffectManager::Instance()->GetEffect("effectparticlespray"));
	m_effectBloodParticleSpray = static_cast<EffectBloodParticleSpray*>(EffectManager::Instance()->GetEffect("effectbloodparticlespray"));
	m_effectBasic = static_cast<EffectBasic*>(EffectManager::Instance()->GetEffect("effectbasic"));
#if _DEBUG
	mlevelEditor = new LevelEditor();
#endif
	m_effectSepia = static_cast<EffectSepia*>(EffectManager::Instance()->GetEffect("effectsepia"));
	m_effectMonochrome = static_cast<EffectMonochrome*>(EffectManager::Instance()->GetEffect("effectmonochrome"));
	m_effectMonochromeRed = static_cast<EffectMonochromeRed*>(EffectManager::Instance()->GetEffect("effectmonochromered"));
	m_effectLightTextureBump = static_cast<EffectLightTextureBump*>(EffectManager::Instance()->GetEffect("effectlighttexturebump"));
	m_effectNoise = static_cast<EffectNoise*>(EffectManager::Instance()->GetEffect("effectnoise"));
	m_effectPixelWobble = static_cast<EffectLightTexturePixelWobble*>(EffectManager::Instance()->GetEffect("effectpixelwobble"));
	m_effectFoliageSway = static_cast<EffectFoliageSway*>(EffectManager::Instance()->GetEffect("effectfoliagesway"));

	m_effectNoise->SetSeed(5);
	m_effectLightTextureVertexWobble->SetWobbleIntensity(30);

	// initialise Materials
	MaterialManager::Instance()->Initialise("XmlFiles\\materials.xml");

	Settings::GetInstance()->ApplySettings();

	// initialise the collision manager, set to 1280 * 4 and 720 * 5 as default, is reset in gameobjectmanager load function
	CollisionManager::Instance()->Initialise(m_pGraphics->BackBufferWidth() * 4, m_pGraphics->BackBufferHeight() * 5, 8, 4);
	
	// initialise environment data
	Environment::Instance()->Initialise(-8000);

	// initialise the particle manager
	ParticleEmitterManager::Instance()->Initialise(m_pGraphics);

	// initialise the UI
	UIManager::Instance()->XmlRead("XmlFiles\\UI\\UI.xml"); // read in all of the UI components
	UIManager::Instance()->LoadContent(m_pGraphics);
	UIManager::Instance()->Initialise();

	// create a screen aligned texture for post processing 
	m_screenAlignedPostProcTex1 = new ScreenAlignedTexture();
	m_screenAlignedPostProcTex1->SetDimensions(m_pGraphics->BackBufferWidth(), m_pGraphics->BackBufferHeight());
	m_screenAlignedPostProcTex1->Initialise();

	mGameScale.X = (float)m_pGraphics->BackBufferWidth() / 1920.f;
	mGameScale.Y = (float)m_pGraphics->BackBufferHeight() / 1080.f;

	PlayerLevelManager::GetInstance()->Initialise();
}

void Game::Update(float delta)
{
	m_pCam2d->Update();

	AudioManager::Instance()->Update();

	bool damageEffectPauseActive = Timing::Instance()->GetTotalTimeSeconds() < (mLastTimeDamagePauseEffect + 
																			(kPauseDamageEffectDelay * Timing::Instance()->GetTimeModifier()));

	if (GameObjectManager::Instance()->IsLevelLoaded())
	{
		if (!mPaused)
		{
			// do collision detection
			if (!damageEffectPauseActive)
			{
				CollisionManager::Instance()->DetectAndResolve(m_pCam2d->X(), m_pCam2d->Y());
			}
			
			// NOTE: NEED to check input AFTER we do collision detection, MUST BE IN THIS ORDER

			// check input for player
			GameObjectManager::Instance()->CheckPlayerInput();
		}

		if (!mLevelEditMode)
		{
			if (!damageEffectPauseActive)
			{
				// update all of our game objects
				GameObjectManager::Instance()->Update(mPaused, delta);
			}
		}
		else
		{
			static bool hasUpdatedOnce = false;
			if (!hasUpdatedOnce)
			{
				// update all of our game objects once
				auto objects = GameObjectManager::Instance()->GetGameObjectList();
				for (auto & obj : objects)
				{
					obj->Update(delta);
				}
				hasUpdatedOnce = true;
			}

			if (mlevelEditor)
			{
				mlevelEditor->Update();
			}
		}

		// TODO: move to the check input function
		bool pressing_pause_gamepad = false;
		GamePad * game_pad = GamePad::GetPad1();
		if (game_pad && game_pad->IsConnected())
		{
			pressing_pause_gamepad = game_pad->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_START;
		}

		static bool pressingPause = false;
		if ((GetAsyncKeyState(VK_ESCAPE) < 0 || pressing_pause_gamepad) && !pressingPause)
		{
			pressingPause = true;

			if (mPaused)
			{
				UnPauseGame();
			}
			else
			{
				PauseGame();
			}
		}
		else if (!(GetAsyncKeyState(VK_ESCAPE) < 0) && !pressing_pause_gamepad)
		{
			pressingPause = false;
		}
	}
	
	// update the UI
	UIManager::Instance()->Update();
	UIManager::Instance()->HandleEvents();
}

void Game::PauseGame()
{
	mPaused = true;
	UIManager::Instance()->PushUI("pause_menu");
}
void Game::UnPauseGame()
{
	mPaused = false;
	UIManager::Instance()->PopUI("pause_menu");
}

void Game::Draw()
{
	// update our effect variables
	D3DXMATRIX camWorld = m_pCam2d->World();
	D3DXMATRIX camView = m_pCam2d->View();
	D3DXMATRIX camProjection = m_pCam2d->Projection();

	m_effectLightTexture->SetWorldViewProjection((float*)camWorld,(float*)camView, (float*)camProjection);
	m_effectLightTextureVertexWobble->SetWorldViewProjection((float*)camWorld,(float*)camView, (float*)camProjection);
	m_effectParticleSpray->SetWorldViewProjection((float*)camWorld,(float*)camView, (float*)camProjection);
	m_effectBloodParticleSpray->SetWorldViewProjection((float*)camWorld,(float*)camView, (float*)camProjection);
	m_effectLightTextureBump->SetWorldViewProjection((float*)camWorld,(float*)camView, (float*)camProjection);
	m_effectLightTextureBump->SetLightColor((float*)D3DXVECTOR4(1.0,1.0,1.0,1.0f));
	m_effectNoise->SetWorldViewProjection((float*)camWorld,(float*)camView, (float*)camProjection);
	m_effectPixelWobble->SetWorldViewProjection((float*)camWorld,(float*)camView, (float*)camProjection);
	m_effectFoliageSway->SetWorldViewProjection((float*)camWorld, (float*)camView, (float*)camProjection);

	float shaderTime = Timing::Instance()->GetTotalTimeSeconds();

	m_effectLightTextureVertexWobble->SetTimeVariable(shaderTime);
	m_effectNoise->SetTimer(shaderTime);
	m_effectFoliageSway->SetTimeVariable(shaderTime);

	m_effectFoliageSway->SetGlobalTimeMultiplier(WeatherManager::GetInstance()->GetFoliageSwayMultiplier());

	static float pixelWobbleShaderTime = 0;
	static bool inReverse = false;

	if (!inReverse)
	{
		pixelWobbleShaderTime += Timing::Instance()->GetLastUpdateDelta();
		if (pixelWobbleShaderTime > kPixelWobbleReverseDelay)
		{
			m_effectPixelWobble->ReverseMotion();
			inReverse = true;
		}
	}
	else
	{
		pixelWobbleShaderTime -= Timing::Instance()->GetLastUpdateDelta();
		if (pixelWobbleShaderTime < -kPixelWobbleReverseDelay)
		{
			m_effectPixelWobble->ReverseMotion();
			inReverse = false;
		}
	}

	m_effectPixelWobble->SetTimeVariable(pixelWobbleShaderTime);

#if _DEBUG
	m_effectBasic->SetWorldViewProjection((float*)camWorld,(float*)camView, (float*)camProjection);
#endif

	// draw all of our gameObjects
	GameObjectManager::Instance()->Draw(m_pGraphics->Device());

#if _DEBUG
	GameObjectManager::Instance()->DebugDraw();

	if (mLevelEditMode && mlevelEditor)
	{
		mlevelEditor->Draw();
	}
#endif
}

void Game::PostDraw() // post processsing effects here
{
	m_pGraphics->DisableAlphaBlending();

	m_effectLightTexture->SetTexture(m_pGraphics->GetPreProcessSRV());
	// m_effectLightTextureVertexWobble->SetTexture(m_pGraphics->GetPreProcessSRV());
	// m_effectSepia->SetTexture(m_pGraphics->GetPreProcessSRV());
	// m_effectMonochrome->SetTexture(m_pGraphics->GetPreProcessSRV());
	// m_effectMonochromeRed->SetTexture(m_pGraphics->GetPreProcessSRV());

	// draw the scene to a texture
	m_screenAlignedPostProcTex1->SetEffect(m_effectLightTexture);
	//m_screenAlignedPostProcTex1->SetEffect(m_effectLightTextureVertexWobble);
	//m_screenAlignedPostProcTex1->SetEffect(m_effectSepia);
	//m_screenAlignedPostProcTex1->SetEffect(m_effectMonochrome);
	// m_screenAlignedPostProcTex1->SetEffect(m_effectMonochromeRed);

	m_screenAlignedPostProcTex1->Draw();


	// we need to unbind the texture as we will be writing to it on the next frame
	m_effectLightTexture->SetTexture(NULL);
	m_effectLightTexture->CurrentTechnique->GetPassByIndex(0)->Apply(0);

	//m_effectSepia->SetTexture(nullptr);
	//m_effectSepia->CurrentTechnique->GetPassByIndex( 0 )->Apply(0);

	//m_effectMonochrome->SetTexture(NULL);
	//m_effectMonochrome->CurrentTechnique->GetPassByIndex( 0 )->Apply(0);

	// m_effectMonochromeRed->SetTexture(nullptr);
	// m_effectMonochromeRed->CurrentTechnique->GetPassByIndex( 0 )->Apply(0);

	//m_effectLightTextureVertexWobble->SetTexture(NULL);
	//m_effectLightTextureVertexWobble->CurrentTechnique->GetPassByIndex( 0 )->Apply(0);
	
	// re-enable alpha blending for drawing the UI
	m_pGraphics->EnableAlphaBlending();
}

void Game::Cleanup()
{
	// delete all of our game objects
	GameObjectManager::Instance()->DeleteGameObjects();

	// delete our textures
	TextureManager::Instance()->Release();
	
	// delete our effects
	EffectManager::Instance()->Release();

	// cleanup audio
	AudioManager::Instance()->Release();

	// cleanup materials
	MaterialManager::Instance()->Release();

	// cleanup UI
	UIManager::Instance()->Release();
	
	// release the screen aligned texture
	m_screenAlignedPostProcTex1->Release();
}

void Game::SetLevelEditFilename(const char * file) 
{ 
	GAME_ASSERT(mlevelEditor);
	if (mlevelEditor)
	{
		mlevelEditor->SetLevelFile(file);
	}
}

void Game::Create()
{
	GAME_ASSERT(!mInstance);

	mInstance = new Game(Graphics::GetInstance());
}

void Game::Destroy()
{
	GAME_ASSERT(mInstance);

	if (mInstance)
	{
		mInstance->Cleanup();
	}

	delete mInstance;
	mInstance = nullptr;
}

bool Game::IsLevelEditTerrainMode() const
{
	if (mlevelEditor && mlevelEditor->IsTerrainEditing())
	{
		return true;
	}

	return false;
}

void Game::ResetLevelEditor()
{
	if (mlevelEditor)
	{
		mlevelEditor->Reset();
	}
}

void Game::DoDamagePauseEffect()
{
	mLastTimeDamagePauseEffect = Timing::Instance()->GetTotalTimeSeconds();
}


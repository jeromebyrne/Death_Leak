#include "precompiled.h"
#include "Game.h"
#include "drawableobject.h"
#include "sprite.h"
#include "collisionManager.h"
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
#include "FeatureUnlockManager.h"
#include "UITextModal.h"
#include "UIUpgradeModal.h"
#include "NinjaSpawner.h"

Game * Game::mInstance = nullptr;

static const float kPixelWobbleReverseDelay = 10.0f;
static const float kPauseDamageEffectDelay = 0.04f;
static const float kPauseDamageEffectLongerDelay = 0.2f;

bool Game::mPaused = false;
bool Game::mLevelEditMode = false;
bool Game::mIsDisplayingTextModal = false;

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

	SaveManager::GetInstance()->ReadSaveFile();

	mGOMInstance = GameObjectManager::Instance();
	mUIManagerInstance = UIManager::Instance();

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

	m_effectNoise->SetSeed(98765);
	m_effectLightTextureVertexWobble->SetWobbleIntensity(30.0f);

	// initialise Materials
	MaterialManager::Instance()->Initialise("XmlFiles\\materials.xml");

	Settings::GetInstance()->ApplySettings();

	// initialise the collision manager, set to 1280 * 4 and 720 * 5 as default, is reset in gameobjectmanager load function
	CollisionManager::Instance()->Initialise(m_pGraphics->BackBufferWidth() * 4, m_pGraphics->BackBufferHeight() * 5, 8, 4);

	// initialise the particle manager
	ParticleEmitterManager::Instance()->Initialise(m_pGraphics);

	// initialise the UI
	mUIManagerInstance->XmlRead("XmlFiles\\UI\\UI.xml"); // read in all of the UI components
	mUIManagerInstance->LoadContent(m_pGraphics);
	mUIManagerInstance->Initialise();

	// create a screen aligned texture for post processing 
	m_screenAlignedPostProcTex1 = new ScreenAlignedTexture();
	m_screenAlignedPostProcTex1->SetDimensions(m_pGraphics->BackBufferWidth(), m_pGraphics->BackBufferHeight());
	m_screenAlignedPostProcTex1->Initialise();
	m_screenAlignedPostProcTex1->SetEffect(m_effectSepia);

	mGameScale.X = (float)m_pGraphics->BackBufferWidth() / 1920.f;
	mGameScale.Y = (float)m_pGraphics->BackBufferHeight() / 1080.f;

	PlayerLevelManager::GetInstance()->Initialise();

	FeatureUnlockManager::GetInstance()->Initialise();

	STEAM_CALLBACK(Game, OnSteamGameOverlayActivated, GameOverlayActivated_t);

	LoadCachedObjectsForPerformance();
}

void Game::Update(float delta)
{
	m_pCam2d->CheckBoundaryCollisions();

	if (!mPaused)
	{
		m_pCam2d->FollowTargetObjectWithLag();

		m_pCam2d->CheckBoundaryCollisions();
	}

	AudioManager::Instance()->Update();

	bool damageEffectPauseActive = Timing::Instance()->GetTotalTimeSeconds() < (mLastTimeDamagePauseEffect + 
																			(mPauseEffectDelay * Timing::Instance()->GetTimeModifier()));

	if (mGOMInstance->IsLevelLoaded())
	{
#if _DEBUG
		if (!mLevelEditMode)
		{
#endif
			if (!damageEffectPauseActive)
			{
				// update all of our game objects
				mGOMInstance->Update(mPaused, delta);
			}
#if _DEBUG
		}
		else
		{
			static bool hasUpdatedOnce = false;
			if (!hasUpdatedOnce)
			{
				// update all of our game objects once
				auto objects = mGOMInstance->GetGameObjectList();
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
#endif

		if (!mPaused)
		{
			// do collision detection
			if (!damageEffectPauseActive)
			{
				CollisionManager::Instance()->DetectAndResolve((int)m_pCam2d->X(), (int)m_pCam2d->Y());
			}
			
			mGOMInstance->PostUpdate(mPaused, delta);

			// NOTE: NEED to check input AFTER we do collision detection, MUST BE IN THIS ORDER
			mInputManager.ProcessGameplayInput();
		}

		if (mIsDisplayingTextModal == false)
		{
			bool pressing_pause_gamepad = false;
			GamePad * game_pad = GamePad::GetPad1();
			if (game_pad && game_pad->IsConnected())
			{
				pressing_pause_gamepad = game_pad->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_START;
			}

			static bool pressingPause = false;
			if ((GetAsyncKeyState(VK_ESCAPE) < 0 || pressing_pause_gamepad) && !pressingPause)
			{
				Player * player = GameObjectManager::Instance()->GetPlayer();

				if (player && !player->IsDead())
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
			}
			else if (!(GetAsyncKeyState(VK_ESCAPE) < 0) && !pressing_pause_gamepad)
			{
				pressingPause = false;
			}
		}
	}
	
	// update the UI
	mUIManagerInstance->Update();
	mUIManagerInstance->HandleEvents();
	
	// m_pCam2d->CheckBoundaryCollisions();

	m_pCam2d->Update();

	mInputManager.Update(delta);
}

void Game::Vibrate(float leftPercent, float rightPercent, float time)
{
	mInputManager.Vibrate(leftPercent, rightPercent, time);
}

void Game::PauseGame(bool pushPauseScreen)
{
	if (mIsDisplayingTextModal)
	{
		return;
	}

	Player* player = GameObjectManager::Instance()->GetPlayer();

	if (player)
	{
		if (player->IsInFinalLevel())
		{
			return;
		}
	}

	mPaused = true;

	if (pushPauseScreen)
	{
		UIManager::Instance()->PushUI("pause_menu");
	}
}
void Game::UnPauseGame(bool popPauseScreen )
{
	if (mIsDisplayingTextModal)
	{
		return;
	}

	mPaused = false;

	if (popPauseScreen)
	{
		UIManager::Instance()->PopUI("pause_menu");
	}
}

void Game::DisplayTextModal(const string & localizedTitle, const string & localizedText)
{
	mPaused = true;

	UIScreen * screen = UIManager::Instance()->PushUI("text_modal");
	UITextModal * textScreen = static_cast<UITextModal*>(screen);

	textScreen->SetLocalizedTitle(localizedTitle);
	textScreen->SetLocalizedDescription(localizedText);

	mIsDisplayingTextModal = true;
}

void Game::DisplayUpgradeModal(const string& localizedTitle, const string& localizedText, FeatureUnlockManager::FeatureType upgradeType, int upgradeCost)
{
	mPaused = true;

	UIScreen* screen = UIManager::Instance()->PushUI("upgrade_modal");
	UIUpgradeModal* upgradeScreen = static_cast<UIUpgradeModal*>(screen);

	upgradeScreen->SetLocalizedTitle(localizedTitle);
	upgradeScreen->SetLocalizedDescription(localizedText);
	upgradeScreen->SetUpgradeType(upgradeType);
	upgradeScreen->SetUpgradeCost(upgradeCost);

	mIsDisplayingTextModal = true;
}

void Game::DismissTextModal()
{
	mPaused = false;

	UIManager::Instance()->PopUIDeferred("text_modal");

	mIsDisplayingTextModal = false;
}

void Game::DismissUpgradeModal()
{
	mPaused = false;

	UIManager::Instance()->PopUIDeferred("upgrade_modal");

	mIsDisplayingTextModal = false;
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
		pixelWobbleShaderTime += (float)Timing::Instance()->GetLastUpdateDelta();
		if (pixelWobbleShaderTime > kPixelWobbleReverseDelay)
		{
			m_effectPixelWobble->ReverseMotion();
			inReverse = true;
		}
	}
	else
	{
		pixelWobbleShaderTime -= (float)Timing::Instance()->GetLastUpdateDelta();
		if (pixelWobbleShaderTime < kPixelWobbleReverseDelay * 0.5f)
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
	mGOMInstance->Draw(m_pGraphics->Device());

#if _DEBUG

	if (mInputManager.ShowDebugInfoEnabled())
	{
		mGOMInstance->DebugDraw();
	}

	if (mLevelEditMode && mlevelEditor)
	{
		mlevelEditor->Draw();
	}
	
	// Vector3 camPos = Camera2D::GetInstance()->Position();
	// Graphics::GetInstance()->DrawDebugText(Utilities::getFormattedString("Cam X,Y: %f %f", camPos.X, camPos.Y).c_str(), 100, 400);

#endif
}

void Game::PostDraw() // post processsing effects here
{
	// m_pGraphics->DisableAlphaBlending();

	// m_effectLightTexture->SetTexture(m_pGraphics->GetPreProcessSRV());
	// m_effectLightTextureVertexWobble->SetTexture(m_pGraphics->GetPreProcessSRV());
	m_effectSepia->SetTexture(m_pGraphics->GetPreProcessSRV());
	// m_effectMonochrome->SetTexture(m_pGraphics->GetPreProcessSRV());
	// m_effectMonochromeRed->SetTexture(m_pGraphics->GetPreProcessSRV());

	// draw the scene to a texture
	//m_screenAlignedPostProcTex1->SetEffect(m_effectLightTexture);
	//m_screenAlignedPostProcTex1->SetEffect(m_effectLightTextureVertexWobble);
	m_screenAlignedPostProcTex1->SetEffect(m_effectSepia);
	// m_screenAlignedPostProcTex1->SetEffect(m_effectMonochrome);
	// m_screenAlignedPostProcTex1->SetEffect(m_effectMonochromeRed);

	m_screenAlignedPostProcTex1->Draw();


	// we need to unbind the texture as we will be writing to it on the next frame
	//m_effectLightTexture->SetTexture(NULL);
	//m_effectLightTexture->CurrentTechnique->GetPassByIndex(0)->Apply(0);

	m_effectSepia->SetTexture(nullptr);
	m_effectSepia->CurrentTechnique->GetPassByIndex( 0 )->Apply(0);

	// m_effectMonochrome->SetTexture(NULL);
	// m_effectMonochrome->CurrentTechnique->GetPassByIndex( 0 )->Apply(0);

	// m_effectMonochromeRed->SetTexture(nullptr);
	// m_effectMonochromeRed->CurrentTechnique->GetPassByIndex( 0 )->Apply(0);

	//m_effectLightTextureVertexWobble->SetTexture(NULL);
	//m_effectLightTextureVertexWobble->CurrentTechnique->GetPassByIndex( 0 )->Apply(0);
	
	// re-enable alpha blending for drawing the UI
	// m_pGraphics->EnableAlphaBlending();
}

void Game::Cleanup()
{
	// delete all of our game objects
	mGOMInstance->DeleteGameObjects();

	// delete our textures
	TextureManager::Instance()->Release();
	
	// delete our effects
	EffectManager::Instance()->Release();

	// cleanup audio
	AudioManager::Instance()->Release();

	// cleanup materials
	MaterialManager::Instance()->Release();

	// cleanup UI
	mUIManagerInstance->Release();
	
	// release the screen aligned texture
	m_screenAlignedPostProcTex1->Release();
}

void Game::SetLevelEditFilename(const string & file) 
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

void Game::ResetLevelEditorSelectedObject()
{
	if (mlevelEditor)
	{
		mlevelEditor->ResetSelectedObject();
	}
}

void Game::DoDamagePauseEffect()
{
	if (!mDamagePauseEnabled)
	{
		return;
	}
	mPauseEffectDelay = kPauseDamageEffectDelay;
	mLastTimeDamagePauseEffect = Timing::Instance()->GetTotalTimeSeconds();
}

void Game::DoDamagePauseEffectLonger()
{
	if (!mDamagePauseEnabled)
	{
		return;
	}
	mPauseEffectDelay = kPauseDamageEffectLongerDelay;
	mLastTimeDamagePauseEffect = Timing::Instance()->GetTotalTimeSeconds();
}

void Game::OnSteamGameOverlayActivated(GameOverlayActivated_t* pCallback)
{
	mPaused = pCallback->m_bActive;
}

void Game::LoadCachedObjectsForPerformance()
{
	if (GameObjectManager::Instance() == nullptr)
	{
		return;
	}

	// the following loads anims from disk and caches them
	Animation * preloadAnim = new Animation("XmlFiles\\animation\\ninjaAnimation.xml");
	delete preloadAnim;
	preloadAnim = new Animation("XmlFiles\\animation\\ghost_enemy_anim.xml");
	delete preloadAnim;
	preloadAnim = new Animation("XmlFiles\\animation\\player_anim_default.xml");
	delete preloadAnim;
	preloadAnim = new Animation("XmlFiles\\bird_anim.xml");
	delete preloadAnim;
	preloadAnim = new Animation("XmlFiles\\bird_anim_2.xml");
	delete preloadAnim;
	preloadAnim = new Animation("XmlFiles\\butterfly_anim.xml");
	delete preloadAnim;
	preloadAnim = new Animation("XmlFiles\\crate_anim.xml");
	delete preloadAnim;
	preloadAnim = new Animation("XmlFiles\\orb_anim.xml");
	delete preloadAnim;
	preloadAnim = new Animation("XmlFiles\\pot_anim.xml");
	delete preloadAnim;
	preloadAnim = new Animation("XmlFiles\\rat_anim.xml");
	delete preloadAnim;
	preloadAnim = new Animation("XmlFiles\\small_rat_anim.xml");
	delete preloadAnim;
	preloadAnim = new Animation("XmlFiles\\stone_smashable.xml");
	delete preloadAnim;
}


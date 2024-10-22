#include "precompiled.h"
#include "HealthDevil.h"
#include "ParticleEmitterManager.h"
#include "AudioManager.h"
#include "SaveManager.h"
#include "HealthUpgradePickup.h"
#include "Game.h"
#include "FocusUpgradePickup.h"
#include "KeyPickup.h"

extern CSteamAchievements* g_SteamAchievements;

HealthDevil::HealthDevil(float x, 
	float y, 
	DepthLayer 
	depthLayer, 
	float width, 
	float height):
	Sprite(x,y, depthLayer, width, height)
{
}

HealthDevil::~HealthDevil(void)
{
	if (mVoiceOverSoundPlaying != nullptr)
	{
		mVoiceOverSoundPlaying->drop();
		mVoiceOverSoundPlaying = nullptr;
	}
}

void HealthDevil::Update(float delta)
{	
	Sprite::Update(delta);

	if (mHasCreatedParticles == false)
	{
		auto pos = Position();
		pos.Y += 60.0f;
		mParticleSpray = ParticleEmitterManager::Instance()->CreateRadialSpray(60,
																pos,
																GameObject::kGround,
																Vector2(3200.0f, 2000.0f),
																"Media\\characters\\health_devil\\shroud_particle.png",
																0.05f,
																0.4f,
																3.3f,
																8.6f,
																300.0f,
																500.0f,
																0.0f,
																true,
																0.2f,
																0.6f,
																-1.0f,
																true,
																0.4f,
																0.1f,
																0.8f,
																12.0f,
																6.0f);

		mHasCreatedParticles = true;
	}

	if (mHasGivenReward)
	{
		// Don't show the eyes if we've given the reward
		m_alpha = 0.0f;
		return;
	}

	if (mHasPlayedDialog && mVoiceOverSoundPlaying != nullptr)
	{
		if (mVoiceOverSoundPlaying->isFinished() || !AudioManager::Instance()->AreSFXenabled())
		{
			// we should do reward now
			GiveReward();

			if (mVoiceOverSoundPlaying != nullptr)
			{
				mVoiceOverSoundPlaying->drop();
				mVoiceOverSoundPlaying = nullptr;
			}

			if (mParticleSpray != nullptr)
			{
				mParticleSpray->SetIsLooping(false);
			}
		}
		else
		{
			m_alpha = 1.0f;
			// shake the camera while talking
			// Camera2D::GetInstance()->DoSmallShake();
		}
	}
}

void HealthDevil::Initialise()
{
	Sprite::Initialise();

	mInteractableProperties.IsInteractable = true;
	mInteractableProperties.PosOffset = Vector2(-10, -200);

	string levelId = GameObjectManager::Instance()->GetCurrentLevelFile();

	mHasGivenReward = SaveManager::GetInstance()->HasHealthDevilGivenReward(levelId);

	if (mHasGivenReward)
	{
		// don't play the siren audio
		if (mPositionalAudioEnabled)
		{
			mPositionalAudio.Disable();
		}
	}

	m_alpha = 0.0f;
}

void HealthDevil::XmlRead(TiXmlElement * element)
{
	Sprite::XmlRead(element);

	// m_applyGravity = XmlUtilities::ReadAttributeAsBool(element, "applygravity", "value");
}

void HealthDevil::XmlWrite(TiXmlElement * element)
{
	Sprite::XmlWrite(element);

	/*
	TiXmlElement * resistanceElem = new TiXmlElement("resistance");
	resistanceElem->SetDoubleAttribute("y", m_resistance.Y);
	resistanceElem->SetDoubleAttribute("x", m_resistance.X);
	element->LinkEndChild(resistanceElem);
	*/
}

void HealthDevil::GiveReward()
{
	int rewardIndex = SaveManager::GetInstance()->GetHealthDevilRewardCount();

	GiveHealthUpgradeReward();

	SaveManager::GetInstance()->SetHealthDevilRewardCount(rewardIndex + 1);

	mHasGivenReward = true;

	string levelId = GameObjectManager::Instance()->GetCurrentLevelFile();
	SaveManager::GetInstance()->SetHealthDevilGivenReward(levelId, true);
}

void HealthDevil::GiveHealthUpgradeReward()
{
	HealthUpgradePickup * huPickup = new HealthUpgradePickup();
	huPickup->SetTextureFilename("Media\\objects\\health_upgrade.png");
	huPickup->SetIsNativeDimensions(true);
	huPickup->SetCollisionDimensions(Vector2(50.0f, 70.0f));
	huPickup->SetApplyGravity(true);
	huPickup->SetGravityApplyAmount(0.8f);
	huPickup->SetUpdateable(true);
	huPickup->SetPassive(false);
	huPickup->SetXY(m_position.X, m_position.Y + 20.0f); // spawn above the health devil
	huPickup->SetMaxVelocityXY(30.0f, 20.0f);
	huPickup->SetVelocityXY(15.0f, 10.0f);
	huPickup->SetResistanceXY(0.95f, 1.0f);
	huPickup->EffectName = "effectlighttexture";
	
	GameObjectManager::Instance()->AddGameObject(huPickup);
}

void HealthDevil::GiveFocusUpgradeReward()
{
	FocusUpgradePickup * fuPickup = new FocusUpgradePickup();
	fuPickup->SetTextureFilename("Media\\objects\\focus_upgrade.png");
	fuPickup->SetIsNativeDimensions(true);
	fuPickup->SetCollisionDimensions(Vector2(50.0f, 70.0f));
	fuPickup->SetApplyGravity(true);
	fuPickup->SetGravityApplyAmount(0.8f);
	fuPickup->SetUpdateable(true);
	fuPickup->SetPassive(false);
	fuPickup->SetXY(m_position.X, m_position.Y + 20.0f); // spawn above the health devil
	fuPickup->SetMaxVelocityXY(30.0f, 20.0f);
	fuPickup->SetVelocityXY(15.0f, 10.0f);
	fuPickup->SetResistanceXY(0.95f, 1.0f);
	fuPickup->EffectName = "effectlighttexture";

	GameObjectManager::Instance()->AddGameObject(fuPickup);
}

void HealthDevil::GiveKeyRewardTest()
{
	KeyPickup * keyPickup = new KeyPickup();
	keyPickup->SetTextureFilename("Media\\keys\\key_1.png");
	keyPickup->SetIsNativeDimensions(true);
	keyPickup->SetCollisionDimensions(Vector2(20.0f, 30.0f));
	keyPickup->SetApplyGravity(true);
	keyPickup->SetGravityApplyAmount(1.0f);
	keyPickup->SetUpdateable(true);
	keyPickup->SetPassive(false);
	keyPickup->SetXY(m_position.X, m_position.Y + 20.0f); // spawn above the health devil
	keyPickup->SetMaxVelocityXY(30.0f, 30.0f);
	keyPickup->SetVelocityXY(15.0f, 20.0f);
	keyPickup->SetResistanceXY(0.95f, 1.0f);
	keyPickup->EffectName = "effectlighttexture";

	keyPickup->SetKeyId("key_1");

	GameObjectManager::Instance()->AddGameObject(keyPickup);
}

void HealthDevil::OnInteracted()
{
	// play audio, after audio finishes the reward will be given
	if (!mHasPlayedDialog)
	{
		Camera2D::GetInstance()->DoBigShake();
		mPositionalAudio.SetRepeat(false);

		int rewardIndex = SaveManager::GetInstance()->GetHealthDevilRewardCount();

		if (rewardIndex == 0)
		{
			mVoiceOverSoundPlaying = AudioManager::Instance()->PlaySoundEffect("character\\health_devil\\vo\\devil_1_fx.wav", false, true);
		}
		else if (rewardIndex == 1)
		{
			mVoiceOverSoundPlaying = AudioManager::Instance()->PlaySoundEffect("character\\health_devil\\vo\\devil_1_fx.wav", false, true);
		}
		else
		{
			mVoiceOverSoundPlaying = AudioManager::Instance()->PlaySoundEffect("character\\health_devil\\vo\\devil_3_fx.wav", false, true);

			if ( g_SteamAchievements)
				g_SteamAchievements->SetAchievement("ACH_THREE_DEVILS");
		}
		
		mHasPlayedDialog = true;

		Game::GetInstance()->Vibrate(1.0f, 1.0f, 0.4f);
	}
}

bool HealthDevil::CanInteract()
{
	if (mHasGivenReward)
	{
		return false;
	}

	Player * player = GameObjectManager::Instance()->GetPlayer();
	if (player == nullptr)
	{
		return false;
	}

	if (player->IsOnSolidSurface())
	{
		return true;
	}

	return false;
}

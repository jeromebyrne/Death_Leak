#include "precompiled.h"
#include "Boat.h"
#include "InputManager.h"
#include "Game.h"
#include "AudioManager.h"
#include "SaveManager.h"
#include "ParticleEmitterManager.h"
#include "DrawUtilities.h"
#include "StringManager.h"
#include "UImanager.h"

static const float kWarmUpTime = 0.75f;
static const string kDefaultDoorLockedSfx = "door_locked.wav";
static const string kDefaultDoorUnlockedSfx = "door_unlock.wav";

static const float kUnlockingTime = 1.0f;

Boat::Boat() :
	Sprite()
{
}

Boat::~Boat()
{
}

void Boat::Initialise()
{
	Sprite::Initialise();

	mInteractableProperties.IsInteractable = true;
	mInteractableProperties.PosOffset = Vector2(0.0f, 30.0f);
	mInteractableProperties.DisableInteractivityOnInteract = false;
	mInteractableProperties.InteractTime = SaveManager::GetInstance()->HasRepairTools() ? 3.0f : 0.05f;

	mBoatBrokenMessageTitle = StringManager::GetInstance()->GetLocalisedString("broken_boat_message_title");
	mBoatBrokenMessageDesc = StringManager::GetInstance()->GetLocalisedString("broken_boat_message_desc");
}

void Boat::Update(float delta)
{
	Sprite::Update(delta);

	if (mBoatWarmUpTime < kWarmUpTime)
	{
		mBoatWarmUpTime += delta;
		return;
	}

	if (mIsUnlockingCurrentTime > 0.0f)
	{
		mIsUnlockingCurrentTime -= delta;
	}
}
void Boat::XmlRead(TiXmlElement * element)
{
	Sprite::XmlRead(element);

	// mRequiredKey = XmlUtilities::ReadAttributeAsString(element, "required_key", "value");

	// mDoorOpenSFX = XmlUtilities::ReadAttributeAsString(element, "door_sfx", "open");
	// mDoorLockedSFX = XmlUtilities::ReadAttributeAsString(element, "door_sfx", "locked");
}

void Boat::XmlWrite(TiXmlElement * element)
{
	Sprite::XmlWrite(element);


	/*
	TiXmlElement * requiredKey = new TiXmlElement("required_key");
	requiredKey->SetAttribute("value", mRequiredKey.c_str());
	element->LinkEndChild(requiredKey);

	
	TiXmlElement * doorSFX = new TiXmlElement("door_sfx");
	doorSFX->SetAttribute("open", mDoorOpenSFX.c_str());
	doorSFX->SetAttribute("locked", mDoorLockedSFX.c_str());
	element->LinkEndChild(doorSFX);
	*/
}

void Boat::EnterBoat()
{
	// pause when we open a door so as not to get hit by projectiles etc...
	// Game::GetInstance()->PauseGame();

	// GameObjectManager::Instance()->SwitchToLevel(mToLevelFile, mDoorIdentifier, true);

	GameObjectManager::Instance()->SwitchToLevel("XmlFiles\\levels\\sea_2.xml", "", true);
	UIManager::Instance()->PopUI("game_hud");
	UIManager::Instance()->PushUI("final_scene_hud");
}

void Boat::OnInteracted()
{
	mCanTryOpen = false;

	if (!SaveManager::GetInstance()->HasRepairTools())
	{
		// show message about broken boat
		Game::GetInstance()->DisplayTextModal(mBoatBrokenMessageTitle, mBoatBrokenMessageDesc);

		/*
		if (!SaveManager::GetInstance()->HasDoorKey(mRequiredKey))
		{
			if (!mDoorLockedSFX.empty())
			{
				AudioManager::Instance()->PlaySoundEffect(mDoorLockedSFX);
			}
			else
			{
				// play default locked sfx
				AudioManager::Instance()->PlaySoundEffect(kDefaultDoorLockedSfx);
			}

			mInteractableProperties.InteractCountdown = 1.0f;
			Camera2D::GetInstance()->DoSmallShake();
			Game::GetInstance()->Vibrate(0.0f, 0.6f, 0.25f);
			mCanTryOpen = true;
			DisplayDoorLockedEffect();
			return;
		}
		else if (!SaveManager::GetInstance()->DoorWasUnlocked(mDoorIdentifier))
		{
			// TODO: do effects
			AudioManager::Instance()->PlaySoundEffect(kDefaultDoorUnlockedSfx);
			SaveManager::GetInstance()->SetDoorWasUnlocked(mDoorIdentifier, true);
			mInteractableProperties.InteractCountdown = 0.4f;
			mCanTryOpen = true;
			mIsUnlockingCurrentTime = kUnlockingTime;

			Game::GetInstance()->Vibrate(0.2f, 0.1f, 0.2f);

			return;
		}
		*/

		return;
	}

	EnterBoat();
}

bool Boat::CanInteract()
{
	/*
	if (!SaveManager::GetInstance()->HasRepairTools())
	{
		return false;
	}
	*/

	if (mIsUnlockingCurrentTime > 0.0f)
	{
		return false;
	}

	Player * player = GameObjectManager::Instance()->GetPlayer();
	if (player == nullptr)
	{
		return false;
	}

	if (mBoatWarmUpTime < kWarmUpTime)
	{
		return false;
	}

	if (mCanTryOpen &&
		player->IsOnSolidSurface())
	{
		return true;
	}		

	return false;
}

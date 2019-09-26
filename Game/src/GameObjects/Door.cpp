#include "precompiled.h"
#include "Door.h"
#include "InputManager.h"
#include "Game.h"
#include "AudioManager.h"
#include "SaveManager.h"
#include "ParticleEmitterManager.h"

static const float kWarmUpTime = 0.75f;
static const string kDefaultDoorLockedSfx = "door_locked.wav";
static const string kDefaultDoorUnlockedSfx = "door_unlock.wav";

static const float kUnlockingTime = 1.0f;

Door::Door() :
	Sprite()
{
}

Door::~Door()
{
}

void Door::Initialise()
{
	Sprite::Initialise();

	mInteractableProperties.IsInteractable = true;
	mInteractableProperties.PosOffset = Vector2(0.0f, 30.0f);
	mInteractableProperties.DisableInteractivityOnInteract = false;
}

void Door::Update(float delta)
{
	Sprite::Update(delta);

	if (mDoorWarmUpTime < kWarmUpTime)
	{
		mDoorWarmUpTime += delta;
		return;
	}

	if (mIsUnlockingCurrentTime > 0.0f)
	{
		mIsUnlockingCurrentTime -= delta;
	}
}
void Door::XmlRead(TiXmlElement * element)
{
	Sprite::XmlRead(element);

	mToLevelFile = XmlUtilities::ReadAttributeAsString(element, "to_level_file", "value");

	mToLevelPosition.X = XmlUtilities::ReadAttributeAsFloat(element, "player_start_pos", "x");
	mToLevelPosition.Y = XmlUtilities::ReadAttributeAsFloat(element, "player_start_pos", "y");

	mRequiredKey = XmlUtilities::ReadAttributeAsString(element, "required_key", "value");

	mDoorOpenSFX = XmlUtilities::ReadAttributeAsString(element, "door_sfx", "open");
	mDoorLockedSFX = XmlUtilities::ReadAttributeAsString(element, "door_sfx", "locked");

	mDoorIdentifier = XmlUtilities::ReadAttributeAsString(element, "door_id", "value");
}

void Door::XmlWrite(TiXmlElement * element)
{
	Sprite::XmlWrite(element);

	TiXmlElement * levelFile = new TiXmlElement("to_level_file");
	levelFile->SetAttribute("value", mToLevelFile.c_str());
	element->LinkEndChild(levelFile);

	TiXmlElement * posElem = new TiXmlElement("player_start_pos");
	posElem->SetDoubleAttribute("x", mToLevelPosition.X);
	posElem->SetDoubleAttribute("y", mToLevelPosition.Y);
	element->LinkEndChild(posElem);

	TiXmlElement * requiredKey = new TiXmlElement("required_key");
	requiredKey->SetAttribute("value", mRequiredKey.c_str());
	element->LinkEndChild(requiredKey);

	TiXmlElement * doorSFX = new TiXmlElement("door_sfx");
	doorSFX->SetAttribute("open", mDoorOpenSFX.c_str());
	doorSFX->SetAttribute("locked", mDoorLockedSFX.c_str());
	element->LinkEndChild(doorSFX);

	TiXmlElement * doorId = new TiXmlElement("door_id");
	doorId->SetAttribute("value", mDoorIdentifier.c_str());
	element->LinkEndChild(doorId);
}

void Door::EnterDoor()
{
	// pause when we open a door so as not to get hit by projectiles etc...
	// Game::GetInstance()->PauseGame();

	if (!mDoorOpenSFX.empty())
	{
		AudioManager::Instance()->PlaySoundEffect(mDoorOpenSFX);
	}

	GAME_ASSERT(mDoorIdentifier.empty() == false);
	GameObjectManager::Instance()->SwitchToLevel(mToLevelFile, mDoorIdentifier, true);
}

void Door::OnInteracted()
{
	mCanTryOpen = false;

	if (!mRequiredKey.empty())
	{
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
				mInteractableProperties.InteractCountdown = 1.0f;

				Camera2D::GetInstance()->DoSmallShake();
			}
			mCanTryOpen = true;
			return;
		}
		else if (!SaveManager::GetInstance()->DoorWasUnlocked(mDoorIdentifier))
		{
			// TODO: play unlocking sound effect
			// TODO: do effects
			AudioManager::Instance()->PlaySoundEffect(kDefaultDoorUnlockedSfx);
			SaveManager::GetInstance()->SetDoorWasUnlocked(mDoorIdentifier, true);
			mInteractableProperties.InteractCountdown = 0.4f;
			mCanTryOpen = true;
			mIsUnlockingCurrentTime = kUnlockingTime;

			/*
			ParticleEmitterManager::Instance()->CreateDirectedSpray(1,
				Position(),
				GetDepthLayer(),
				Vector2(0.0f, 0.0f),
				0.1f,
				Vector2(3200.0f, 1200.0f),
				"Media\\explosion_lines.png",
				1.0f,
				1.0f,
				0.1f,
				0.1f,
				256.0f,
				256.0f,
				0.0f,
				false,
				1.0f,
				1.0f,
				0.0f,
				true,
				12.0f,
				0.0f,
				0.0f,
				0.05f,
				0.1f,
				true);
				*/

			return;
		}
	}

	EnterDoor();
}

bool Door::CanInteract()
{
	if (mIsUnlockingCurrentTime > 0.0f)
	{
		return false;
	}

	Player * player = GameObjectManager::Instance()->GetPlayer();
	if (player == nullptr)
	{
		return false;
	}

	if (mDoorWarmUpTime < kWarmUpTime)
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

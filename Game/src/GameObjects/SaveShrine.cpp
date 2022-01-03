#include "precompiled.h"
#include "SaveShrine.h"
#include "TextObject.h"
#include "Game.h"
#include "AudioManager.h"
#include "ParticleEmitterManager.h"
#include "SaveManager.h"

static const float kTimeBetweenSaves = 5.0f;

SaveShrine::SaveShrine(float x, float y, DepthLayer depthLayer, float width, float height) :
	GameObject(x, y, depthLayer, width, height)
{
	mAlwaysUpdate = true;
}

SaveShrine::~SaveShrine(void)
{
}

void SaveShrine::Initialise()
{
	GameObject::Initialise();
}

void SaveShrine::Update(float delta)
{
	GameObject::Update(delta);

	if (Game::GetInstance()->GetIsLevelEditMode())
	{
		return;
	}

	Player * player = GameObjectManager::Instance()->GetPlayer();
	if (!player)
	{
		return;
	}

	if (Utilities::IsObjectInRectangle(player, m_position.X, m_position.Y, m_dimensions.X, m_dimensions.Y))
	{
		if (!mHasSaved && player->IsFullyCrouched())
		{
			// DisplaySaveText();

			AudioManager::Instance()->PlaySoundEffect("save_game.wav");

			AudioManager::Instance()->PlaySoundEffect("water/steam_hissing.wav");

			Camera2D::GetInstance()->DoMediumShake();

			DisplaySaveParticles();

			//Note: I'm no longer allowing saving here
			// The game now always saves permanent upgrades and is a roguelike similar to Returnal
			/*
			SaveManager::GetInstance()->SetLevelLastSavedAt(GameObjectManager::Instance()->GetCurrentLevelFile());

			GameObjectManager::Instance()->SaveGame();
			*/

			mHasSaved = true;

			mTimeUntilCanSaveAgain = kTimeBetweenSaves;
		}	
	}

	if (mHasSaved && !player->IsFullyCrouched())
	{
		mTimeUntilCanSaveAgain -= delta;

		if (mTimeUntilCanSaveAgain <= 0.0f)
		{
			mHasSaved = false;
		}
	}
}

void SaveShrine::DisplaySaveParticles()
{
	ParticleEmitterManager::Instance()->CreateDirectedSpray(1,
		m_position,
		GameObject::kGroundFront,
		Vector2(0.0f, 0.0f),
		0.1f,
		Vector2(3200.0f, 1200.0f),
		"Media\\blast_circle.png",
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
		8.0f,
		0.0f,
		0.0f,
		0.05f,
		0.1f,
		true);

	ParticleEmitterManager::Instance()->CreateDirectedSpray(50,
															Vector2(m_position.X, m_position.Y - 20.0f),
															GameObject::kFarForeground,
															Vector2(0.0f, 1.0f),
															0.4f,
															Vector2(3200.0f, 1200.0f),
															"Media\\levels\\hot_spring\\fog_1.png",
															2.8f,
															3.0f,
															2.0f,
															7.5f,
															128.0f,
															128.0f,
															0.6f,
															false,
															1.0f,
															1.0f,
															2.0f,
															true,
															2.5f,
															m_dimensions.X * 0.07f,
															3.0f,
															0.05f,
															0.9f,
															true);
}

void SaveShrine::XmlRead(TiXmlElement * element)
{
	GameObject::XmlRead(element);
}

void SaveShrine::XmlWrite(TiXmlElement * element)
{
	GameObject::XmlWrite(element);
}

void SaveShrine::DisplaySaveText()
{
	TextObject * saveTextObject = new TextObject(m_position.X, m_position.Y, GetDepthLayer(), 3.0f);

	saveTextObject->SetFont("Courier New");
	saveTextObject->SetFontColor(0.35f, 0.35f, 0.35f);
	saveTextObject->SetFontSize(40.0f);
	saveTextObject->SetStringKey("game_saved");
	saveTextObject->SetNoClip(true);
	saveTextObject->SetAlwaysShow(false);
	saveTextObject->SetTriggeredByPlayer(false);
	saveTextObject->SetHasShown(true);
	saveTextObject->SetTimeToShow(0.5f);

	saveTextObject->SetDimensionsXY(300.0f, 100.0f);

	GameObjectManager::Instance()->AddGameObject(saveTextObject);

	saveTextObject->AttachTo(GameObjectManager::Instance()->GetObjectByID(ID()), Vector2(0.0f, 200.0f), GameObject::kGroundFront,  false);
}

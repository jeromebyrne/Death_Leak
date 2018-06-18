#include "precompiled.h"
#include "SaveShrine.h"
#include "TextObject.h"

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

	Player * player = GameObjectManager::Instance()->GetPlayer();
	if (!player)
	{
		return;
	}

	if (Utilities::IsObjectInRectangle(player, m_position.X, m_position.Y, m_dimensions.X, m_dimensions.Y))
	{
		if (!mHasSaved && mHasPlayerExitedBounds)
		{
			DisplaySaveText();

			GameObjectManager::Instance()->SaveGame();

			mHasSaved = true;

			mTimeUntilCanSaveAgain = kTimeBetweenSaves;
		}
		mHasPlayerExitedBounds = false;
	
	}
	else
	{
		mHasPlayerExitedBounds = true;
	}

	if (mHasSaved)
	{
		mTimeUntilCanSaveAgain -= delta;

		if (mTimeUntilCanSaveAgain <= 0.0f)
		{
			mHasSaved = false;
		}
	}
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

	saveTextObject->SetFont("Jing Jing");
	saveTextObject->SetFontColor(0.1f, 0.1f, 0.1f);
	saveTextObject->SetFontSize(40.0f);
	saveTextObject->SetStringKey("game_saved");
	saveTextObject->SetNoClip(true);

	saveTextObject->SetDimensionsXY(300.0f, 100.0f);

	GameObjectManager::Instance()->AddGameObject(saveTextObject);

	saveTextObject->AttachTo(GameObjectManager::Instance()->GetObjectByID(ID()), Vector2(0.0f, 0.0f), GetDepthLayer(),  false);
}

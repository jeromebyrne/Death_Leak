#include "precompiled.h"
#include "LevelEntry.h"
#include "Game.h"
#include "DrawUtilities.h"

LevelEntry::LevelEntry(float x, float y, DepthLayer depthLayer, float width, float height):
	GameObject(x, y, depthLayer, width, height)
{
	mAlwaysUpdate = true;
}

LevelEntry::~LevelEntry(void)
{
}

void LevelEntry::Initialise()
{
	GameObject::Initialise();

	auto gom = GameObjectManager::Instance();
	Player * p = gom->GetPlayer();

	if (p == nullptr)
	{
		GAME_ASSERT(false);
		return;
	}

	if (mLevelCameFrom == gom->mLastLevel)
	{
		if (mDoorIdentifier != gom->mDoorIdCameFrom)
		{
			// we came from a door and this is not the entry point
			return;
		}

		// a match so let's set the players position to this object
		p->SetXY(m_position.X, m_position.Y);
		p->AccelerateX(mPlayerDirectionX);
		p->ResetJumpHeightVariables();
	}
}

void LevelEntry::XmlRead(TiXmlElement * element)
{
	GameObject::XmlRead(element);

	// level_to_load
	mLevelCameFrom = XmlUtilities::ReadAttributeAsString(element, "level_came_from", "value");
	mPlayerDirectionX = XmlUtilities::ReadAttributeAsFloat(element, "player_direction", "x");
	mDoorIdentifier = XmlUtilities::ReadAttributeAsString(element, "door_id", "value");
}

void LevelEntry::XmlWrite(TiXmlElement * element)
{
	GameObject::XmlWrite(element);

	TiXmlElement * levelFile = new TiXmlElement("level_came_from");
	levelFile->SetAttribute("value", mLevelCameFrom.c_str());
	element->LinkEndChild(levelFile);

	TiXmlElement * dirElem = new TiXmlElement("player_direction");
	dirElem->SetDoubleAttribute("x", mPlayerDirectionX);
	element->LinkEndChild(dirElem);

	TiXmlElement * doorid = new TiXmlElement("door_id");
	doorid->SetAttribute("value", mDoorIdentifier.c_str());
	element->LinkEndChild(doorid);
}

void LevelEntry::DebugDraw(ID3D10Device *  device)
{
	GameObject::DebugDraw(device);

	DrawUtilities::DrawTexture(Vector3(m_position.X, m_position.Y, 3), Vector2(GetLevelEditSelectionDimensions().X * mPlayerDirectionX, GetLevelEditSelectionDimensions().Y), "Media\\editor\\level_entry.png");
}

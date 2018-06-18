#include "precompiled.h"
#include "ForceBox.h"

ForceBox::ForceBox(float x, float y, DepthLayer depthLayer, float width, float height) :
	GameObject(x, y, depthLayer, width, height)
{
	mAlwaysUpdate = true;
}

ForceBox::~ForceBox(void)
{
}

void ForceBox::Initialise()
{
	GameObject::Initialise();
}

void ForceBox::Update(float delta)
{
	GameObject::Update(delta);

	Player * player = GameObjectManager::Instance()->GetPlayer();
	if (player)
	{
		if (Utilities::IsObjectInRectangle(player, m_position.X, m_position.Y, m_dimensions.X, m_dimensions.Y))
		{
			if (player->IsOnSolidSurface())
			{
				player->SetVelocityY(1.0f);
			}
			
			float yDiff = Top() - player->Bottom();
			float percent = yDiff / m_dimensions.Y;

			// player->AccelerateX(mForceDirection.X, mForce * delta);
			player->AccelerateY(mForceDirection.Y, (mForce * percent) * delta);
		}
	}
}

void ForceBox::XmlRead(TiXmlElement * element)
{
	GameObject::XmlRead(element);

	mForce = XmlUtilities::ReadAttributeAsFloat(element, "force", "value");
	mForceDirection.X = XmlUtilities::ReadAttributeAsFloat(element, "force_direction", "x");
	mForceDirection.Y = XmlUtilities::ReadAttributeAsFloat(element, "force_direction", "y");
}

void ForceBox::XmlWrite(TiXmlElement * element)
{
	GameObject::XmlWrite(element);

	TiXmlElement * force = new TiXmlElement("force");
	force->SetDoubleAttribute("value", mForce);
	element->LinkEndChild(force);

	TiXmlElement * forceDirection = new TiXmlElement("force_direction");
	forceDirection->SetDoubleAttribute("x", mForceDirection.X);
	forceDirection->SetDoubleAttribute("y", mForceDirection.Y);
	element->LinkEndChild(forceDirection);
}

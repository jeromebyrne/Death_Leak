#include "precompiled.h"
#include "CollisionManager.h"

CollisionManager* CollisionManager::m_instance = 0;
CollisionManager::CollisionManager():m_detectAreaDimensions(1580, 1020), m_dimensionDivisions(1,1)
{
}

CollisionManager::~CollisionManager(void)
{
}

void CollisionManager::Initialise(int collisionAreaWidth, int collisionAreaHeight, int numXDivisions, int numYDivisions)
{
	m_detectAreaDimensions = Vector2(collisionAreaWidth, collisionAreaHeight);
	m_dimensionDivisions = Vector2(numXDivisions, numYDivisions);
}

// do collision detection and then resolve collisions
void CollisionManager::DetectAndResolve(int camX, int camY)
{
	list<shared_ptr<GameObject> > & objList = GameObjectManager::Instance()->GetGameObjectList();

	for (auto & obj : objList)
	{
		if (!obj)
		{
			GAME_ASSERT(obj);
			continue;
		}

		if (!obj->IsSolidSprite())
		{
			continue;
		}

		SolidMovingSprite * solidSprite = static_cast<SolidMovingSprite*>(obj.get());
		GAME_ASSERT(dynamic_cast<SolidMovingSprite *>(obj.get()));

		if (!Utilities::IsSolidSpriteInRectangle(solidSprite, camX, camY, m_detectAreaDimensions.X, m_detectAreaDimensions.Y))
		{
			continue;
		}

		// set the object to not collide before we determine anything
		solidSprite->SetNotColliding();

		for (auto & otherObj : objList)
		{
			if (obj == otherObj)
			{
				continue;
			}

			if (!otherObj)
			{
				GAME_ASSERT(otherObj);
				continue;
			}

			if (!otherObj->IsSolidSprite())
			{
				continue;
			}

			SolidMovingSprite * otherSolidSprite = static_cast<SolidMovingSprite*>(otherObj.get());
			GAME_ASSERT(dynamic_cast<SolidMovingSprite *>(otherObj.get()));

			if (IsColliding(solidSprite, otherSolidSprite))
			{
				// TODO: check the OnCollision functions for safety 
				// best to pass the shared pointers rather than raw pointers
				solidSprite->OnCollision(otherSolidSprite);
				otherSolidSprite->OnCollision(solidSprite);
			}
		}
	}// end outer for	
}

bool CollisionManager::IsColliding(SolidMovingSprite* first, SolidMovingSprite* second)
{
	bool colliding = false;

	// first coordinates
	float firstLeft = first->CollisionLeft();
	float firstRight = first->CollisionRight();
	float firstTop = first->CollisionTop();
	float firstBottom = first->CollisionBottom();
	
	// second coordinates
	float secondLeft = second->CollisionLeft();
	float secondRight = second->CollisionRight();
	float secondTop = second->CollisionTop();
	float secondBottom = second->CollisionBottom();

	if(firstRight > secondLeft	&&
	   firstLeft < secondRight	&&
	   firstTop > secondBottom  &&
	   firstBottom < secondTop)
	{
		colliding = true;
	}

	return colliding;
}



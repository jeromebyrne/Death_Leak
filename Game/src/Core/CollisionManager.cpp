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

/*void CollisionManager::AddObject(SolidMovingSprite * object)
{
	m_collisionObjects.push_back(object);
}*/

// do collision detection and then resolve collisions
void CollisionManager::DetectAndResolve(int camX, int camY)
{
	// detect and store collidables
	Detect(camX, camY);

	// resolve our collsions
	Resolve();
}

void CollisionManager::Detect(int camX, int camY)
{
	// TODO: possible optimisation, keep objects in map and only remove if need to
	m_collisionMap.clear();

	// loop through our collision objects and determine if they are in the colision zone
	list<SolidMovingSprite*>::iterator current = m_collisionObjects.begin();

	for(; current != m_collisionObjects.end(); current++)
	{
		// check if it's in the collision zone
		if(Utilities::IsSolidSpriteInRectangle((*current), camX, camY, m_detectAreaDimensions.X, m_detectAreaDimensions.Y))
		{
			list<SolidMovingSprite*>::iterator other = m_collisionObjects.begin();
			for(; other != m_collisionObjects.end(); other++)
			{
				// dont check against itself
				if((*current) != (*other))
				{
					// check if there is a collision
					if(IsColliding((*current), (*other)))
					{
						// add to the current objects collision map
						m_collisionMap[(*current)].push_back((*other));
					}
					else
					{
						(*current)->SetNotColliding();
					}
				}
			}// end inner for
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

void CollisionManager::Resolve()
{
	map<SolidMovingSprite*, list<SolidMovingSprite*>>::iterator current = m_collisionMap.begin();

	for(; current != m_collisionMap.end(); current++)
	{
		list<SolidMovingSprite*>::iterator listCurrent = (*current).second.begin();

		for(; listCurrent != (*current).second.end(); listCurrent++)
		{
			// call objects on collision function
			(*current).first->OnCollision((*listCurrent));
		}
	}
}

/*void CollisionManager::RemoveObject(SolidMovingSprite * object)
{
	m_collisionObjects.remove(object);
}*/


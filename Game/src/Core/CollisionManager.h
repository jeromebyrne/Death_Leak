#ifndef COLLISIONMANAGER_H
#define COLLISIONMANAGER_H
#include "SolidMovingSprite.h"

class CollisionManager
{
public:

	inline static CollisionManager * Instance()
	{
		if (m_instance == 0)
		{
			m_instance = new CollisionManager();
		}
		return m_instance;
	}

	void Initialise(int collisionAreaWidth, int collisionAreaHeight, int numXDivisions, int numYDivisions);

	void DetectAndResolve(int camX, int camY);

	void SetCollisionArea(int width, int height) 
	{
		m_detectAreaDimensions.X = width;
		m_detectAreaDimensions.Y = height;
	}

private:

	CollisionManager();
	~CollisionManager(void);

	static CollisionManager * m_instance;
	Vector2 m_detectAreaDimensions; // we onlt do detection of objects within this area
	Vector2 m_dimensionDivisions; // how many collision areas do we have?

	bool IsColliding(SolidMovingSprite* first, SolidMovingSprite* second);
};

#endif

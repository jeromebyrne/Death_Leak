#include "precompiled.h"
#include "Camera2D.h"
#include "MovingSprite.h"

Camera2D * Camera2D::mInstance = 0;

const int kPanXtime = 0.55f;

Camera2D::Camera2D(int screenWidth, int screenHeight, float x, float y, float z): 
	m_position(x,y,z), 
	m_width(screenWidth),
	m_height(screenHeight),
	mZoomInPercent(1.0f)
{
	// Initialize the world matrix
    D3DXMatrixIdentity( &m_world );
	D3DXMatrixIdentity( &m_view );

	D3DXMatrixTranslation(&m_view, (int)-m_position.X, (int)-m_position.Y, m_position.Z);

	// create orthographic projection
	D3DXMatrixOrthoLH(&m_projection, screenWidth, screenHeight, 0.0f, (std::numeric_limits<float>::max)());

	mInstance = this;
}

Camera2D::~Camera2D(void)
{
}

bool Camera2D::IsObjectInView(GameObject * object)
{
	bool inView = false;

	float objectX = object->X() - object->GetCurrentParallaxOffsetX();
	float objectY = object->Y() - object->GetCurrentParallaxOffsetY();
	float screenLeft = (m_position.X - m_width/2);
	float screenRight = (m_position.X + m_width/2);
	float screenBottom = (m_position.Y - m_height/2);
	float screenTop = (m_position.Y + m_height/2);

	// first check if the objects origin is in view
	if(objectX > screenLeft && objectX < screenRight &&
	   objectY > screenBottom && objectY < screenTop)
	{
		inView = true;
	}
	// ok so the origin might not be in view but part of the object may be in view
	if(!inView)
	{
		float objectRight = object->Right() - object->GetCurrentParallaxOffsetX();
		float objectLeft = object->Left() - object->GetCurrentParallaxOffsetX();
		float objectTop = object->Top() - object->GetCurrentParallaxOffsetY();
		float objectBottom = object->Bottom() - object->GetCurrentParallaxOffsetY();

		// check if we are partially in view on the x plane
		if(objectRight > screenLeft && objectLeft < screenRight)
		{
			// now check if we are partially in view on the y plane
			if(objectTop > screenBottom && objectBottom < screenTop)
			{
				// we are viewable
				inView = true;
			}
		}
	}
	return inView;
}

void Camera2D::SetBounds(float left, float right, float top, float bottom)
{
	mBoundsTopLeft.X = left;
	mBoundsTopLeft.Y = top;
	mBoundsBottomRight.X = right;
	mBoundsBottomRight.Y = bottom;
}

void Camera2D::FollowObjectWithOffset(GameObject * object)
{
	if (UpdateBoundsX(object->X() + mTargetOffset.X))
	{
		m_position.X = object->X() + mTargetOffset.X;
	}

	if (UpdateBoundsY(object->Y() + mTargetOffset.Y))
	{
		m_position.Y = object->Y() + mTargetOffset.Y;
	}
}

bool Camera2D::UpdateBoundsX(float newPositionX)
{
	if (newPositionX < mBoundsBottomRight.X &&
		newPositionX > mBoundsTopLeft.X)
	{
		return true;
	}

	return false;
}

bool Camera2D::UpdateBoundsY(float newPositionY)
{
	if (newPositionY > mBoundsBottomRight.Y &&
		newPositionY < mBoundsTopLeft.Y)
	{
		return true;
	}

	return false;
}

void Camera2D::Update()
{
	D3DXMatrixTranslation(&m_view, (int)-m_position.X, (int)-m_position.Y, m_position.Z);
#if _DEBUG
	int movespeed = 40;
	// test
	if(GetAsyncKeyState('D') < 0)
	{
		m_position.X+= movespeed;
	}
	else if(GetAsyncKeyState('A') < 0)
	{
		m_position.X-=movespeed;
	}

	if(GetAsyncKeyState('W') < 0)
	{ 
		m_position.Y+= movespeed;
	}
	else if(GetAsyncKeyState('S') < 0 && GetAsyncKeyState(VK_CONTROL) >= 0)
	{
		m_position.Y-=movespeed;
	}
#endif
}

void Camera2D::FollowMovingObjectWithLag(MovingSprite * object)
{
	// get the x and y distance between the camera and the object
	float distanceX = 0.0f;
	float distanceY = 0.0f;

	float xLag = mTargetLag.X * mZoomInPercent;

	if (xLag < 1.0f)
	{
		xLag = 1.0f;
	}

	if (mTargetLag.Y < 1.0f)
	{
		mTargetLag.Y = 1.0f;
	}

	if(object->DirectionX() > 0)
	{
		distanceX = m_position.X - (object->X() + mTargetOffset.X * mZoomInPercent);
	}
	else
	{
		distanceX = m_position.X - (object->X() - mTargetOffset.X * mZoomInPercent);
	}

	distanceY = m_position.Y - (object->Y() + mTargetOffset.Y * mZoomInPercent);

	if (UpdateBoundsX(m_position.X - distanceX / xLag))
	{
		m_position.X -= distanceX / xLag;
	}

	if (UpdateBoundsY(m_position.Y - distanceY / mTargetLag.Y))
	{
		m_position.Y -= distanceY / mTargetLag.Y;
	}
}

bool Camera2D::IsCameraOriginInsideObject(GameObject * object)
{
	float right = object->Right();
	float left = object->Left();
	float top = object->Top();
	float bottom = object->Bottom();

	if (m_position.X < object->Right() &&
		m_position.X > object->Left() &&
		m_position.Y < object->Top() &&
		m_position.Y > object->Bottom())
	{
		return true;
	}

	return false;
}

bool Camera2D::IsCameraOriginInsideRect(Vector3 pos, Vector2 dimensions)
{
	if (m_position.X < pos.X + dimensions.X * 0.5f &&
		m_position.X > pos.X - dimensions.X * 0.5f &&
		m_position.Y < pos.Y + dimensions.Y * 0.5f && 
		m_position.Y > pos.Y - dimensions.Y * 0.5f)
	{
		return true;
	}

	return false;
}

void Camera2D::SetZoomInLevel(float value)
{
	if (value > 1.0f)
	{
		value = 1.0f;
	}

	if (value < 0.2f)
	{
		value = 0.2f;
	}

	mZoomInPercent = value;

	D3DXMatrixOrthoLH(&m_projection, m_width * value, m_height * value, 0.0f, (std::numeric_limits<float>::max)());
}


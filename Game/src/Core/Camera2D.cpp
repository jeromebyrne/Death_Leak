#include "precompiled.h"
#include "Camera2D.h"
#include "MovingSprite.h"

Camera2D * Camera2D::mInstance = 0;

const int kPanXtime = 0.55f;

Camera2D::Camera2D(int screenWidth, int screenHeight, float x, float y, float z): 
m_position(x,y,z), 
m_width(screenWidth),
m_height(screenHeight),
mPanningX(false),
mPanStartX(0),
mPanTargetX(0),
mPanStartTime(0.0f)
{
	// Initialize the world matrix
    D3DXMatrixIdentity( &m_world );
	D3DXMatrixIdentity( &m_view );

	D3DXMatrixTranslation(&m_view, (int)-m_position.X, (int)-m_position.Y, m_position.Z);

	// create orthographic projection
	D3DXMatrixOrthoLH(&m_projection, screenWidth, screenHeight, 0.1f, 100000.0f);

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

void Camera2D::FollowObjectWithOffset(GameObject * object, float xOffset, float yOffset)
{
	m_position.X = object->X() + xOffset;
	m_position.Y = object->Y() + yOffset;
}

void Camera2D::Update()
{
	D3DXMatrixTranslation(&m_view, (int)-m_position.X, (int)-m_position.Y, m_position.Z);

	if (mPanningX)
	{
		float time_spent = Timing::Instance()->GetTotalTimeSeconds() - mPanStartTime;

		if (time_spent)
		{
			float total_percent_time = (float)(min(time_spent, kPanXtime)) / (float)(kPanXtime);

			// moving to the right
			float progress_diff= mPanTargetX - mPanStartX;

			m_position.X = mPanStartX + (progress_diff * total_percent_time);

			if (total_percent_time >= 1)
			{
				m_position.X = mPanTargetX;
				mPanningX = false;
			}
		}
	}

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

void Camera2D::FollowMovingObjectWithLag(MovingSprite * object, float xLag, float yLag, float xOffset, float yOffset)
{
	// get the x and y distance between the camera and the object
	float distanceX;
	float distanceY;

	if(object->DirectionX() > 0)
	{
		distanceX = m_position.X - (object->X() + xOffset);
	}
	else
	{
		distanceX = m_position.X - (object->X() - xOffset);
	}

	distanceY = m_position.Y - (object->Y() + yOffset);
	m_position.X -= distanceX / xLag;
	m_position.Y -= distanceY / yLag;
}

void Camera2D::FollowMovingObjectPanMode(MovingSprite * object, float xOffset, float yOffset, float xPanWindowMargin )
{
	// follow the object on the Y at all times
	m_position.Y = object->Y() + yOffset;

	// TODO: scale xPanWindowMargin somewhere
	int pan_window_width = Graphics::GetInstance()->BackBufferWidth() - (xPanWindowMargin * 2);

	// get the x difference between the camera origin and the moving object
	int diff_x = abs(m_position.X - object->X());

	if (diff_x > pan_window_width * 0.5)
	{
		if (!mPanningX)
		{
			mPanningX = true;
			mPanStartX = m_position.X;
			mPanStartTime = Timing::Instance()->GetTotalTimeSeconds();
			
			if (object->DirectionX() > 0)
			{
				mPanTargetX = object->X() + xOffset;
			}
			else
			{
				mPanTargetX = object->X() - xOffset;
			}
		}
	}
}

bool Camera2D::IsCameraOriginInsideObject(GameObject * object)
{
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


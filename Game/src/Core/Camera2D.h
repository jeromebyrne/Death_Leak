#ifndef CAMERA2D_H
#define CAMERA2D_H

class MovingSprite;

class Camera2D
{

public:

	static Camera2D * GetInstance() { return mInstance; }

	Camera2D(int screenWidth, int screenHeight, float x = 0, float y = 0, float z = 0);
	~Camera2D(void);

	void SetBounds(float left, float right, float top, float bottom);

	bool IsObjectInView(GameObject * object);
	inline D3DXMATRIX World()
	{
		return m_world;
	}
	inline D3DXMATRIX View()
	{
		return m_view;
	}
	inline D3DXMATRIX Projection()
	{
		return m_projection;
	}

	inline float X()
	{
		return m_position.X;
	}
	inline float Y()
	{
		return m_position.Y;
	}

	void Update();

	inline void FollowObjectsOrigin(GameObject * object)
	{
		m_position.X = object->X();
		m_position.Y = object->Y();
	}

	void FollowObjectWithOffset(GameObject * object, float xOffset, float yOffset);

	Vector3 Position() const { return m_position; }

	void FollowMovingObjectWithLag(MovingSprite * object, float xLag, float yLag, float xOffset, float yOffset);

	void FollowMovingObjectPanMode(MovingSprite * object, float xOffset, float yOffset, float xPanWindowMargin);

	inline float ViewWidth()
	{
		return m_width;
	}
	inline float ViewHeight()
	{
		return m_height;
	}

	inline float Left() const { return m_position.X - m_width * 0.5; }
	inline float Right() const { return m_position.X + m_width * 0.5; }
	inline float Top() const { return m_position.Y + m_height * 0.5; }

	bool IsCameraOriginInsideObject(GameObject * object);

	bool IsCameraOriginInsideRect(Vector3 pos, Vector2 dimensions);

	void SetZoomInLevel(float value);

private:

	bool UpdateBoundsX(float newPositionX);

	bool UpdateBoundsY(float newPositionY);

	// wvp matrices
	D3DXMATRIX                  m_world;
    D3DXMATRIX                  m_view;
    D3DXMATRIX                  m_projection;

	Vector3 m_position;
	int m_width;
	int m_height;

	static Camera2D * mInstance;

	bool mPanningX; // are we panning to a point
	float mPanTargetX;
	float mPanStartX;
	float mPanStartTime;

	Vector2 mBoundsTopLeft;
	Vector2 mBoundsBottomRight;

	float mZoomInPercent;
};

#endif

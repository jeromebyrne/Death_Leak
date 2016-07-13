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
		if (mFollowX)
		{
			m_position.X = object->X();
		}

		if (mFollowY)
		{
			m_position.Y = object->Y();
		}
	}

	void SetTargetObject(MovingSprite * target) { mTargetObject = target; }

	void FollowObjectWithOffset(GameObject * object);

	Vector3 Position() const { return m_position; }

	void SetPositionY(float y) { m_position.Y = y; }

	void SetPositionX(float x) { m_position.X = x; }

	void FollowTargetObjectWithLag(bool forceUpdate = false, float overrideLagX = 0.0f, float overrideLagY = 0.0f);

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
	inline float Bottom() const { return m_position.Y - m_height * 0.5; }

	bool IsCameraOriginInsideObject(GameObject * object);

	bool IsCameraOriginInsideRect(Vector3 pos, Vector2 dimensions);

	bool IsWorldPosInView(Vector3 & pos, float parallaxXOffset, float parallaxYOffset);

	void SetZoomInLevel(float value);

	void SetTargetOffset(Vector2 offset) { mTargetOffset = offset; };

	void SetTargetLag(Vector2 lag) { mTargetLag = lag; };

	void DoSmallShake();

	void DoMediumShake();

	void DoBigShake();

	void SetOverrideDirection(bool shouldOverride, Vector2 direction) { mIsOverrideDirection = shouldOverride; mOverrideDirection = direction; }

	float GetLeftLevelBounds() const { return mBoundsTopLeft.X; }

	float GetRightLevelBounds() const { return mBoundsBottomRight.X; }

	void SetShouldFollowX(bool value) { mFollowX = value; }

	void SetShouldFollowY(bool value) { mFollowY = value; }

private:

	bool UpdateBoundsX(GameObject * target);

	bool UpdateBoundsY(GameObject * target);

	void Camera2D::DoShake(float intensity, float shakeDuration);

	// wvp matrices
	D3DXMATRIX                  m_world;
    D3DXMATRIX                  m_view;
    D3DXMATRIX                  m_projection;

	Vector3 m_position;
	int m_width;
	int m_height;

	static Camera2D * mInstance;

	Vector2 mBoundsTopLeft;
	Vector2 mBoundsBottomRight;

	float mZoomInPercent;

	Vector2 mTargetOffset;
	Vector2 mTargetLag;

	bool mCurrentlyShaking;
	float mCurrentShakeIntensity;
	float mShakeStartTime;
	float mCurrentShakeDuration;

	MovingSprite * mTargetObject;

	bool mIsOverrideDirection;
	Vector2 mOverrideDirection;

	bool mFollowX;
	bool mFollowY;
};

#endif

#ifndef MOVINGSPRITE_H
#define MOVINGSPRITE_H

#include "sprite.h"

// doesn't inherit from moving Object or contain a movingObject instance
// we just copy all of the properties of MovingObject (not as messy as its just one object to update etc...)
class MovingSprite : public Sprite
{
public:

	MovingSprite(float x = 0.0f, float y = 0.0f, DepthLayer depthLayer = kPlayer, float width = 10.0f, float height = 10.0f,
		float groundFriction = 1.0f, float airResistance = 1.0f);
	virtual ~MovingSprite(void);
	virtual void Update(float delta) override;
	virtual void Initialise() override;
	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;
	virtual void Scale(float xScale, float yScale, bool scalePosition = true) override;

	float DirectionX() const
	{
		return m_direction.X;
	}
	float DirectionY() const
	{
		return m_direction.Y;
	}
	Vector2 GetVelocity() const
	{
		return m_velocity;
	}
	float VelocityX() const
	{
		return m_velocity.X;
	}
	float VelocityY() const
	{
		return m_velocity.Y;
	}
	void SetVelocityX(float value) 
	{
		m_velocity.X = value;
	}
	inline void SetVelocityY(float value)
	{
		m_velocity.Y = value;
	}

	float GetAccelX() const
	{
		return m_acceleration.X;
	}
	float GetAccelY() const
	{
		return m_acceleration.Y;
	}

	inline void SetResistanceXY(float x, float y)
	{
		m_resistance = Vector2(x, y);
	}
	inline void SetDirectionXY(float x, float y)
	{
		m_direction = Vector2(x, y);
	}
	inline void SetDirectionX(float dirX) { m_direction.X = dirX; }
	inline void SetVelocity(Vector2 value)
	{
		m_velocity = value;
	}
	inline void SetVelocityXY(float x, float y)
	{
		m_velocity = Vector2(x, y);
	}
	inline Vector2 GetMaxVelocity()
	{
		return m_maxVelocity;
	}
	inline void SetMaxVelocityX(float x)
	{
		m_maxVelocity.X = x;
	}
	inline void SetMaxVelocityXY(float x, float y)
	{
		m_maxVelocity = Vector2(x, y);
	}
	inline void SetAccelerationXY(float x, float y)
	{
		m_acceleration = Vector2(x, y);
	}

	float GetYResistance() const { return m_resistance.Y; }

	virtual void AccelerateX(float directionX, float rate);
	virtual void AccelerateY(float directionY, float rate);
	virtual void StopXAccelerating();
	virtual void StopYAccelerating();

	const MovingSprite * GetObjectMovingWith() const { return mObjectMovingWith; }

	void SetObjectMovingWith(MovingSprite * movingWith) { mObjectMovingWith = movingWith; }

	void SetIsInWater(bool value);
	bool GetIsInWater() const { return mIsInWater; }

	bool WasInWaterLastFrame() const { return mWasInWaterLastFrame; }

	void SetCurrentYResistance(float value) { mCurrentYResistance = value; }

	void SetCurrentXResistance(float value) { mCurrentXResistance = value; }

	bool IsHittingSolidLineEdge() const { return mHittingSolidLineEdge; }

	void SetMaxVelocityLimitEnabled(bool value) { mMaxVelocityXLimitEnabled = value; }

	void SetGravityApplyAmount(float value) { mGravityApplyAmount = value; }

	virtual bool IsOnSolidSurface() const { return false; }

protected:

	virtual void DoWaterAccelerationBubbles();

	Vector2 m_velocity;
	Vector2 m_maxVelocity;
	Vector2 m_direction;
	Vector2 m_resistance;
	Vector2 m_acceleration;
	bool m_applyGravity;
	bool mMaxVelocityXLimitEnabled;
	float mGravityApplyAmount = 1.0f;

	// are we moving within the space of anothe rmoving object
	MovingSprite * mObjectMovingWith;

	float mCurrentYResistance; // this can change dynamically
	float mCurrentXResistance; // this can change dynamically
	bool mIsInWater;
	bool mWasInWaterLastFrame;
	float mTimeUntilCanSpawnWaterBubbles;
	bool mHittingSolidLineEdge;
};

#endif

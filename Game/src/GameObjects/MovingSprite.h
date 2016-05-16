#ifndef MOVINGSPRITE_H
#define MOVINGSPRITE_H

#include "sprite.h"

// doesn't inherit from moving Object or contain a movingObject instance
// we just copy all of the properties of MovingObject (not as messy as its just one object to update etc...)
class MovingSprite : public Sprite
{
public:

	MovingSprite(float x = 0, float y = 0, float z = 0, float width = 10, float height = 10, float breadth = 0,
		float groundFriction = 1, float airResistance = 1);
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
	Vector3 GetVelocity() const
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

	inline void SetResistanceXYZ(float x, float y, float z)
	{
		m_resistance = Vector3(x, y, z);
	}
	inline void SetDirectionXYZ(float x, float y, float z)
	{
		m_direction = Vector3(x, y, z);
	}
	inline void SetDirectionX(float dirX) { m_direction.X = dirX; }
	inline void SetVelocity(Vector3 value)
	{
		m_velocity = value;
	}
	inline void SetVelocityXYZ(float x, float y, float z)
	{
		m_velocity = Vector3(x, y, z);
	}
	inline Vector3 GetMaxVelocity()
	{
		return m_maxVelocity;
	}
	inline void SetMaxVelocityX(float x)
	{
		m_maxVelocity.X = x;
	}
	inline void SetMaxVelocityXYZ(float x, float y, float z)
	{
		m_maxVelocity = Vector3(x, y, z);
	}
	inline void SetAccelerationXYZ(float x, float y, float z)
	{
		m_acceleration = Vector3(x, y, z);
	}

	float GetYResistance() const { return m_resistance.Y; }

	bool IsOnGround() const { return m_isOnGround; }

	virtual void AccelerateX(float directionX, float rate);
	virtual void AccelerateY(float directionY, float rate);
	virtual void StopXAccelerating();
	virtual void StopYAccelerating();

	const MovingSprite * GetObjectMovingWith() const { return mObjectMovingWith; }

	void SetIsInWater(bool value, bool isDeepWater);
	bool GetIsInWater() const { return mIsInWater; }

	bool WasInWaterLastFrame() const { return mWasInWaterLastFrame; }

	bool GetWaterIsDeep() const { return mIsDeepWater; }

	void SetCurrentYResistance(float value) { mCurrentYResistance = value; }

protected:

	virtual void DoWaterAccelerationBubbles();

	Vector3 m_velocity;
	Vector3 m_maxVelocity;
	Vector3 m_direction;
	Vector3 m_resistance;
	Vector3 m_acceleration;
	bool m_applyGravity;

	// are we moving within the space of anothe rmoving object
	MovingSprite * mObjectMovingWith;

	float mCurrentYResistance; // this can change dynamically

	bool m_isOnGround;

	bool mIsInWater;
 
	bool mWasInWaterLastFrame;

	bool mIsDeepWater;

	float mTimeUntilCanSpawnWaterBubbles;
};

#endif

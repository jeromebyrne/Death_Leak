#ifndef PATHINGPLATFORM_H
#define PATHINGPLATFORM_H

#include "platform.h"

class PathingPlatform : public Platform
{
public:

	enum PlatformPathingType
	{
		kAlwaysPathing,
		kPathWhenTriggered
	};

	enum PlatformPathState
	{
		kNotPathing,
		kLoopingForward,
		kLoopingBackward,
		kReturningToStart
	};

	PathingPlatform(float x = 0.0f, float y = 0.0f, DepthLayer depthLayer = kPlayer, float width = 10.0f, float height = 10.0f,float groundFriction = 1.0f, float airResistance = 1.0f);
	virtual ~PathingPlatform(void);

	virtual bool OnCollision(SolidMovingSprite * object) override;

	virtual void Update(float delta) override;

	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

	virtual void Scale(float x, float y, bool scalePosition = true) override;

	virtual void Initialise() override;

protected:

	bool IsPlayerColliding();

	std::vector<Vector2> mPathPoints;

	int mCurrentPathIndex; // the last point we reached

	PlatformPathingType mPathingType;

	PlatformPathState mCurrentPathState;

	void PathForward();

	void PathBackward();

	void ReturnToStart();

	float mPlatformSpeedX = 1.0f;
	float mPlatformSpeedY = 1.0f;

	Vector2 mClosestPointToNextTarget;

	bool mPathForward; // move forward or backward

	Vector2 mOriginalPosition;

	bool mCollidingWithPlayer = false;
};

#endif

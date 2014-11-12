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

	PathingPlatform(float x = 0, float y = 0, float z = 0, float width = 10, float height = 10, float breadth = 0,float groundFriction = 1, float airResistance = 1);
	virtual ~PathingPlatform(void);

	virtual void OnCollision(SolidMovingSprite * object) override;

	virtual void Update(float delta) override;

	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

	virtual void Scale(float x, float y, bool scalePosition = true) override;

	virtual void Initialise() override;

protected:

	std::vector<Vector3> mPathPoints;

	int mCurrentPathIndex; // the last point we reached

	PlatformPathingType mPathingType;

	PlatformPathState mCurrentPathState;

	void PathForward();

	void PathBackward();

	void ReturnToStart();

	float mPlatformSpeed;

	Vector3 mClosestPointToNextTarget;

	bool mPathForward; // move forward or backward

	Vector3 mOriginalPosition;

};

#endif

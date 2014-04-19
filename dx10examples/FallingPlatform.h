#ifndef FALLINGPLATFORM_H
#define FALLINGPLATFORM_H

#include "platform.h"

class FallingPlatform : public Platform
{
public:
	
	enum FallingPlatformState
	{
		kStatic,
		kTriggered,
		kFalling,
		kReturning
	};

	FallingPlatform(float x = 0, float y = 0, float z = 0, float width = 10, float height = 10, float breadth = 0,float groundFriction = 1, float airResistance = 1);
	virtual ~FallingPlatform(void);

	virtual void OnCollision(SolidMovingSprite * object);

	virtual void XmlRead(TiXmlElement * element);
	virtual void XmlWrite(TiXmlElement * element);

	virtual void Update(float delta) override;

	virtual void Scale(float x, float y, bool ScalePosition = true);

private:

	void DoTriggerLogic();

	void DoReturnLogic();

	void DoFallingLogic();

	Vector3 mInitialPosition;

	FallingPlatformState mCurrentState;

	float mTriggerStartTime;

	float mMaxTriggerTime;

	float mFallStartTime;

	float mReturnStartTime;

	float mTimeUntilReturn;

	Vector3 mInitialReturnPosition;

};

#endif

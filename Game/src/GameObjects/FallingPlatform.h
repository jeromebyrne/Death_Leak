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

	FallingPlatform(float x = 0.0f, float y = 0.0f, DepthLayer depthLayer = kPlayer, float width = 10.0f, float height = 10.0f, float groundFriction = 1.0f, float airResistance = 1.0f);
	virtual ~FallingPlatform(void);

	virtual bool OnCollision(SolidMovingSprite * object) override;

	virtual void XmlRead(TiXmlElement * element) override;
	virtual void XmlWrite(TiXmlElement * element) override;

	virtual void Update(float delta) override;

	virtual void Scale(float x, float y, bool ScalePosition = true) override;

	virtual void OnDamage(GameObject * damageDealer, float damageAmount, Vector2 pointOfContact, bool shouldExplode = true) override;

private:

	virtual void Initialise() override;

	void DoTriggerLogic();

	void DoReturnLogic();

	void DoFallingLogic();

	Vector2 mInitialPosition;

	FallingPlatformState mCurrentState;

	float mTriggerStartTime;

	float mMaxTriggerTime;

	float mFallStartTime;

	float mReturnStartTime;

	float mTimeUntilReturn;

	Vector2 mInitialReturnPosition;

};

#endif

#ifndef AISTATEFLYING_H
#define AISTATEFLYING_H

#include "AIState.h"

class AIStateFlying : public AIState
{
public:

	AIStateFlying(NPC * npc);
	virtual ~AIStateFlying(void);

	virtual void OnTransition() override;
	virtual void Update(float delta) override;

private:

	void MoveTowardsDesiredHeight(float delta);

	void MoveTowardsDesiredXPosition(float delta);

	float mCurrentHeightTarget = 400.0f;
	
	float mTimeUntilCanChangeHeight = 0.0f;

	int mRandXTargetOffset = 0;
};

#endif
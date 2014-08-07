#ifndef AISTATEBUTTERFLYWANDER_H
#define AISTATEBUTTERFLYWANDER_H

#include "AIState.h"

class AIStateButterflyWander : public AIState
{
public:

	AIStateButterflyWander(NPC * npc);
	virtual ~AIStateButterflyWander(void);

	virtual void OnTransition() override;
	virtual void Update(float delta) override;

private:

	void DoWander(float delta);

	float mCurrentYVelocityDelay;
	float mLastYVelocityBurst;
	float mCurrentDirectionChangeDelay;
	float mLastDirectionChangeDelay;
};

#endif

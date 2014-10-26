#ifndef AISTATERANGEATTACK_H
#define AISTATERANGEATTACK_H

#include "AIState.h"

class AIStateRangeAttack : public AIState
{
public:

	AIStateRangeAttack(NPC * npc);
	virtual ~AIStateRangeAttack(void);

	virtual void OnTransition() override;
	virtual void Update(float delta) override;

	void SetDesiredRange(float value) { mDesiredRange = value; }

private:

	float mDesiredRange;
	float mFollowRange;
	float mRandOffset;

	float mLastTimeRanAway;
};

#endif
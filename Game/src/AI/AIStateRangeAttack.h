#ifndef AISTATERANGEATTACK_H
#define AISTATERANGEATTACK_H

#include "AIState.h"

class AIStateRangeAttack : public AIState
{
public:

	AIStateRangeAttack(NPC * npc);
	virtual ~AIStateRangeAttack(void);

	virtual void OnTransition();
	virtual void Update();

	void SetDesiredRange(float value) { mDesiredRange = value; }

private:

	float mDesiredRange;
	float mFollowRange;
	float mRandOffset;
};

#endif

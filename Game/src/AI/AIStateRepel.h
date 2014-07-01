#ifndef AISTATEREPEL_H
#define AISTATEREPEL_H

#include "AIState.h"

class AIStateRepel : public AIState
{
public:
	AIStateRepel(NPC * npc);
	virtual ~AIStateRepel(void);

	virtual void OnTransition();
	virtual void Update();

	void SetStopDistance(float value) { mStopDistance = value; }
	void SetFollowRadius(float value) { mFollowRadius = value; }

private:

	float mStopDistance;
	float mFollowRadius;
};

#endif

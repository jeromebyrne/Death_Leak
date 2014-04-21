#ifndef AISTATEFOLLOW_H
#define AISTATEFOLLOW_H

#include "AIState.h"

class AIStateFollow : public AIState
{
public:
	AIStateFollow(NPC * npc);
	virtual ~AIStateFollow(void);

	virtual void OnTransition();
	virtual void Update();

	void SetStopDistance(float value) { mStopDistance = value; }
	void SetFollowRadius(float value) { mFollowRadius = value; }

private:

	float mStopDistance;
	float mFollowRadius;
};

#endif

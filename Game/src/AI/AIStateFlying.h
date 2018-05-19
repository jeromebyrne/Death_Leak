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

	float mHeightAbovePlayer = 400.0f;

};

#endif
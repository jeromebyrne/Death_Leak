#ifndef AIStateGroundAnimalWander_H
#define AIStateGroundAnimalWander_H

#include "AIState.h"

class AIStateGroundAnimalWander : public AIState
{
public:

	AIStateGroundAnimalWander(NPC * npc);
	virtual ~AIStateGroundAnimalWander(void);

	virtual void OnTransition() override;
	virtual void Update(float delta) override;

private:

	void UpdateMoving(float delta);

	void UpdateResting(float delta);

	void UpdateRunningFromPlayer(float delta);

	void PickRandomXDirection();

	bool CanAccelerateX(float direction);

	bool IsPlayerClose() const;

	bool IsPlayerCloseEnoughToJumpFromEdge() const;

	enum GroundAnimalWanderInternalState
	{
		kResting,
		kMoving,
		kRunningFromPlayer
	};

	float mCurrentRestTime;
	float mCurrentMovingTime;
	float mCurrentTimeRunningFromPlayer;

	GroundAnimalWanderInternalState mCurrentInternalState;

	float mOriginalMaxVelocityX;
};

#endif

#ifndef AISTATE_H
#define AISTATE_H

class NPC;

class AIState
{
public:

	enum AIStateType
	{
		kAbstract = 0, // this is given to the abstract base class 
		kStill,
		kFriendlyFollowing,
		kRepel,
		kRangeAttack,
		kButterflyWander,
		kGroundAnimalWander,
		kFlying,
		kNone
	};

	AIState(NPC * npc);
	~AIState(void);

	virtual void OnTransition() = 0;
	virtual void Update(float delta) = 0;

protected:

	AIStateType mStateType;

	NPC * m_npc;
};

#endif

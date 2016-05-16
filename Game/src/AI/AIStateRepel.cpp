#include "precompiled.h"
#include "AIStateRepel.h"
#include "NPC.h"

AIStateRepel::AIStateRepel(NPC * npc) :
	AIState(npc),
	mStopDistance(700),
	mFollowRadius(999999)
{
	mStateType = kRepel;
}

AIStateRepel::~AIStateRepel(void)
{
}

void AIStateRepel::OnTransition()
{
	// do stuff
}

void AIStateRepel::Update(float delta)
{
	if (m_npc->m_player /*&& m_npc->IsOnSolidSurface()*/)
	{
		// get the distance to the player
		Vector3 distanceSquaredVector =  m_npc->m_player->Position() - m_npc->Position();
		float distanceSquared = distanceSquaredVector.LengthSquared();
		if (/*distanceSquared > mStopDistance * mStopDistance &&*/
			distanceSquared < mFollowRadius * mFollowRadius)
		{
			distanceSquaredVector.Normalise();
			m_npc->AccelerateX(-distanceSquaredVector.X);
		}
		else
		{
			m_npc->StopXAccelerating();
		}
	}
}

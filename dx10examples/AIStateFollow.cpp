#include "precompiled.h"
#include "AIStateFollow.h"
#include "NPC.h"

AIStateFollow::AIStateFollow(NPC * npc): 
	AIState(npc),
	mStopDistance(700),
	mFollowRadius(999999)
{
	mStateType = kFriendlyFollowing;
}

AIStateFollow::~AIStateFollow(void)
{
}

void AIStateFollow::OnTransition()
{
	// do stuff
}

void AIStateFollow::Update()
{
	// just prototyping the basic following here
	if (m_npc->m_player)
	{
		// get the distance to the player
		Vector3 distanceSquaredVector =  m_npc->m_player->Position() - m_npc->Position();
		float distanceSquared = distanceSquaredVector.LengthSquared();
		if (distanceSquared > mStopDistance * mStopDistance &&
			distanceSquared < mFollowRadius * mFollowRadius)
		{
			distanceSquaredVector.Normalise();
			m_npc->AccelerateX(distanceSquaredVector.X);

			if (!m_npc->m_player->IsOnSolidSurface() && m_npc->IsOnSolidSurface())
			{
				// jump - this is just test logic for the moment
				m_npc->Jump(100);
			}
		}
		else
		{
			m_npc->StopXAccelerating();
		}
	}
}

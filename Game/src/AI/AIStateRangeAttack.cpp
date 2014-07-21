#include "precompiled.h"
#include "AIStateRangeAttack.h"
#include "NPC.h"

AIStateRangeAttack::AIStateRangeAttack(NPC * npc) :
	AIState(npc),
	mDesiredRange(650),
	mFollowRange(800),
	mRandOffset(0.0f)
{
	mStateType = kRangeAttack;
}

AIStateRangeAttack::~AIStateRangeAttack(void)
{
}

void AIStateRangeAttack::OnTransition()
{
	float randMaxXVelocity = rand() % 3000;
	randMaxXVelocity *= 0.001;
	randMaxXVelocity += 20;

	m_npc->SetMaxVelocityXYZ(randMaxXVelocity, 99999, 0);

	mRandOffset = rand() % 150;
}

void AIStateRangeAttack::Update(float delta)
{
	if (m_npc->m_player)
	{
		// get the distance to the player
		Vector3 distanceSquaredVector =  m_npc->m_player->Position() - m_npc->Position();
		float distanceSquared = distanceSquaredVector.LengthSquared();
		if (distanceSquared < (mDesiredRange + mRandOffset) * (mDesiredRange + mRandOffset))
		{
			distanceSquaredVector.Normalise();
			m_npc->AccelerateX(-distanceSquaredVector.X);
		}
		else if (distanceSquared > (mFollowRange + mRandOffset) * (mFollowRange + mRandOffset))
		{
			distanceSquaredVector.Normalise();
			m_npc->AccelerateX(distanceSquaredVector.X);
		}
		else
		{
			m_npc->StopXAccelerating();
			GAME_ASSERT(GameObjectManager::Instance()->GetPlayer());
			m_npc->FireProjectileAtObject(GameObjectManager::Instance()->GetPlayer());

			if (distanceSquaredVector.X < 0)
			{
				// flip the sprite horizontally
				m_npc->FlipHorizontal();
			}
			else if (distanceSquaredVector.X > 0)
			{
				// unflip
				m_npc->UnFlipHorizontal();
			}
		}
	}
}

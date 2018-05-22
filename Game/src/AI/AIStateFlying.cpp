#include "precompiled.h"
#include "AIStateFlying.h"
#include "NPC.h"

const float kHeightFollowRange = 10.0f;
const float kXPosFollowRangeMin = 150.0f;
const float kXPosFollowRangeMax = 400.0f;
const float kHeightReadjustmentDelay = 2.0f;
const float kMinHeight = 450.0f;
const int kHeightRandVariationMax = 150.0f;
const float MaxHeightBelowCameraTop = 170.0f;

AIStateFlying::AIStateFlying(NPC * npc) :
	AIState(npc)
{
	mStateType = kRangeAttack;

	mFireRange = 20.0f + ((rand() % 500) * 0.1f);
}

AIStateFlying::~AIStateFlying(void)
{
}

void AIStateFlying::OnTransition()
{
	if (m_npc)
	{
		mCurrentHeightTarget = m_npc->m_player->Y() + (kMinHeight + (rand() % kHeightRandVariationMax));

		mRandXTargetOffset = kXPosFollowRangeMin + rand() % (int)(kXPosFollowRangeMax - kXPosFollowRangeMin);
	}
}

void AIStateFlying::Update(float delta)
{
	if (m_npc->m_player == nullptr)
	{
		return;
	}

	if (m_npc->m_player->Y() < m_npc->Y() &&
		std::abs(m_npc->m_player->X() - m_npc->X()) < mFireRange)
	{
		m_npc->FireProjectileAtObject(m_npc->m_player);
	}

	mTimeUntilCanChangeHeight += delta;

	if (mTimeUntilCanChangeHeight > kHeightReadjustmentDelay)
	{
		if (m_npc->m_player->IsOnSolidSurface())
		{
			mCurrentHeightTarget = m_npc->m_player->Y() + (kMinHeight + (rand() % kHeightRandVariationMax));

			if (mCurrentHeightTarget > (Camera2D::GetInstance()->Top() - MaxHeightBelowCameraTop))
			{
				mCurrentHeightTarget = Camera2D::GetInstance()->Top() - MaxHeightBelowCameraTop;
			}

			mTimeUntilCanChangeHeight = 0.0f;
		}
	}
	
	MoveTowardsDesiredHeight(delta);

	MoveTowardsDesiredXPosition(delta);
}

void AIStateFlying::MoveTowardsDesiredHeight(float delta)
{
	if (m_npc->Y() < (mCurrentHeightTarget - kHeightFollowRange))
	{
		m_npc->AccelerateY(1.0f, 0.1f);
	}
	else if (m_npc->Y() > (mCurrentHeightTarget + kHeightFollowRange))
	{
		m_npc->AccelerateY(-1.0f, 0.1f);
	}
	else
	{
		m_npc->StopYAccelerating();
		m_npc->SetVelocityY(m_npc->GetVelocity().Y * 0.99f);
	}
}

void AIStateFlying::MoveTowardsDesiredXPosition(float delta)
{
	if (m_npc->X() > (m_npc->m_player->X() + mRandXTargetOffset))
	{
		m_npc->AccelerateX(-1.0f);
	}
	else if (m_npc->X() < (m_npc->m_player->X() - mRandXTargetOffset))
	{
		m_npc->AccelerateX(1.0f);
	}
	else
	{
		m_npc->SetVelocityX(m_npc->VelocityX() * 0.99f);
	}
}
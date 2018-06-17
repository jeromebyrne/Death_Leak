#include "precompiled.h"
#include "AIStateGroundAnimalWander.h"
#include "NPC.h"

static const float kRestDelayMin = 0.5f;
static const float kRestDelayMax = 3.0f;
static const float kMoveDelayMin = 0.75f;
static const float kMoveDelayMax = 1.5f;
static const float kRunAwayMinDelay = 1.0f;
static const float kRunAwayPlayerDistanceSquared = 200.0f * 200.0f;

AIStateGroundAnimalWander::AIStateGroundAnimalWander(NPC * npc) :
	AIState(npc),
	mCurrentInternalState(kMoving),
	mCurrentMovingTime(0.0f),
	mCurrentRestTime(0.0f),
	mCurrentTimeRunningFromPlayer(0.0f),
	mOriginalMaxVelocityX(0.0f)
{
	mStateType = kGroundAnimalWander;
}

AIStateGroundAnimalWander::~AIStateGroundAnimalWander(void)
{
}

void AIStateGroundAnimalWander::OnTransition()
{
	// do stuff
	mCurrentInternalState = kMoving;
	PickRandomXDirection();
	mCurrentMovingTime = (rand() % (int)((kMoveDelayMax - kMoveDelayMin) * 100)) * 0.01 + kMoveDelayMin;
	mOriginalMaxVelocityX = m_npc->GetMaxVelocity().X;
}

void AIStateGroundAnimalWander::Update(float delta)
{
	switch (mCurrentInternalState)
	{
		case kMoving:
		{
			m_npc->SetMaxVelocityX(mOriginalMaxVelocityX);
			m_npc->m_resistance.X = 0.9f;
			UpdateMoving(delta);
			break;
		}
		case kResting:
		{
			m_npc->SetMaxVelocityX(mOriginalMaxVelocityX);
			m_npc->m_resistance.X = 0.9f;
			UpdateResting(delta);
			break;
		}
		case kRunningFromPlayer:
		{
			m_npc->SetMaxVelocityX(mOriginalMaxVelocityX * 3.0f);
			m_npc->m_resistance.X = 0.97f;
			UpdateRunningFromPlayer(delta);
			break;
		}
	}
}

void AIStateGroundAnimalWander::UpdateMoving(float delta)
{
	if (m_npc->IsHittingSolidLineEdge())
	{
		if (IsPlayerCloseEnoughToJumpFromEdge())
		{
			mCurrentInternalState = kRunningFromPlayer;
			mCurrentTimeRunningFromPlayer = 2.0f;
			m_npc->SetVelocityY(0.5f);
			m_npc->Jump(100.0f);
		}
		else
		{
			m_npc->SetDirectionX(-m_npc->DirectionX());
		}
	}

	if (IsPlayerClose())
	{
		m_npc->SetVelocityX(0.0f);
		mCurrentInternalState = kRunningFromPlayer;
		return;
	}

	if (mCurrentMovingTime > 0.0f)
	{
		mCurrentMovingTime -= delta;

		float dirX = m_npc->DirectionX();
		if (!CanAccelerateX(dirX))
		{
			dirX = -dirX;
			m_npc->SetDirectionX(dirX);
		}
		m_npc->AccelerateX(dirX);
	}
	else
	{
		mCurrentInternalState = kResting;
		mCurrentMovingTime = 0.0f;
		mCurrentRestTime = (rand() % (int)((kRestDelayMax - kRestDelayMin) * 100)) * 0.01 + kRestDelayMin;
	}
}

void AIStateGroundAnimalWander::UpdateResting(float delta)
{
	if (IsPlayerClose())
	{
		m_npc->SetVelocityX(0.0f);
		mCurrentInternalState = kRunningFromPlayer;
		return;
	}

	if (mCurrentRestTime > 0.0f)
	{
		mCurrentRestTime -= delta;
		m_npc->StopXAccelerating();
	}
	else
	{
		mCurrentInternalState = kMoving;
		PickRandomXDirection();
		mCurrentRestTime = 0.0f;
		mCurrentMovingTime = (rand() % (int)((kMoveDelayMax - kMoveDelayMin) * 100)) * 0.01 + kMoveDelayMin;
	}
}

void AIStateGroundAnimalWander::PickRandomXDirection()
{
	float dirX = rand() % 2;

	if (dirX == 0)
	{
		m_npc->m_direction.X = 1.0f;
	}
	else
	{
		m_npc->m_direction.X = -1.0f;
	}
}

bool AIStateGroundAnimalWander::CanAccelerateX(float direction)
{
	GAME_ASSERT(std::abs(direction) == 1.0f);

	auto cam = Camera2D::GetInstance();

	if (cam->IsObjectInView(m_npc))
	{
		// should be able to accelerate if in view
		return true;
	}

	if (direction < 0.0f)
	{
		if (m_npc->m_position.X - 10.0f < cam->GetLeftLevelBounds())
		{
			return false;
		}
	}
	else
	{
		if (m_npc->m_position.X + 10.0f > cam->GetRightLevelBounds())
		{
			return false;
		}
	}

	return true;
}

void AIStateGroundAnimalWander::UpdateRunningFromPlayer(float delta)
{
	if (m_npc->IsHittingSolidLineEdge())
	{
		mCurrentInternalState = kRunningFromPlayer;
		mCurrentTimeRunningFromPlayer = 2.0f;
		m_npc->SetVelocityY(0.5f);
		m_npc->Jump(70.0f);
	}

	if (mCurrentTimeRunningFromPlayer > 0.0f)
	{
		if (m_npc->m_player)
		{
			float xDiff = m_npc->m_player->Position().X - m_npc->X();

			if (xDiff > 0.0f)
			{
				m_npc->AccelerateX(-5.0f);
			}
			else
			{
				m_npc->AccelerateX(5.0f);
			}
		}

		mCurrentTimeRunningFromPlayer -= delta;
	}
	else
	{
		// re-evaluate if the player is still close
		if (!IsPlayerClose())
		{
			mCurrentTimeRunningFromPlayer = 0.0f;

			// go back to rest state
			mCurrentInternalState = kResting;
		}
		else
		{
			mCurrentTimeRunningFromPlayer = kRunAwayMinDelay;
		}
	}
}

bool AIStateGroundAnimalWander::IsPlayerClose() const
{
	if (!m_npc->m_player)
	{
		return false;
	}

	Vector2 distanceSquaredVector = m_npc->m_player->Position() - m_npc->Position();
	float distanceSquared = distanceSquaredVector.LengthSquared();

	if (distanceSquared < kRunAwayPlayerDistanceSquared)
	{
		return true;
	}

	return false;
}

bool AIStateGroundAnimalWander::IsPlayerCloseEnoughToJumpFromEdge() const
{
	if (!m_npc->m_player)
	{
		return false;
	}

	Vector2 distanceSquaredVector = m_npc->m_player->Position() - m_npc->Position();
	float distanceSquared = distanceSquaredVector.LengthSquared();

	if (distanceSquared < kRunAwayPlayerDistanceSquared * 2.0f)
	{
		return true;
	}

	return false;
}
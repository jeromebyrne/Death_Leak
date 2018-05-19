#include "precompiled.h"
#include "AIStateFlying.h"
#include "NPC.h"

AIStateFlying::AIStateFlying(NPC * npc) :
	AIState(npc)
{
	mStateType = kRangeAttack;

	mHeightAbovePlayer = 475.0f + (rand() % 150);
}

AIStateFlying::~AIStateFlying(void)
{
}

void AIStateFlying::OnTransition()
{
	/*
	float randMaxXVelocity = rand() % 3000;
	randMaxXVelocity *= 0.001f;
	randMaxXVelocity += 10.0f;
	*/

	// m_npc->SetMaxVelocityXYZ(randMaxXVelocity, 99999, 0);
}

void AIStateFlying::Update(float delta)
{
	if (m_npc->m_player == nullptr)
	{
		return;
	}

	m_npc->FireProjectileAtObject(m_npc->m_player);

	m_npc->SetY(m_npc->m_player->Y() +  mHeightAbovePlayer);
}
#include "precompiled.h"
#include "AIStateFlying.h"
#include "NPC.h"

AIStateFlying::AIStateFlying(NPC * npc) :
	AIState(npc)
{
	mStateType = kRangeAttack;
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
	
}
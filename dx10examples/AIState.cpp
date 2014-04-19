#include "precompiled.h"
#include "AIState.h"
#include "NPC.h"

AIState::AIState(NPC * npc):
mStateType(kAbstract),
m_npc(npc)
{
}

AIState::~AIState(void)
{
}
